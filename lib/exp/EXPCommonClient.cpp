#define LOG_MODULE DBG_API

#pragma unmanaged

#include "ExpCommonClient.hpp"


#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <memory>

/* Make sure we don't use Windows.h definition of CreateMutex */
#ifdef CreateMutex
#undef CreateMutex
#endif

using namespace std;
using namespace common;
using namespace Microsoft::Applications::Telemetry;
using namespace Microsoft::Applications::Telemetry::PlatformAbstraction;

namespace Microsoft {
	namespace Applications {
		namespace Experimentation {

			void ThrowExpError(EXP_ERROR errCode, const char* pFile, int lineNumber)
			{
				std::string errMsg;

				switch (errCode)
				{
				case EXP_ERROR_INVALID_CONFIG:
					errMsg = "Invalid AFDClientConfiguration specified";
					LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
					THROW(std::invalid_argument(errMsg));
					break;

				case EXP_ERROR_OUT_OF_MEMORY:
					errMsg = "Out of momery";
					LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
					THROW(std::bad_alloc());
					break;

				case EXP_ERROR_INVALID_STATUS:
					errMsg = "This operation is not allowed at current state";
					LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
					THROW(std::logic_error(errMsg));
					break;

				default:
					errMsg = "Runtime error occurred.";
					LOG_ERROR("[AFDClient] Exception: %s File: %s Line: %d", errMsg.c_str(), pFile, lineNumber);
					THROW(std::runtime_error(errMsg));
					break;
				}
			}

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
				m_messageProcessTimer(NULL),
				m_configExpireTimer(NULL),
				m_retryTimer(NULL),
				m_isTimerCancelling(false),
				m_httpClientManager(NULL),
				m_minExpireTimeInSEXP(DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN),
				m_forceRefech(false),
                m_configActiveUserId(""),
				m_retrybackoffTimesIndex(0),
				m_retryTimeFactor(0)
			{
				GETPAL(m_pal);
				TRACE("ExpCommonClient c'tor: this=0x%x", this);

			}

			/******************************************************************************
			* ExpCommon::~ExpCommon
			*
			* D'tor
			*
			******************************************************************************/
			ExpCommon::~ExpCommon()
			{
				TRACE("ExpCommon d'tor: this=0x%x", this);

				if (m_status == EXP_STARTED)
				{
					(void)Stop();
				}

				SAFERELEASE(m_messageProcessTimer);
				SAFERELEASE(m_configExpireTimer);
				SAFERELEASE(m_retryTimer);
				SAFERELEASE(m_httpClientManager);
				{
					LOCKGUARD(m_lock); //make sure timer is not running and we can safely destruct
				}
				FREEPAL(m_pal);
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
				TRACE("RegisterLogger[%d]: this=0x%x, ILogger=0x%x, agent=%s", __LINE__, this, pLogger, agentName.c_str());

				if (pLogger == NULL || agentName.empty())
				{
					LOG_ERROR("RegisterLogger: Either the logger provided is null or the agentname is empty");
					return false;
				}

				{
					LOCKGUARD(m_smalllock);

					if (m_registeredLoggers.find(pLogger) != m_registeredLoggers.end())
					{
						LOG_ERROR("RegisterLogger: Logger provided was registered already");
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
			bool ExpCommon::Start(RetryTimes retryTimes)
			{

				// check status first, simply return if it hasn't been initialzied or has already started
				if (m_status != EXP_INITIALIZED &&
					m_status != EXP_STOPPED)
				{
					LOG_ERROR("Start: ExpCommon hasn't been initialzied or has already started");
					return false;
				}

				m_httpClientManager = HttpClientManager::GetInstance();
				if (m_httpClientManager == NULL)
				{
					LOG_ERROR("Initialize: Failed to create HttpClientManager");
					EXP_THROW(EXP_ERROR_OUT_OF_MEMORY);
				}

			    m_retryBackoffTimes = retryTimes.timeOuts;

				// Start it if not started already
				if (!m_httpClientManager->Start(retryTimes)) {
					EXP_THROW(EXP_ERROR_HTTPSTACK_INIT_FAILED);
				}



				m_status = EXP_STARTED;
				TRACE("Start: ExpCommon successfully started");

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
				TRACE("Start: ExpCommon successfully started");

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

				TRACE("Suspend: ExpCommon successfully suspended");
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

				if (m_httpClientManager != NULL)
				{
					m_httpClientManager->Stop(m_retry_Queue_Name);
					// Release our instance
					HttpClientManager::Free();
					m_httpClientManager = NULL;
				}

				TRACE("Stop: ExpCommon successfully stopped");				

				return true;
			}

			void ExpCommon::_StopInternal()
			{
				m_isTimerCancelling = true;

				// stop the message process timer
				if (m_messageProcessTimer)
				{
					m_messageProcessTimer->Stop();
					SAFERELEASE(m_messageProcessTimer);
				}

				// stop the configuration expiry timer
				if (m_configExpireTimer)
				{
					m_configExpireTimer->Stop();
					SAFERELEASE(m_configExpireTimer);
				}

				m_isTimerCancelling = false;
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
                TRACE("SetRequestParameters[%d]: ExpCommon=0x%x, request parameters count=%u", __LINE__, this, requestParams.size());

                std::map<std::string, std::string> requestParamsLocal;

                std::map<std::string, std::string>::const_iterator it;
                for (it = requestParams.begin(); it != requestParams.end(); it++)
                {
                    requestParamsLocal[it->first] = it->second;
                }

                requestParamsLocal[EXPCLIENT_RP_KEY_CLIENTID] = m_configActiveDeviceId;
                
                if (setUserId)
                {
                    requestParamsLocal[EXPCLIENT_RP_KEY_ID] = m_configActiveUserId;
                }

                LOCKGUARD(m_lock);
                m_configActiveRequestParams = requestParams;
                               
                return true;
            }

			/******************************************************************************
			* ExpCommon::OnHttpCallback
			*
			* Custom HTTP callback used for EXP instead of default HCM callback
			*
			******************************************************************************/
			bool ExpCommon::OnHttpCallback(
				int statusCode,
				int httpstackError,
				common::HeaderMap* headers,
				std::string* body,
				void* userData,
				std::vector<std::string>& userDataTag,
				unsigned int retryFailedTimes)
			{

				if (statusCode == INT32_MAX) //special code for retry request
				{
					unsigned int expiryTimeInSec = m_retryBackoffTimes.at(m_retrybackoffTimesIndex);
					m_retrybackoffTimesIndex++;

					if (m_retryTimeFactor > 0)
					{
						expiryTimeInSec = m_retryTimeFactor;
					}

					int timerData = INT32_MAX;
					if (m_retryTimer == NULL)
					{   // create new timer
						
						m_retryTimer = _CreateTimer(expiryTimeInSec, &timerData);
					}
					

					return false;
				}

                string userDataTags;
				m_retrybackoffTimesIndex = 0;

                for (auto userData : userDataTag)
                    userDataTags += (userDataTags.empty() ? "" : ",") + userData;

				TRACE("OnHttpCallback: HTTPStack error=%d, status code=%d, retryFailedTimes=%d, userData=%p, userDataTag=%s",
                    httpstackError, statusCode, retryFailedTimes, userData, (userDataTags.empty()) ? "NULL" : userDataTags.c_str());

				if ((userData == NULL) || (userDataTag.empty())) {
					LOG_WARN("userData or userDataTag is NULL, don't trigger EXP callback.");
					return true;
				}

				Message message(MT_HTTP_CALLBACK);

				message.httpstackError = httpstackError;
				message.statusCode = statusCode;
				message.userData = userData;
				message.userDataTag = userDataTag;
				message.retryFailedTimes = retryFailedTimes;

				if (headers)
				{
					TRACE("OnHttpCallback: respondse headers received");
					message.headers.swap(*headers);
				}

				if (body)
				{
					TRACE("OnHttpCallback: respondse body received");
					message.body.swap(*body);
				}

				while (!m_lock.try_lock())
				{
					TRACE("OnHttpCallback: tryLock failed, wait for 200ms.");
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				}

				// This callback function could be invoked by HTTPStach Stop()/Abort(), in which case
				// it will ignored
				if (m_status != EXP_STARTED)
				{
					TRACE("OnHttpCallback: callback ignored[Status=%d]", m_status);
                    m_lock.unlock();
					return true;
				}

				// dispatch the message to the message queue for async processing
				_DispatchMessage(message);

				m_lock.unlock();

				// No HCM retry needed
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
                LOCKGUARD(m_smalllock);
                m_messages.push(msg);
                {
                    // if the message processing timer has been created, create one to start processing messages
                    if (!m_messageProcessTimer)
                    {
                        m_messageProcessTimer = _CreateTimer(0,NULL);
                    }
                    else
                    {
                        m_messageProcessTimer->Start(0, 0, this, NULL);
                    }
                }
			}

			/******************************************************************************
			* ExpCommon::_CreateTimer
			*
			* Create a timer for async processing
			*
			******************************************************************************/
			// This creates a timer in the same strand, it will make sure the timer is executed one by one. 
			// You should never invoke cancelSync() in timer callback otherwise you may hit the dead lock. 
			// Use cancel() instead.
			ITimer* ExpCommon::_CreateTimer(unsigned int startDelayInSec, void* data )
			{
				ITimer* timer = NULL;

				m_pal->CreateTimer(&timer);
				if (timer)
				{
					timer->Start(startDelayInSec * 1000 * 1000, 0, this, data);
				}

				return timer;
			}

			/******************************************************************************
			* ExpCommon::OnTimerElapsed
			*
			* Timer callback function invoked by timer thread
			*
			******************************************************************************/
			void ExpCommon::OnTimerElapsed(ITimer* pTimer, void* userData)
			{
				if (userData)
				{ // this is not NULL for retry timer only
					m_httpClientManager->HandleRetryQueue(m_retry_Queue_Name);
					m_retryTimer = NULL;
					return;
				}
				// try grabbing the lock first prior to processing the message queue to either
				// refetch configuration from EXP or handle HTTP callback with EXP response 
				while (!m_lock.try_lock())
				{
					// TODO: handle the case where the timer is being cancelled
					if (m_isTimerCancelling)
					{
						TRACE("OnTimerElapsed: Timer wake-up ignored while being cancelled");
						return;
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(200));
				}

				TRACE("OnTimerElapsed: Timer wake-up, lock acquired");

				if (m_status != EXP_STARTED)
				{
					TRACE("OnTimerElapsed: Timer wake-up ignored[Status=%d]", m_status);
					m_lock.unlock();
					return;
				}

				// cancel any existing m_configExpireTimer because _HandleMessages below will check
				// if active config is expired. If expired it triggers an immediate refetch or 
				// create a new timer to expire it with the new expiry time.
				if (m_configExpireTimer)
				{
					TRACE("OnTimerElapsed: stop m_configExpireTimer");
                    m_configExpireTimer->Stop();
				}

                // stop the message process timer
                if (m_messageProcessTimer)
                {
                    TRACE("OnTimerElapsed: stop m_messageProcessTimer");
                    m_messageProcessTimer->Stop();
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
					LOCKGUARD(m_smalllock);

					std::swap(messages, m_messages);

					// handle all messages by either reload configs from local cache or refetch from EXP server        
					_HandleMessages(messages, isActiveConfigSwitched, isActiveConfigSwitchedSaveNeeded, isActiveConfigUpdatedOnEXP, isActiveConfigUpdatedOnEXPSaveNeeded);

                    if (m_clientPtr && (isActiveConfigSwitched || isActiveConfigUpdatedOnEXP))
					{
						m_clientPtr->HandleUpdateClient(isActiveConfigSwitched, isActiveConfigUpdatedOnEXP, isActiveConfigUpdatedOnEXPSaveNeeded);
					}
				}

                if (m_clientPtr && (isActiveConfigSwitchedSaveNeeded || isActiveConfigUpdatedOnEXPSaveNeeded))
				{
					m_clientPtr->HandleConfigSave(isActiveConfigSwitchedSaveNeeded, isActiveConfigUpdatedOnEXPSaveNeeded);
				}

                TRACE("OnTimerElapsed: messages handled, lock released.");
				m_lock.unlock();
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
                    TRACE("_HandleMessages: Timer wake-up ignored[Status=%d]", m_status);
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

					TRACE("_HandleMessages: Processing message [type=%u]", msg.type);
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
						LOG_ERROR("_HandleMessages: Unexpected message [type=%u]", msg.type);
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
				TRACE("_HandleConfigRefetch: Reload/Re-fetch config for RequestName=%s", msg.requestName.c_str());

                if (m_status != EXP_STARTED)
                {
                    TRACE("_HandleConfigRefetch: Timer wake-up ignored[Status=%d]", m_status);
                    return;
                }

				assert(msg.type == MT_RELOAD_CONFIG);

				// Check if the active config has expired or not, refetch config from EXP if expired.
				// Otherwise set a timer to expire it and notify all listeners if it has now changed.

				if (m_clientPtr)
				{
					if(!m_clientPtr->FetchFromServerIfRequired())
					{						
						unsigned int expiryTimeInSec = 0;

						if (m_clientPtr)
						{
							expiryTimeInSec = m_clientPtr->GetExpiryTimeInSec();
						}

						TRACE("_HandleConfigReloadAndRefetch: Config still valid [Expires in %d sec]", expiryTimeInSec);
                        if (m_configExpireTimer == NULL)
                        {   // create new timer
                            m_configExpireTimer = _CreateTimer(expiryTimeInSec, NULL);
                        }
                        else
                        {   // restart timer
                            m_configExpireTimer->Start(expiryTimeInSec * 1000 * 1000, 0, this, NULL);
                        }
					}
				}
			}

			void ExpCommon::_FireClientEvent(CommonClientEventType evtType, bool fConfigUpdateFromEXP)
			{
				TRACE("_FireExpCommonEvent[%d]:  EventType=%d, ConfigUpdateFromEXP=%d", evtType, fConfigUpdateFromEXP);
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

					TRACE("_UpdateLoggerWithEXPConfig: logger(0x%x) added with ETag=%s", pLogger, eTag.c_str());

					// For version 1 the ConfigIDs are not under the EventToConfigIds mapping.
					// Read V1 value and set it first.
					// Read V2 value 2nd and set it so that if there are V2 value then
					// the V2 value will override the V1 value.
					if (!configIds.empty())
					{
						TRACE("_UpdateLoggerWithEXPConfig: logger(0x%x) SetAppExperimentIds configIds=%s", pLogger, configIds.c_str());
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

					TRACE("_UpdateLoggerWithEXPConfig: logger(0x%x) added with ETag=%s", pLogger, eTag.c_str());

					// For version 2, the ConfigIDs are under the EventToConfigIds
					for (std::map<std::string, std::string>::const_iterator iter  = eventconfigIds.begin(); iter != eventconfigIds.end(); iter++)
					{						
						TRACE("_UpdateLoggerWithEXPConfig: logger(0x%x) SetEventExperimentIds eventName=%s, eventConfigIds=%s", pLogger, iter->first.c_str(), iter->second.c_str());
						pLoggerCtx->SetEventExperimentIds(iter->first, iter->second);
					}

					// TODO: consider whether we should clean-up/reset the EventToConfigIds mapping previously set on a logger
					// Doing this likely will require a separate API on ISemanticContext
				}
			}

			void ExpCommon::_LogEXPConfigEvent(EventProperties& evtProperties)
			{
				TRACE("_LogEXPConfigEvent[%d]: ExpCommon=0x%x, logger count=%u", __LINE__, this, m_registeredLoggers.size());

				for (std::map<ILogger *, string>::iterator it = m_registeredLoggers.begin(); it != m_registeredLoggers.end(); it++)
				{
					ILogger* pLogger = it->first;
					assert(pLogger != NULL);

					pLogger->LogEvent(evtProperties);
				}
			}
		}
	}
}
