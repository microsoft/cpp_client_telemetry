#define LOG_MODULE DBG_API

//#pragma unmanaged

#include "ExpCommonClient.hpp"
#include "pal/PAL.hpp"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <memory>

#if ARIASDK_PAL_WIN32
#ifdef _WINRT_DLL
#include "http/HttpClient_WinRt.hpp"
#else 
#include "http/HttpClient_WinInet.hpp"
#endif
//    #include "http/HttpClient_WinInet.hpp"
#endif

using namespace std;
using namespace Microsoft::Applications::Telemetry;
using namespace Microsoft::Applications::Telemetry::PAL;

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {

        ARIASDK_LOG_INST_COMPONENT_CLASS(ExpCommon, "AriaSDK.ExpCommonClient", "Aria experimentation client - common client class");

            const std::string ExpCommon::EXPClientStatus2STR[EXP_COUNT] =
            {
                EXPCLIENT_STATE_UNKNOWN,
                EXPCLIENT_STATE_INITIALIZED,
                EXPCLIENT_STATE_STARTED,
                EXPCLIENT_STATE_SUSPENDED,
                EXPCLIENT_STATE_STOPPING,
                EXPCLIENT_STATE_STOPPED,
                EXPCLIENT_STATE_REQUEST_PARAM_CHANGED
            };

            const std::string ExpCommon::EXPConfigUpdateResult2STR[EXP_CUR_COUNT] =
            {
                EXPCLIENT_CONFIG_UPDATE_SUCCEEDED,
                EXPCLIENT_CONFIG_UPDATE_FAILED,
                EXPCLIENT_CONFIG_UPDATE_TOBERETRIED
            };

            const std::string ExpCommon::EXPConfigUpdateSource2STR[EXP_CUS_COUNT] =
            {
                EXPCLIENT_CONFIG_SOURCE_SERVER,
                EXPCLIENT_CONFIG_SOURCE_LOCAL
            };

            /******************************************************************************
            * ExpCommon::ExpCommon
            *
            * C'tor
            *
            ******************************************************************************/
            ExpCommon::ExpCommon(IExpCommonClient* client, std::string retry_Queue_Name)
                : m_clientPtr(client),
                m_retry_Queue_Name(retry_Queue_Name),
                m_status(EXP_UNKNOWN),
                m_isTimerCancelling(false),
                m_minExpireTimeInSEXP(DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN),
                m_forceRefech(false),
                m_configActiveUserId(""),
                m_retrybackoffTimesIndex(0),
                m_retryTimeFactor(0),
                m_messageProcessingTaskScheduled(false)
            {
                ARIASDK_LOG_DETAIL("ExpCommonClient c'tor: this=0x%x", this);

                ARIASDK_NS::PAL::initialize();
#if ARIASDK_PAL_SKYPE
                ARIASDK_LOG_DETAIL("HttpClient: Skype HTTP Stack (provided IHttpStack=%p)", configuration.skypeHttpStack);
                m_ownHttpClient.reset(new HttpClient_HttpStack(configuration.skypeHttpStack));
#elif ARIASDK_PAL_WIN32
                
#ifdef _WINRT_DLL
                ARIASDK_LOG_DETAIL("HttpClient: WinRt");
                m_httpClient = new HttpClient_WinRt();
#else 
                ARIASDK_LOG_DETAIL("HttpClient: WinInet");
                m_httpClient = new HttpClient_WinInet();
#endif
#else
#error The library cannot work without an HTTP client implementation.
#endif

            }

            /******************************************************************************
            * ExpCommon::~ExpCommon
            *
            * D'tor
            *
            ******************************************************************************/
            ExpCommon::~ExpCommon()
            {
                ARIASDK_LOG_DETAIL("ExpCommon d'tor: this=0x%x", this);

                if (m_status == EXP_STARTED)
                {
                    (void)Stop();
                }
                CancelRequestsAsync();
                
                if (m_messageProcessingTaskScheduled)
                {
                    m_messageProcessingTask.cancel();
                    m_messageProcessingTaskScheduled = false;
                }
                if (m_httpClient) delete m_httpClient;

                ARIASDK_NS::PAL::shutdown();

                std::lock_guard<std::mutex> lockguard(m_smalllock);
                std::lock_guard<std::mutex> lock(m_lock);                
            }

            /******************************************************************************
            * ExpCommon::_ValidateExpCommonConfiguration
            *
            * Validate the ExpCommonConfiguration struct specified by client application
            *
            ******************************************************************************/

            //    void ExpCommon::AddHeader(const std::string& headerName, const std::string& headerValue)
            //    {
           //         m_configActiveHeaders.emplace(headerName, headerValue);
            //    }

            bool ExpCommon::RegisterLogger(ILogger* pLogger, const string& agentName)
            {
                ARIASDK_LOG_DETAIL("RegisterLogger[%d]: this=0x%x, ILogger=0x%x, agent=%s", __LINE__, this, pLogger, agentName.c_str());

                if (pLogger == NULL || agentName.empty())
                {
                    ARIASDK_LOG_ERROR("RegisterLogger: Either the logger provided is null or the agentname is empty");
                    return false;
                }

                {
                    std::lock_guard<std::mutex> lockguard(m_smalllock);

                    if (m_registeredLoggers.find(pLogger) != m_registeredLoggers.end())
                    {
                        ARIASDK_LOG_ERROR("RegisterLogger: Logger provided was registered already");
                        return false;
                    }

                    m_registeredLoggers[pLogger] = agentName;
                }

                return true;
            }

            /******************************************************************************
            * ExpCommon::Start
            *
            * Start the EXP client
            *
            ******************************************************************************/
            bool ExpCommon::Start(std::vector<int>& retryTimes)
            {

                // check status first, simply return if it hasn't been initialzied or has already started
                if (m_status != EXP_INITIALIZED &&
                    m_status != EXP_STOPPED)
                {
                    ARIASDK_LOG_ERROR("Start: ExpCommon hasn't been initialzied or has already started");
                    return false;
                }


                m_retryBackoffTimes = retryTimes;

                m_status = EXP_STARTED;
                ARIASDK_LOG_DETAIL("Start: ExpCommon successfully started");

                // dispatch message to asynchronously refetch the active config from EXP server if necessary
                Message message(MT_RELOAD_CONFIG);
                message.requestName = m_configActiveRequestName;

                _DispatchMessage(message);

                return true;
            }

            /******************************************************************************
            * ExpCommon::Resume
            *
            * Resume the EXP client to retrieve configuration from EXP server
            *
            ******************************************************************************/
            bool ExpCommon::Resume()
            {
                m_status = EXP_STARTED;
                ARIASDK_LOG_DETAIL("Start: ExpCommon successfully started");

                // dispatch message to asynchronously refetch the active config from EXP server if necessary
                Message message(MT_RELOAD_CONFIG);
                message.requestName = m_configActiveRequestName;

                _DispatchMessage(message);

                return true;
            }

            /******************************************************************************
            * ExpCommon::Suspend
            *
            * Suspend the EXP client from retrieving configuration from EXP server
            *
            ******************************************************************************/
            bool ExpCommon::Suspend()
            {	
                _StopInternal();

                ARIASDK_LOG_DETAIL("Suspend: ExpCommon successfully suspended");
                m_status = EXP_SUSPENDED;

                return true;
            }

            /******************************************************************************
            * ExpCommon::Stop
            *
            * Stop the EXP client
            *
            ******************************************************************************/
            bool ExpCommon::Stop()
            {	
                m_status = EXP_STOPPED;

                _StopInternal();

                ARIASDK_LOG_DETAIL("Stop: ExpCommon successfully stopped");				

                return true;
            }

            void ExpCommon::_StopInternal()
            {

            }

                        
            std::string ExpCommon::GetRequestName(const std::string clientName)
            {
                std::string requestName;

                if (!clientName.empty())
                {
                    requestName += "[" + EXPCLIENT_RP_KEY_CLIENT_NAME + ":" + clientName + "]";
                }
                
                if (!m_configActiveDeviceId.empty())
                {
                    requestName += "[" + EXPCLIENT_RP_KEY_CLIENTID + ":" + m_configActiveDeviceId + "]";
                }

                if (!m_configActiveUserId.empty())
                {
                    requestName += "[" + EXPCLIENT_RP_KEY_ID + ":" + m_configActiveUserId + "]";
                }

                return requestName;
            }


            /******************************************************************************
            * ExpCommon::SetRequestParameters
            *
            * Set a list of request parameters for retrieve configurations from AFD server
            *
            ******************************************************************************/
            bool ExpCommon::SetRequestParameters(const std::map<std::string, std::string>& requestParams, bool setUserId)
            {
                ARIASDK_LOG_DETAIL("SetRequestParameters[%d]: ExpCommon=0x%x, request parameters count=%u", __LINE__, this, requestParams.size());

                std::map<std::string, std::string> requestParamsLocal;

                std::map<std::string, std::string>::const_iterator it;
                for (it = requestParams.begin(); it != requestParams.end(); it++)
                {
                    requestParamsLocal[it->first] = it->second;
                }

                if (!m_configActiveDeviceId.empty())
                {
                    requestParamsLocal[EXPCLIENT_RP_KEY_CLIENTID] = m_configActiveDeviceId;
                }               
                
                if (setUserId)
                {
                    requestParamsLocal[EXPCLIENT_RP_KEY_ID] = m_configActiveUserId;
                }

                std::lock_guard<std::mutex> lockguard(m_lock);
                m_configActiveRequestParams = requestParams;
                               
                return true;
            }

            

            /******************************************************************************
            * ExpCommon::_DispatchMessage
            *
            * Dispatch MT_RELOAD_CONFIG or MT_HTTP_CALLBACK for async processing
            *
            ******************************************************************************/
            void ExpCommon::_DispatchMessage(const Message& msg)
            {
                if (m_status != EXP_STARTED)
                {
                    ARIASDK_LOG_DETAIL("OnTimerElapsed: Timer wake-up ignored[Status=%d]", m_status);
                    return;
                }
                std::lock_guard<std::mutex> lockguard(m_smalllock);
                m_messages.push(msg);

                if (m_messageProcessingTaskScheduled)
                {
                    m_messageProcessingTask.cancel();
                    m_messageProcessingTaskScheduled = false;
                }
                
                m_messageProcessingTask = PAL::scheduleOnWorkerThread(0,self(), &ExpCommon::handleMessageTask );
                m_messageProcessingTaskScheduled = true;
            }

            /******************************************************************************
            * ExpCommon::handleMessageTask
            *
            * callback function invoked by PAL thread
            *
            ******************************************************************************/
            void ExpCommon::handleMessageTask()
            {
                ARIASDK_LOG_DETAIL("handleMessageTask: ");

                if (m_status != EXP_STARTED)
                {
                    ARIASDK_LOG_DETAIL("OnTimerElapsed: Timer wake-up ignored[Status=%d]", m_status);
                    return;
                }
                                
                bool isActiveConfigSwitched = false;
                bool isActiveConfigSwitchedSaveNeeded = false;
                bool isActiveConfigUpdatedOnEXP = false;
                bool isActiveConfigUpdatedOnEXPSaveNeeded = false;

                // PAL::ITimer* tempTimer = NULL;
                // handle messages in the queue by taking a snapshot of all messages in the queue.
                // message coming after the snapshot won't be handled until next time timer expires
                std::queue<Message> messages;
                {
                    std::lock_guard<std::mutex> lockguard(m_smalllock);

                    std::swap(messages, m_messages);
                }

                // handle all messages by either reload configs from local cache or refetch from EXP server        
                _HandleMessages(messages, isActiveConfigSwitched, isActiveConfigSwitchedSaveNeeded, isActiveConfigUpdatedOnEXP, isActiveConfigUpdatedOnEXPSaveNeeded);

                if (m_clientPtr && (isActiveConfigSwitched || isActiveConfigUpdatedOnEXP))
                {
                    m_clientPtr->HandleUpdateClient(isActiveConfigSwitched, isActiveConfigUpdatedOnEXP, isActiveConfigUpdatedOnEXPSaveNeeded);
                }				

                if (m_clientPtr && (isActiveConfigSwitchedSaveNeeded || isActiveConfigUpdatedOnEXPSaveNeeded))
                {
                    m_clientPtr->HandleConfigSave(isActiveConfigSwitchedSaveNeeded, isActiveConfigUpdatedOnEXPSaveNeeded);
                }

                ARIASDK_LOG_DETAIL("OnTimerElapsed: messages handled, lock released.");
            }

            /******************************************************************************
            * ExpCommon::_HandleMessages
            *
            * Invoked within timer callback to handle messages asynchronously
            *
            ******************************************************************************/
            void ExpCommon::_HandleMessages(
                std::queue<Message>& msgs,
                bool& isActiveConfigSwitched,
                bool& isActiveConfigSwitchedSaveNeeded,
                bool& isActiveConfigUpdatedOnEXP,
                bool& isActiveConfigUpdatedOnEXPSaveNeeded)
            {
                if (m_status != EXP_STARTED)
                {
                    ARIASDK_LOG_DETAIL("_HandleMessages: Timer wake-up ignored[Status=%d]", m_status);
                    return;
                }
                isActiveConfigSwitched = false;
                isActiveConfigSwitchedSaveNeeded = false;

                isActiveConfigUpdatedOnEXP = false;
                isActiveConfigUpdatedOnEXPSaveNeeded = false;

                Message msgPending(MT_UNKNOWN);

                while (!msgs.empty())
                {
                    Message& msg = msgs.front();

                    ARIASDK_LOG_DETAIL("_HandleMessages: Processing message [type=%u]", msg.type);
                    switch (msg.type)
                    {
                    case MT_HTTP_CALLBACK:
                        _HandleHttpCallback(msg, isActiveConfigUpdatedOnEXP, isActiveConfigUpdatedOnEXPSaveNeeded);
                        break;

                    case MT_RELOAD_CONFIG:
                        // there could be multiple MT_RELOAD_CONFIG messages, hanlde only the latest one
                        msgPending = msg;
                        break;

                    default:
                        ARIASDK_LOG_ERROR("_HandleMessages: Unexpected message [type=%u]", msg.type);
                        assert(false);
                        break;
                    }

                    msgs.pop();
                }

                // Process the latest config reload message if there is one
                if (msgPending.type == MT_RELOAD_CONFIG)
                {
                    if (m_clientPtr)
                    {
                        m_clientPtr->HandleConfigReload(msgPending, isActiveConfigSwitched, isActiveConfigSwitchedSaveNeeded);
                    }
                    _HandleConfigRefetch(msgPending, isActiveConfigSwitched, isActiveConfigSwitchedSaveNeeded);
                }
            }

            /******************************************************************************
            * ExpCommon::_HandleHttpCallback
            *
            * Process the HTTPCallback within timer callback
            *
            ******************************************************************************/
            void ExpCommon::_HandleHttpCallback(
                Message& msg,
                bool& isActiveConfigUpdatedOnEXP,
                bool& isActiveConfigUpdatedOnEXPSaveNeeded)
            {
                if (m_clientPtr)
                {
                    m_clientPtr->HandleHttpCallback(msg, isActiveConfigUpdatedOnEXP, isActiveConfigUpdatedOnEXPSaveNeeded);
                }
            }

            /******************************************************************************
            * ExpCommon::_HandleConfigefetch
            *
            * Reload from local cache or issue condig refetch request within timer callback
            *
            ******************************************************************************/
            void ExpCommon::_HandleConfigRefetch(
                Message& msg,
                bool& isActiveConfigSwitched,
                bool& isActiveConfigSwitchedSaveNeeded)
            {
                UNREFERENCED_PARAMETER(isActiveConfigSwitched);
                UNREFERENCED_PARAMETER(isActiveConfigSwitchedSaveNeeded);
                ARIASDK_LOG_DETAIL("_HandleConfigRefetch: Reload/Re-fetch config for RequestName=%s", msg.requestName.c_str());

                if (m_status != EXP_STARTED)
                {
                    ARIASDK_LOG_DETAIL("_HandleConfigRefetch: Timer wake-up ignored[Status=%d]", m_status);
                    return;
                }

                assert(msg.type == MT_RELOAD_CONFIG);

                // Check if the active config has expired or not, refetch config from EXP if expired.
                // Otherwise set a timer to expire it and notify all listeners if it has now changed.

                if (m_clientPtr)
                {
                    if(!m_clientPtr->FetchFromServerIfRequired())
                    {						
                        _ScheduleFetch();
                    }
                }
            }

            void ExpCommon::_ScheduleFetch(unsigned int seconds /* default value 0*/) // handles retry
            {
                unsigned int expiryTimeInSec = 0;

                if (m_clientPtr)
                {
                    expiryTimeInSec = m_clientPtr->GetExpiryTimeInSec();
                }

                if (seconds != 0)
                {//select the minimun of passed time and expire time

                    if (seconds < expiryTimeInSec)
                    {
                        expiryTimeInSec = seconds;
                    }
                }

                ARIASDK_LOG_DETAIL("_HandleConfigReloadAndRefetch: Config still valid [Expires in %d sec]", expiryTimeInSec);


                if (m_messageProcessingTaskScheduled)
                {
                    m_messageProcessingTask.cancel();
                    m_messageProcessingTaskScheduled = false;
                }

                Message message(MT_RELOAD_CONFIG);
                message.requestName = m_configActiveRequestName;
                {
                    std::lock_guard<std::mutex> lockguard(m_smalllock);
                    m_messages.push(message);
                }

                m_messageProcessingTask = PAL::scheduleOnWorkerThread(expiryTimeInSec * 1000, self(), &ExpCommon::handleMessageTask);
                m_messageProcessingTaskScheduled = true;
            }

            void ExpCommon::_FireClientEvent(CommonClientEventType evtType, bool fConfigUpdateFromEXP)
            {
                ARIASDK_LOG_DETAIL("_FireExpCommonEvent[%d]:  EventType=%d, ConfigUpdateFromEXP=%d", evtType, fConfigUpdateFromEXP);
                if (m_clientPtr)
                {
                    m_clientPtr->FireClientEvent(evtType, fConfigUpdateFromEXP);
                }
            }
            
            
            void ExpCommon::_UpdateLoggerWithEXPConfig(ILogger* pLogger, const string& agentName, const std::string etag, const std::string configIds)
            {
                if (pLogger && !agentName.empty())
                {
                    ISemanticContext* pLoggerCtx = pLogger->GetSemanticContext();
                    assert(pLoggerCtx != NULL);

                    // 1) Set the EXP Etag as context info to the logger 
                    string eTag = etag;
                    pLoggerCtx->SetAppExperimentETag(eTag);

                    ARIASDK_LOG_DETAIL("_UpdateLoggerWithEXPConfig: logger(0x%x) added with ETag=%s", pLogger, eTag.c_str());

                    // For version 1 the ConfigIDs are not under the EventToConfigIds mapping.
                    // Read V1 value and set it first.
                    // Read V2 value 2nd and set it so that if there are V2 value then
                    // the V2 value will override the V1 value.
                    if (!configIds.empty())
                    {
                        ARIASDK_LOG_DETAIL("_UpdateLoggerWithEXPConfig: logger(0x%x) SetAppExperimentIds configIds=%s", pLogger, configIds.c_str());
                        pLoggerCtx->SetAppExperimentIds(configIds);
                    }
                }
            }

            void ExpCommon::_UpdateLoggerWithEXPConfig(ILogger* pLogger, const std::string& agentName, const std::string etag, const std::map<std::string, std::string> eventconfigIds)
            {
                if (pLogger && !agentName.empty())
                {
                    ISemanticContext* pLoggerCtx = pLogger->GetSemanticContext();
                    assert(pLoggerCtx != NULL);

                    // 1) Set the EXP Etag as context info to the logger 
                    string eTag = etag;
                    pLoggerCtx->SetAppExperimentETag(eTag);

                    ARIASDK_LOG_DETAIL("_UpdateLoggerWithEXPConfig: logger(0x%x) added with ETag=%s", pLogger, eTag.c_str());

                    // For version 2, the ConfigIDs are under the EventToConfigIds
                    for (std::map<std::string, std::string>::const_iterator iter  = eventconfigIds.begin(); iter != eventconfigIds.end(); iter++)
                    {						
                        ARIASDK_LOG_DETAIL("_UpdateLoggerWithEXPConfig: logger(0x%x) SetEventExperimentIds eventName=%s, eventConfigIds=%s", pLogger, iter->first.c_str(), iter->second.c_str());
                        pLoggerCtx->SetEventExperimentIds(iter->first, iter->second);
                    }

                    // TODO: consider whether we should clean-up/reset the EventToConfigIds mapping previously set on a logger
                    // Doing this likely will require a separate API on ISemanticContext
                }
            }

            void ExpCommon::_LogEXPConfigEvent(EventProperties& evtProperties)
            {
                ARIASDK_LOG_DETAIL("_LogEXPConfigEvent[%d]: ExpCommon=0x%x, logger count=%u", __LINE__, this, m_registeredLoggers.size());

                for (std::map<ILogger *, string>::iterator it = m_registeredLoggers.begin(); it != m_registeredLoggers.end(); it++)
                {
                    ILogger* pLogger = it->first;
                    assert(pLogger != NULL);

                    pLogger->LogEvent(evtProperties);
                }
            }

            void ExpCommon::SendRequestAsync(std::string const& url)
            {
                IHttpRequest* request = m_httpClient->CreateRequest();
                m_httpRequestId = request->GetId();
                request->SetMethod("GET");
                HttpHeaders& header = request->GetHeaders();

                for (std::map<std::string, std::string>::iterator iter = m_configActiveHeaders.begin(); iter != m_configActiveHeaders.end(); ++iter)
                {
                    header.add(iter->first, iter->second);
                }

                header.add("Content-Encoding", "gzip");

                std::string urlAndParams = url;
                if (urlAndParams.at(urlAndParams.length() - 1) == '/')
                {
                    urlAndParams = urlAndParams.substr(0, urlAndParams.length() - 1);
                }

                bool first = true;
                for (std::map<std::string, std::string>::iterator iterP = m_configActiveRequestParams.begin(); iterP != m_configActiveRequestParams.end(); ++iterP)
                {
                    if (first)
                    {
                        first = false;
                        urlAndParams.push_back('?');
                    }
                    else
                    {
                        urlAndParams.push_back('&');
                    }
                    urlAndParams = urlAndParams + iterP->first;
                    urlAndParams.push_back('=');
                    urlAndParams = urlAndParams + iterP->second;
                }

                request->SetUrl(urlAndParams);
                m_httpClient->SendRequestAsync(request, this);
            }

            /******************************************************************************
            * ExpCommon::OnHttpCallback
            *
            * Custom HTTP callback used for EXP instead of default HCM callback
            *
            ******************************************************************************/
            void ExpCommon::OnHttpResponse(IHttpResponse const* response)
            {
                std::lock_guard<std::mutex> lock(m_lock);
                m_httpRequestId = "";
                if (response->GetStatusCode() != 200)
                {
                    if (m_retrybackoffTimesIndex < static_cast<unsigned>(m_retryBackoffTimes.size()))
                    {
                        unsigned int expiryTimeInSec = m_retryBackoffTimes.at(m_retrybackoffTimesIndex);
                        m_retrybackoffTimesIndex++;

                        if (m_retryTimeFactor > 0)
                        {
                            expiryTimeInSec = m_retryTimeFactor;
                        }
                        // create new timer
                        _ScheduleFetch(expiryTimeInSec);
                    }									
                    return;
                }

                m_retrybackoffTimesIndex = 0;

                Message message(MT_HTTP_CALLBACK);
                message.statusCode = response->GetStatusCode();
                ARIASDK_LOG_DETAIL("OnHttpCallback: respondse headers received");
                message.headers = response->GetHeaders();

                ARIASDK_LOG_DETAIL("OnHttpCallback: respondse body received");
                std::vector<uint8_t> body = response->GetBody();
                if (body.size() > 0)
                {
                    std::string s;
                    s.reserve(body.size());

                    for (uint8_t value : body)
                    {
                        s.push_back(value);
                    }
                    message.body = s;
                }

                // This callback function could be invoked by HTTPStach Stop()/Abort(), in which case
                // it will ignored
                if (m_status != EXP_STARTED)
                {
                    ARIASDK_LOG_DETAIL("OnHttpCallback: callback ignored[Status=%d]", m_status);
                    return;
                }

                // dispatch the message to the message queue for async processing
                _DispatchMessage(message);
                
                // No HCM retry needed
                return;
            }

            bool ExpCommon::CancelRequestsAsync()
            {
                if (!m_httpRequestId.empty())
                {
                    m_httpClient->CancelRequestAsync(m_httpRequestId);
                }
                return true;
            }
        }
    }
}
