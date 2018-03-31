#define LOG_MODULE DBG_API

#include "ECSClient.hpp"
#include "ECSClientConfig.hpp"
#include "ECSConfigCache.hpp"
#include "ECSClientUtils.hpp"
#include "json.hpp"
#include "../../JsonHelper.hpp"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <memory>
#include <math.h>

using namespace std;
using namespace nlohmann; 
using namespace MAT;
using namespace PAL;


namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace ECS {
				        
				int GetItemIndex(const char* items[], size_t itemCount, size_t itemMaxSize, const char* pItemToFind)
				{
					if ((itemMaxSize > 128) || (strlen(pItemToFind) > itemMaxSize))
					{
						return -1;
					}

					char itemLoweredCase[129];
					strcpy_s(itemLoweredCase, pItemToFind);

					// strlwr does not seem to be available for gcc
					for (size_t i = 0; i < strlen(itemLoweredCase); i++)
					{
						itemLoweredCase[i] = static_cast<char>(tolower(itemLoweredCase[i]));
					}

					for (size_t i = 0; i < itemCount; i++)
					{
						if (strcmp(items[i], itemLoweredCase) == 0)
						{
							return (int)i;
						}
					}

					return -1;
				}

				static int GetDayIndex(const char* pItemToFind)
				{
					const char* days[] =
					{
						"sun",
						"mon",
						"tue",
						"wed",
						"thu",
						"fri",
						"sat",
					};

					return GetItemIndex(days, 7, 3, pItemToFind);
				}

				static int GetMonthIndex(const char* pItemToFind)
				{
					const char* months[] =
					{
						"jan",
						"feb",
						"mar",
						"apr",
						"may",
						"jun",
						"jul",
						"aug",
						"sep",
						"oct",
						"nov",
						"dec",
					};

					return GetItemIndex(months, 12, 3, pItemToFind);
				}


				uint64_t GetTimeFromRFC1123Pattern(const std::string& gmt, struct tm* res)
				{
					struct tm tms = {};
					tms.tm_wday = tms.tm_mon = -1;
					if (!gmt.empty())
					{
						char wday[4] = {};
						char month[4] = {};

						// There should be 7 valid fields scanned. 
						if (7 == sscanf_s(gmt.c_str(), "%3c, %d %3c %d %d:%d:%d", wday, 3, &tms.tm_mday, month, 3,  &tms.tm_year, &tms.tm_hour, &tms.tm_min, &tms.tm_sec))
						{
							tms.tm_wday = GetDayIndex(wday);
							tms.tm_mon = GetMonthIndex(month);

							// tm_year in struct tm is year since 1900
							tms.tm_year -= 1900;
						}
					}
					*res = tms;
					return _mkgmtime(res);
				}

				uint64_t ParseTime(const std::string& time, uint32_t defaultValue/*=0*/)
				{
					tm tms = {};
					uint64_t rfcTime;
					if ((rfcTime = GetTimeFromRFC1123Pattern(time, &tms)) > 0)
					{
						return rfcTime;
					}
					else
					{
						return defaultValue;
					}
				}

                IECSClient* IECSClient::CreateInstance()
                {
                    return (IECSClient*)(new ECSClient());
                }

                void IECSClient::DestroyInstance(IECSClient** ppECSClient)
                {
                    if (ppECSClient != NULL && *ppECSClient != NULL)
                    {
                        delete (ECSClient*)(*ppECSClient);
                        *ppECSClient = NULL;
                    }
                }

                /******************************************************************************
                * ECSClient::ECSClient
                *
                * C'tor
                *
                ******************************************************************************/
                ECSClient::ECSClient()
                    :m_configCache(NULL),
                    m_configActive(NULL),
					m_EXPCommon(this, Retry_Queue_Name),
					m_minExpireTimeInSecs(DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN)					
                {
                    LOG_TRACE("ECSClient c'tor: this=0x%x", this);
                }

                /******************************************************************************
                * ECSClient::~ECSClient
                *
                * D'tor
                *
                ******************************************************************************/
                ECSClient::~ECSClient()
                {
                    LOG_TRACE("EcsClient d'tor: this=0x%x", this);
                     
                    Stop();

                    if (m_configCache != NULL)
                    {
                        delete m_configCache;
                        m_configCache = NULL;
                    }
                }

                /******************************************************************************
                * ECSClient::_ValidateECSClientConfiguration
                *
                * Validate the ECSClientConfiguration struct specified by client application
                *
                ******************************************************************************/
                void ECSClient::_ValidateECSClientConfiguration(const ECSClientConfiguration& config)
                {
                    if (config.clientName.empty() ||
                        config.clientVersion.empty() ||
                        config.cacheFilePathName.empty())
                    {
                        LOG_ERROR("_ValidateECSClientConfiguration: Invalid ECSClientConfiguration specified");
                    }
                }

                /******************************************************************************
                * ECSClient::Initialize
                *
                * Initialize the ECSClient with the specified ECSClientConfiguration
                *
                ******************************************************************************/
                void ECSClient::Initialize(const ECSClientConfiguration& config)
                {
                    // Validate the ECSClientConfiguration struct
                    _ValidateECSClientConfiguration(config);

                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);

                    if (config.serverUrls.empty())
                    {
                        // Use DEFAULT_INT_ECS_SERVER_URL_1 or DEFAULT_INT_ECS_SERVER_URL_2 for test purpose
                        std::string serverUrl1 = CreateServerUrl(DEFAULT_PROD_ECS_SERVER_URL_1, config.clientName, config.clientVersion);
                        LOG_TRACE("Initialize: Added default ECS ServerUrl=%s", serverUrl1.c_str());
						m_EXPCommon.m_serverUrls.push_back(serverUrl1);

                        std::string serverUrl2 = CreateServerUrl(DEFAULT_PROD_ECS_SERVER_URL_2, config.clientName, config.clientVersion);
                        LOG_TRACE("Initialize: Added default ECS ServerUrl=%s", serverUrl2.c_str());
						m_EXPCommon.m_serverUrls.push_back(serverUrl2);
                    }
                    else
                    {
                        for (size_t i = 0; i < config.serverUrls.size(); ++i)
                        {
                            std::string serverUrl = CreateServerUrl(config.serverUrls[i], config.clientName, config.clientVersion);
                            LOG_TRACE("Initialize: Added ECS ServerUrl=%s", serverUrl.c_str());

							m_EXPCommon.m_serverUrls.push_back(serverUrl);
                        }
                    }

                    // Start with random server from the list of servers available
					m_EXPCommon.m_serverUrlIdx = static_cast<unsigned int>(PAL::getUtcSystemTime() % (m_EXPCommon.m_serverUrls.size()));

                    m_configCache = new ECSConfigCache(config.cacheFilePathName);
                    if (!m_configCache)
                    {
                        LOG_ERROR("Initialize: Failed to create local config cache");
                    }

                    m_ecsClientConfiguration = config;

					m_EXPCommon.m_status = EXP_INITIALIZED;
                    LOG_TRACE("Initialize: ECSClient successfully initialized");
                }

                bool ECSClient::AddListener(IECSClientCallback *listener)
                {
                    LOG_TRACE("AddListener[%d]: ECSClient=0x%x, listener=0x%x", __LINE__, this, listener);

                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_smalllock);

                    if (m_listeners.find(listener) == m_listeners.end())
                    {
                        m_listeners.insert(listener);
                        return true;
                    }

                    return false;
                }

                bool ECSClient::RemoveListener(IECSClientCallback *listener)
                {
                    LOG_TRACE("RemoveListener[%d]: ECSClient=0x%x, listener=0x%x", __LINE__, this, listener);

                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_smalllock);

                    if (m_listeners.find(listener) != m_listeners.end())
                    {
                        m_listeners.erase(listener);
                        return true;
                    }

                    return false;
                }

                bool ECSClient::RegisterLogger(ILogger* pLogger, const string& agentName)
                {
                    LOG_TRACE("RegisterLogger[%d]: this=0x%x, ILogger=0x%x, agent=%s", __LINE__, this, pLogger, agentName.c_str());
					
					m_EXPCommon.RegisterLogger(pLogger, agentName);
					                   
                    // Update the logger with the ECS configuration info like Etag if this function is called after ECSClient is started
                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);

					if (m_EXPCommon.m_status == EXP_STARTED || m_EXPCommon.m_status == EXP_SUSPENDED)
					{
						if (m_configActive && m_configActive->etag != DEFAULT_CONFIG_ETAG)
						{
							// update all registered loggers with the ECS configurations infos(such as Etag, configIDs)
							_UpdateLoggerWithEXPConfig(pLogger, agentName);
						}
					}

                    return true;
                }
				void ECSClient::_UpdateLoggerWithEXPConfig(ILogger* pLogger, std::string agentName)
				{
					// update all registered loggers with the EXP configurations infos(such as Etag, configIDs)
					// For version 1 the ConfigIDs are not under the EventToConfigIds mapping.
					// Read V1 value and set it first.
					string configIds = GetSetting(CONFIG_IDS_KEY, agentName, std::string());
					m_EXPCommon._UpdateLoggerWithEXPConfig(pLogger, agentName, m_configActive->etag, configIds);

					// Read V2 value 2nd and set it so that if there are V2 value then
					// the V2 value will override the V1 value.						

					// For version 2, the ConfigIDs are under the EventToConfigIds
					vector<string> events = GetKeys(DEFAULT_EVENT_TO_CONFIGIDS_MAPPING_AGENT_NAME, agentName);
					std::map<std::string, std::string> eventConfigIdMap;

					for (size_t i = 0; i < events.size(); i++)
					{
						if (!events[i].empty())
						{
							string eventConfigIdsPath = agentName;
							eventConfigIdsPath += "/";
							eventConfigIdsPath += events[i];

							string eventConfigIds = GetSetting(DEFAULT_EVENT_TO_CONFIGIDS_MAPPING_AGENT_NAME, eventConfigIdsPath, std::string());
							if (!eventConfigIds.empty())
							{
								LOG_TRACE("_UpdateLoggerWithEXPConfig: SetEventExperimentIds eventName=%s, eventConfigIds=%s", events[i].c_str(), eventConfigIds.c_str());
								eventConfigIdMap[events[i]] = eventConfigIds;
							}
						}
					}
					if (eventConfigIdMap.size() > 0)
					{
						m_EXPCommon._UpdateLoggerWithEXPConfig(pLogger, agentName, m_configActive->etag, eventConfigIdMap);
					}
				}
				void ECSClient::_UpdateLoggersWithEXPConfig()
				{
					for (std::map<ILogger *, string>::iterator it = m_EXPCommon.m_registeredLoggers.begin(); it != m_EXPCommon.m_registeredLoggers.end(); it++)
					{
						_UpdateLoggerWithEXPConfig(it->first, it->second);
					}
				}

                /******************************************************************************
                * ECSClient::Start
                *
                * Start the ECS client
                *
                ******************************************************************************/
                bool ECSClient::Start()
                {
					std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);
                    // check status first, simply return if it hasn't been initialzied or has already started
					if (m_EXPCommon.m_status != EXP_INITIALIZED &&
						m_EXPCommon.m_status != EXP_STOPPED)
					{
                        LOG_ERROR("Start: EcsClient hasn't been initialzied or has already started");
                        return false;
                    }

					// load cached configuration from local cache
					if (!m_configCache->LoadConfig())
					{
						LOG_WARN("Start: Failed to load configurations from local cache");
					}
					else
					{
						// point active config to the default configuration
						m_configActive = m_configCache->GetConfigByRequestName(m_EXPCommon.m_configActiveRequestName);
						if (m_configActive && m_configActive->etag != DEFAULT_CONFIG_ETAG)
						{
                            //save config to next time
                             m_configCache->SaveConfig(*m_configActive);
							// TODO: notify listener if the Etag of the active configuration is different from default
							std::lock_guard<std::mutex> lock(m_EXPCommon.m_smalllock);

							// log event thru all register loggers to indicate that ECS config is updated from local cache
							_LogEXPConfigUpdateEvent(EXP_CUR_SUCCEEDED, EXP_CUS_LOCAL);

							// update all registered loggers with the ECS configurations infos(such as Etag, configIDs)
							_UpdateLoggersWithEXPConfig();

							// notify listners if the active config is either updated on ECS server or changed to a different one
							FireClientEvent(ET_CONFIG_UPDATE_SUCCEEDED, false);
						}
					}

					std::vector<int> backoffTimes;
					for (int index = 0; index < MAX_RETRY_TIMES; index++)
					{
						// back off formula is: 4^retry, ie.
						// 1st retry is of 4^1 = 4 seconds delay,
						// 2nd retry is of 4^2 = 16 seconds delay.
						int backoffTimeInSec = (int)pow((double)DEFAULT_RETRY_TIME_FACTOR, (double)(index + 1));
						backoffTimes.push_back(backoffTimeInSec);
					}

					m_EXPCommon.Start(backoffTimes);

                    // log EVENT_TYPE_ECSCLIENT_STATE_CHANGE event to all registered logger to indicate the ECSClient state 
					_LogEXPCleintStateChangeEvent(EXP_STARTED);

                    LOG_TRACE("Start: EcsClient successfully started");

                    return true;
                }

                /******************************************************************************
                * ECSClient::Resume
                *
                * Resume the ECS client to retrieve configuration from ECS server
                *
                ******************************************************************************/
                bool ECSClient::Resume(bool fetchConfig)
                {
                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);

                    // check status first, simply return if it hasn't been initialzied or has already started
                    if (m_EXPCommon.m_status != EXP_SUSPENDED)
                    {
                        LOG_ERROR("Resume: ExpCommon wasn't suspended");
                        return false;
                    }

                    m_EXPCommon.m_forceRefech = fetchConfig;
					m_EXPCommon.Resume();
					// log EVENT_TYPE_ExpCommon_STATE_CHANGE event to all registered loggers 
					_LogEXPCleintStateChangeEvent(EXP_STARTED);
					return true;
                }

                /******************************************************************************
                * ECSClient::Suspend
                *
                * Suspend the ECS client from retrieving configuration from ECS server
                *
                ******************************************************************************/
                bool ECSClient::Suspend()
                {
                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);
                    // check status first, simply return if it hasn't been initialzied or has already started
                    if (m_EXPCommon.m_status != EXP_STARTED)
                    {
                        LOG_ERROR("Suspend: ExpCommon isn't started");
                        return false;
                    }

					m_EXPCommon.Suspend();
					// log EVENT_TYPE_ExpCommon_STATE_CHANGE event to all registered loggers
					_LogEXPCleintStateChangeEvent(EXP_SUSPENDED);
					return true;
                }

                /******************************************************************************
                * ECSClient::Stop
                *
                * Stop the ECS client
                *
                ******************************************************************************/
                bool ECSClient::Stop()
                {
					std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);

					// check status first, simply return if not started
					if (m_EXPCommon.m_status != EXP_STARTED && m_EXPCommon.m_status != EXP_SUSPENDED)
					{
						LOG_ERROR("Stop: ExpCommon isn't started");
						return false;
					}

					// stop and destroy the offline storage used for local cache of configs
					m_configCache->StopAndDestroyOfflineStorage();

					m_EXPCommon.Stop();
					// log EVENT_TYPE_ExpCommon_STATE_CHANGE event to all registered loggers
					_LogEXPCleintStateChangeEvent(EXP_STOPPED);
					return true;
                }
				              

                /******************************************************************************
                * ECSClient::SetUserId
                *
                * Specify a user Id to be used as the request parameter for retrieving
                * configurations from ECS server. Client can optionally pass this in the request
                * so that configuration can be allocated on a per-user bases. The allocation
                * persists for the user Id therefore good for allocation to follow the user account
                *
                ******************************************************************************/
                bool ECSClient::SetUserId(const string& userId)
                {
                    LOG_TRACE("SetUserId[%d]: ECSClient=0x%x, userId=%u", __LINE__, this, userId.c_str());

                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);

                    if (m_EXPCommon.m_configActiveUserId.compare(userId) == 0)
                    {
                        LOG_ERROR("SetUserId: User Id is the same");
                        return false;
                    }

                    m_EXPCommon.m_configActiveUserId = userId;
					m_EXPCommon.m_configActiveRequestParams[EXPCLIENT_RP_KEY_ID] = m_EXPCommon.m_configActiveUserId;
                    m_EXPCommon.m_configActiveRequestName = m_EXPCommon.GetRequestName(m_ecsClientConfiguration.clientName);

                    // log EVENT_TYPE_ECSCLIENT_STATE_CHANGE event to all registered logger to indicate the ECSClient state 
					_LogEXPCleintStateChangeEvent(EXP_REQUESTPARAMETER_CHANGED);

                    // re-fetch config from ECS server due to a significant context switch like user Id 
                    // but make sure we don't trigger refetch if suspended
                    if (m_EXPCommon.m_status == EXP_STARTED)
                    {
						m_EXPCommon._StopInternal();

                        // dispatch message to asynchronously refetch the active config from ECS server
                        Message message(MT_RELOAD_CONFIG);
                        message.requestName = m_EXPCommon.m_configActiveRequestName;
						m_EXPCommon.m_forceRefech = true;
						m_EXPCommon._DispatchMessage(message);
                    }

                    return true;
                }

                /******************************************************************************
                * ECSClient::SetDeviceId
                *
                * Specify a device Id to be used as the request parameter for retrieving
                * configurations from ECS server. Client can optionally pass this in the request
                * so that configuration can be allocated on a per-device bases. The allocation
                * persists for the device therefore good for allocation that does not need to
                * cross-device and does not depend on the user's login state
                *
                ******************************************************************************/
                bool ECSClient::SetDeviceId(const string& deviceId)
                {
                    LOG_TRACE("SetDeviceId[%d]: ECSClient=0x%x, userId=%u", __LINE__, this, deviceId.c_str());

                    std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_lock);

                    if (m_EXPCommon.m_configActiveDeviceId.compare(deviceId) == 0)
                    {
                        LOG_ERROR("SetDeviceId: Device Id is the same");
                        return false;
                    }

                    m_EXPCommon.m_configActiveDeviceId = deviceId;
					m_EXPCommon.m_configActiveRequestParams[EXPCLIENT_RP_KEY_CLIENTID] = m_EXPCommon.m_configActiveDeviceId;
                    m_EXPCommon.m_configActiveRequestName = m_EXPCommon.GetRequestName(m_ecsClientConfiguration.clientName);

                    // log EVENT_TYPE_ECSCLIENT_STATE_CHANGE event to all registered logger to indicate the ECSClient state 
					_LogEXPCleintStateChangeEvent(EXP_REQUESTPARAMETER_CHANGED);

                    // re-fetch config from ECS server due to a significant context switch like user Id 
                    // but make sure we don't trigger refetch if suspended
					if (m_EXPCommon.m_status == EXP_STARTED)
					{
						m_EXPCommon._StopInternal();

						// dispatch message to asynchronously refetch the active config from ECS server
						Message message(MT_RELOAD_CONFIG);
						message.requestName = m_EXPCommon.m_configActiveRequestName;
						m_EXPCommon.m_forceRefech = true;
						m_EXPCommon._DispatchMessage(message);
					}

                    return true;
                }

                /******************************************************************************
                * ECSClient::SetRequestParameters
                *
                * Set a list of request parameters for retrieve configurations from ECS server
                *
                ******************************************************************************/
                bool ECSClient::SetRequestParameters(const std::map<std::string, std::string>& requestParams)
                {
                    LOG_TRACE("SetRequestParameters[%d]: ECSClient=0x%x, request parameters count=%u", __LINE__, this, requestParams.size());
					
                    m_EXPCommon.SetRequestParameters(requestParams); 
                    
                    // log EVENT_TYPE_ECSCLIENT_STATE_CHANGE event to all registered logger to indicate the ECSClient state 
					_LogEXPCleintStateChangeEvent(EXP_REQUESTPARAMETER_CHANGED); 

                    // Note: we don't consider request parameter change except for userId and deviceId as significant
                    // context switch that should warrant a config refetch from ECS server. Instead we hold off the
                    // refetch till the expiration of current config.

                    return true;
                }


				void ECSClient::FireClientEvent(CommonClientEventType evtType, bool fConfigUpdateFromServer)
				{
					LOG_TRACE("FireClientEvent[%d]:  ECSClient=0x%x, listener count=%u", __LINE__, this, m_listeners.size());

					LOG_TRACE("FireClientEvent[%d]:  EventType=%d, ConfigUpdateFromECS=%d", evtType, fConfigUpdateFromServer);

					// notify listners if the active config is either updated on ECS server or changed to a different one
					IECSClientCallback::ECSClientEventContext evtContext = {};

					evtContext.clientName = m_ecsClientConfiguration.clientName;
					evtContext.clientVersion = m_ecsClientConfiguration.clientVersion;
					evtContext.userId = m_EXPCommon.m_configActiveUserId;
					evtContext.deviceId = m_EXPCommon.m_configActiveDeviceId;
					evtContext.requestParameters = m_EXPCommon.m_configActiveRequestParams;
					evtContext.configExpiryTimeInSec = (unsigned int)m_configActive->GetExpiryTimeInSec();
					evtContext.configUpdateFromECS = fConfigUpdateFromServer;
					
					//pre-condition: m_smalllock is held in caller while this function is called
					//std::lock_guard<std::mutex> lockguard(m_smalllock);
					IECSClientCallback::ECSClientEventType eventTypeLocal = IECSClientCallback::ECSClientEventType::ET_CONFIG_UPDATE_SUCCEEDED;
					if (evtType == CommonClientEventType::ET_CONFIG_UPDATE_FAILED)
					{
						eventTypeLocal = IECSClientCallback::ECSClientEventType::ET_CONFIG_UPDATE_FAILED;
					}

					for (std::set<IECSClientCallback *>::iterator it = m_listeners.begin(); it != m_listeners.end(); it++)
					{
						IECSClientCallback* ecsclientCallback = *it;

						LOG_TRACE("_FireECSClientEvent[%d]:: EcsClient=0x%x, listener=0x%x", __LINE__, this, ecsclientCallback);
						ecsclientCallback->OnECSClientEvent(eventTypeLocal, evtContext);
					}
				}

                /******************************************************************************
                * ECSClient::_HandleHttpCallback
                *
                * Process the HTTPCallback within timer callback
                *
                ******************************************************************************/
                void ECSClient::HandleHttpCallback(
                    Message& msg,
                    bool& isActiveConfigUpdatedOnECS,
                    bool& isActiveConfigUpdatedOnECSSaveNeeded)
                {
                    LOG_TRACE("_HandleHttpCallback: HTTPstack error=%u, HTTP status code=%u",
                        msg.httpstackError, msg.statusCode);

                    isActiveConfigUpdatedOnECS = false;
                    isActiveConfigUpdatedOnECSSaveNeeded = false;

                    switch (msg.statusCode)
                    {
                    case 200:
                    {
                        ECSConfig*  configRequireUpdate = m_configActive;
                        if (m_inProgressRequestName != m_configActive->requestName)
                        {
                            configRequireUpdate = m_configCache->GetConfigByRequestName(m_inProgressRequestName);
                        }
                        // Config retrieved successfully from ECS, update the local cache
                        LOG_TRACE("_HandleHttpCallback: config retrieved from ECS, ETag=%s", msg.headers.get("etag").c_str());
                        configRequireUpdate->expiryUtcTimestamp = getUtcSystemTime() + _GetExpiryTimeInSecFromHeader(msg);

                        configRequireUpdate->etag = msg.headers.get("etag");
                        //Update the active config version after getting new one from ECS server
                        configRequireUpdate->clientVersion = m_ecsClientConfiguration.clientVersion;
                        try
                        {
                            if (msg.body.size() > 0)
                            {
                                configRequireUpdate->configSettings = json::parse(msg.body.c_str());
                            }
                        }
                        catch (...)
                        {
                            LOG_TRACE("Json pasring failed");
                        }

                        // notify listners if the active config is updated on ECS server
                        isActiveConfigUpdatedOnECS = true;
                        isActiveConfigUpdatedOnECSSaveNeeded = true;
                        break;
                    }
                    case 304:
                        // ECS server returns HTTP status code 304 and empty configuration in case it generated 
                        // the same ETag as the one that was passed in the 'If-None-Match' header value of request
                        assert(isActiveConfigUpdatedOnECS == false);

                        // Config settings not modified but the expiry time does, which requires a save to local cache
                        m_configActive->expiryUtcTimestamp = getUtcSystemTime() + _GetExpiryTimeInSecFromHeader(msg);
                        isActiveConfigUpdatedOnECSSaveNeeded = true;
                        break;

                    case 401:
                        // TODO: handle invalid auto token
                        assert(false);
                        break;

                    default:
                        // for all other status code, treat them as error
                        isActiveConfigUpdatedOnECS = true;
                        isActiveConfigUpdatedOnECSSaveNeeded = false;

                        // Next time try a different URL if avail
						m_EXPCommon.m_serverUrlIdx++;
						m_EXPCommon.m_serverUrlIdx%= m_EXPCommon.m_serverUrls.size();
                        break;
                    }
                    m_inProgressRequestName = "";
                }

                /******************************************************************************
                * ECSClient::_HandleConfigReload
                *
                * Reload from local cache or issue condig refetch request within timer callback
                *
                ******************************************************************************/
                void ECSClient::HandleConfigReload(
                    Message& msg,
                    bool& isActiveConfigSwitched,
                    bool& isActiveConfigSwitchedSaveNeeded)
                {
                    LOG_TRACE("_HandleConfigReloadAndRefetch: Reload/Re-fetch config for RequestName=%s", msg.requestName.c_str());

                    if (m_configCache)
                    {
                        assert(msg.type == MT_RELOAD_CONFIG);

                        isActiveConfigSwitched = false;
                        isActiveConfigSwitchedSaveNeeded = false;

                        // Reload config from local cache and set it as active config if necessary
                        ECSConfig* pConfig = m_configCache->GetConfigByRequestName(msg.requestName);
                        if (pConfig == NULL)
                        {
                            
                            unsigned int timeoutinSec = m_ecsClientConfiguration.defaultExpiryTimeInMin * 60;
                            if (timeoutinSec == 0)
                            {
                                timeoutinSec = m_minExpireTimeInSecs;
                            }
                            ECSConfig config;
                            config.expiryUtcTimestamp = PAL::getUtcSystemTime() + timeoutinSec;
                            config.requestName = msg.requestName;

                            pConfig = m_configCache->AddConfig(config);
                        }

                        // Update the active config if necessary
                        if (m_configActive != pConfig)
                        {
                            m_configActive = pConfig;
                            m_EXPCommon.m_configActiveRequestName = pConfig->requestName;

                            isActiveConfigSwitched = true;
                            isActiveConfigSwitchedSaveNeeded = true;
                        }
                    }
                }

				/******************************************************************************
				* ECSClient::HandleConfigSave
				*
				* Reload from local cache or issue condig refetch request within timer callback
				*
				******************************************************************************/
				void ECSClient::HandleConfigSave(bool isActiveConfigSwitchedSaveNeeded, bool isActiveConfigUpdatedOnEXPSaveNeeded)
				{
					// The save config is pretty time-consuming, we need save it outside the m_smalllock
                    if (m_configCache && (isActiveConfigSwitchedSaveNeeded || isActiveConfigUpdatedOnEXPSaveNeeded) && m_configActive->etag != DEFAULT_CONFIG_ETAG)
					{
						m_configCache->SaveConfig(*m_configActive);
					}
				}

				/******************************************************************************
				* ECSClient::HandleUpdateClient
				*
				* Reload from local cache or issue condig refetch request within timer callback
				*
				******************************************************************************/
				void ECSClient::HandleUpdateClient(bool isActiveConfigSwitched, bool isActiveConfigUpdatedOnEXP, bool isActiveConfigUpdatedOnEXPSaveNeeded)
				{
                    if (m_EXPCommon.m_status != EXP_STARTED)
                    {
                        return;
                    }
					if (isActiveConfigSwitched || isActiveConfigUpdatedOnEXP)
					{
						if (isActiveConfigSwitched)
						{
							if (m_configActive->etag != DEFAULT_CONFIG_ETAG)
							{
								// update all registered loggers with the EXP configurations infos(such as Etag, configIDs)
								_UpdateLoggersWithEXPConfig();

								// log event to indicate that EXP config is updated from local cache
								_LogEXPConfigUpdateEvent(EXP_CUR_SUCCEEDED, EXP_CUS_LOCAL);

								// fire event to notify all listeners of this active config change
								FireClientEvent(CommonClientEventType::ET_CONFIG_UPDATE_SUCCEEDED, false);
							}
						}
						else
						{
							assert(isActiveConfigUpdatedOnEXP);

							if (isActiveConfigUpdatedOnEXPSaveNeeded)
							{
								assert(m_configActive->etag != DEFAULT_CONFIG_ETAG);

								// update all registered loggers with the EXP configurations infos(such as Etag, configIDs)
								_UpdateLoggersWithEXPConfig();

								// log event to indicate that EXP config is updated from local cache
								_LogEXPConfigUpdateEvent(EXP_CUR_SUCCEEDED, EXP_CUS_SERVER);

								// fire event to notify all listeners of this active config change
								FireClientEvent(CommonClientEventType::ET_CONFIG_UPDATE_SUCCEEDED, true);
							}
							else
							{
								// Fetch from EXP server failed
								// log event to indicate that EXP config is updated from local cache
								_LogEXPConfigUpdateEvent(EXP_CUR_FAILED, EXP_CUS_SERVER);

								// fire event to notify all listeners of this active config change
								FireClientEvent(CommonClientEventType::ET_CONFIG_UPDATE_FAILED, true);
							}
						}
					}
				}

				bool ECSClient::FetchFromServerIfRequired()
				{
					// Check if the active config has expired or not, refetch config from EXP if expired.
					// Otherwise set a timer to expire it and notify all listeners if it has now changed.
					unsigned int expiryTimeInSec = static_cast<unsigned int>(m_configActive->GetExpiryTimeInSec());
					if (expiryTimeInSec == 0 ||
						m_EXPCommon.m_forceRefech ||
						m_ecsClientConfiguration.clientVersion != m_configActive->clientVersion)   // UI version has changed, we need to fetch new one.
					{
						m_EXPCommon.m_forceRefech = false;
						// if active config has expired or client chnaged version, refetch it from EXP server immediately

						std::string url = m_EXPCommon.m_serverUrls.at(m_EXPCommon.m_serverUrlIdx);

                        m_inProgressRequestName = m_configActive->requestName;
						m_EXPCommon.SendRequestAsync(url);
						LOG_TRACE("_HandleConfigReloadAndRefetch: Config refetch request successfully sent to EXP.");
						return true;
					}
					return false;
				}

				json ECSClient::GetActiveConfigVariant()
				{
					return (m_configActive != NULL) ? m_configActive->configSettings : json();
				}

				void ECSClient::_LogEXPConfigUpdateEvent(EXPConfigUpdateResult result, EXPConfigUpdateSource source)
				{
                    if (m_ecsClientConfiguration.enableECSClientTelemetry)
                    {
                        //pre-condition: m_smalllock is held in caller while this function is called
                        //std::lock_guard<std::mutex> lockguard(m_smalllock);

                        EventProperties evtProperties(EVENT_TYPE_ECSCLIENT_CONFIG_UPDATE);

                        evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTNAME, m_ecsClientConfiguration.clientName);
                        evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTVERSION, m_ecsClientConfiguration.clientVersion);
                        evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CONFIG_RESULT, ExpCommon::EXPConfigUpdateResult2STR[result]);
                        evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CONFIG_SOURCE, ExpCommon::EXPConfigUpdateSource2STR[source]);

                        m_EXPCommon._LogEXPConfigEvent(evtProperties);
                    }
				}

				void ECSClient::_LogEXPCleintStateChangeEvent(EXPClientStatus status)
				{
                    if (m_ecsClientConfiguration.enableECSClientTelemetry)
                    {
                        std::lock_guard<std::mutex> lockguard(m_EXPCommon.m_smalllock);

                        EventProperties evtProperties(EVENT_TYPE_ECSCLIENT_STATE_CHANGE);

                        evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTNAME, m_ecsClientConfiguration.clientName);
                        evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTVERSION, m_ecsClientConfiguration.clientVersion);

                        evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_STATE, ExpCommon::EXPClientStatus2STR[status]);

                        m_EXPCommon._LogEXPConfigEvent(evtProperties);
                    }
				}

                void ECSClient::SetRetryTimeFactor(int time)
                {
                    m_EXPCommon.m_retryTimeFactor = time;
                }

				unsigned int ECSClient::GetExpiryTimeInSec()
				{
                    int64_t value = m_configActive->GetExpiryTimeInSec();
                    if (0 == value)
                    {
                        value = m_minExpireTimeInSecs;
                    }

                    return static_cast<unsigned int>(value);
                }
                              
                json ECSClient::_GetActiveConfigVariant()
                {
                    return (m_configActive != NULL) ? m_configActive->configSettings : json();
                }

                // IECSClient APIs
                string ECSClient::GetETag()
                {
                    return (m_configActive != NULL) ? m_configActive->etag : "";
                }

                string ECSClient::GetConfigs()
                {
                    return (m_configActive != NULL) ? m_configActive->configSettings.dump() : "";
                }

                std::vector<std::string> ECSClient::GetKeys(
                    const std::string& agentName,
                    const std::string& keysPath)
                {
                    string fullPath = JsonHelper::Combine(agentName, keysPath, '/');

                    return JsonHelper::GetKeys(_GetActiveConfigVariant(), fullPath);
                }

                bool ECSClient::TryGetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    std::string& value)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::TryGetValueString(_GetActiveConfigVariant(), fullPath, value);
                }

                bool ECSClient::TryGetBoolSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    bool& value)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::TryGetValueBool(_GetActiveConfigVariant(), fullPath, value);
                }

                bool ECSClient::TryGetIntSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    int& value)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::TryGetValueInt(_GetActiveConfigVariant(), fullPath, value);
                }

                bool ECSClient::TryGetLongSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    long& value)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::TryGetValueLong(_GetActiveConfigVariant(), fullPath, value);
                }

                bool ECSClient::TryGetDoubleSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    double& value)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::TryGetValueDouble(_GetActiveConfigVariant(), fullPath, value);
                }

                string ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const std::string& defaultValue)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::GetValueString(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                bool ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const bool defaultValue)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::GetValueBool(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                int ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const int defaultValue)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::GetValueInt(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                double ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const double defaultValue)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::GetValueDouble(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                std::vector<std::string> ECSClient::GetSettings(
                    const std::string& agentName,
                    const std::string& settingPath)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::GetValuesString(_GetActiveConfigVariant(), fullPath);
                }

                std::vector<int> ECSClient::GetSettingsAsInts(
                    const std::string& agentName,
                    const std::string& settingPath)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::GetValuesInt(_GetActiveConfigVariant(), fullPath);
                }

                std::vector<double> ECSClient::GetSettingsAsDbls(
                    const std::string& agentName,
                    const std::string& settingPath)
                {
                    string fullPath = JsonHelper::Combine(agentName, settingPath, '/');

                    return JsonHelper::GetValuesDouble(_GetActiveConfigVariant(), fullPath);
                }

                std::int64_t ECSClient::_GetExpiryTimeInSecFromHeader(Message& msg)
                {
                    std::string expireInHeaderStr = msg.headers.get("expires");
                    std::string dateInHeaderStr = msg.headers.get("date");

                    if (expireInHeaderStr.empty() || dateInHeaderStr.empty())
                    {
                        LOG_WARN("_GetExpiryTimeInSecFromHeader: Expires time or date in response header is empty, use the default expiry time.");
                        return DEFAULT_CONFIG_REFETCH_INTERVAL_IN_SECONDS;
                    }

                    std::int64_t expireInHeader = ParseTime(expireInHeaderStr, 0);
                    std::int64_t dateInHeader = ParseTime(dateInHeaderStr, 0);

                    if (expireInHeader == 0 || dateInHeader == 0) //Time Parse failure
                    {
						LOG_WARN("_GetExpiryTimeInSecFromHeader: Parse expiry time or date error(Expires=%s, Date=%s), use the default expiry time.",
                            expireInHeaderStr.c_str(), dateInHeaderStr.c_str());
                        return DEFAULT_CONFIG_REFETCH_INTERVAL_IN_SECONDS;
                    }

                    std::int64_t expireRelative = expireInHeader - dateInHeader;

                    //make sure the relative expire time is no less than m_minExpireTimeInSecs
                    if (expireRelative < m_minExpireTimeInSecs)
                    {
                        LOG_WARN("_GetExpiryTimeInSecFromHeader: Expires time(%ld) from response header is less than min limit(%ld sec), use min.",
                            expireRelative, m_minExpireTimeInSecs);
                        expireRelative = m_minExpireTimeInSecs;
                    }

                    //make sure the relative expire time is no more than DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX
                    if (expireRelative > DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX)
                    {
						LOG_WARN("_GetExpiryTimeInSecFromHeader: Expire time(%ld) from response header is more than max limit(%ld sec), use max.",
                            expireRelative, DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX);
                        expireRelative = DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX;
                    }

                    return expireRelative;
                }

            }
        }
    }
}