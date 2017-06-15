#define LOG_MODULE DBG_API

#pragma unmanaged

#include "ECSClient.hpp"
#include "ECSClientConfig.hpp"
#include "ECSConfigCache.hpp"
#include "ECSClientUtils.hpp"

#include "common/PalSingleton.hpp"
#include "common/TraceHelper.hpp"
#include "common/Misc.hpp"
#include "common/JsonHelper.hpp"

#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <memory>
#include <math.h>

using namespace std;
using namespace nlohmann; 
using namespace common;
using namespace Microsoft::Applications::Telemetry;
using namespace Microsoft::Applications::Telemetry::PlatformAbstraction;

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace ECS {
				                
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
                    TRACE("ECSClient c'tor: this=0x%x", this);
                }

                /******************************************************************************
                * ECSClient::~ECSClient
                *
                * D'tor
                *
                ******************************************************************************/
                ECSClient::~ECSClient()
                {
                    TRACE("EcsClient d'tor: this=0x%x", this);

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
                        ECS_THROW(ECS_ERROR_INVALID_CONFIG);
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

                    LOCKGUARD(m_EXPCommon.m_lock);

                    if (config.serverUrls.empty())
                    {
                        // Use DEFAULT_INT_ECS_SERVER_URL_1 or DEFAULT_INT_ECS_SERVER_URL_2 for test purpose
                        std::string serverUrl1 = CreateServerUrl(DEFAULT_PROD_ECS_SERVER_URL_1, config.clientName, config.clientVersion);
                        TRACE("Initialize: Added default ECS ServerUrl=%s", serverUrl1.c_str());
						m_EXPCommon.m_serverUrls.push_back(serverUrl1);

                        std::string serverUrl2 = CreateServerUrl(DEFAULT_PROD_ECS_SERVER_URL_2, config.clientName, config.clientVersion);
                        TRACE("Initialize: Added default ECS ServerUrl=%s", serverUrl2.c_str());
						m_EXPCommon.m_serverUrls.push_back(serverUrl2);
                    }
                    else
                    {
                        for (size_t i = 0; i < config.serverUrls.size(); ++i)
                        {
                            std::string serverUrl = CreateServerUrl(config.serverUrls[i], config.clientName, config.clientVersion);
                            TRACE("Initialize: Added ECS ServerUrl=%s", serverUrl.c_str());

							m_EXPCommon.m_serverUrls.push_back(serverUrl);
                        }
                    }

                    // Start with random server from the list of servers available
					m_EXPCommon.m_serverUrlIdx = static_cast<unsigned int>(common::GetCurrentTimeStamp() % (m_EXPCommon.m_serverUrls.size()));

                    m_configCache = new ECSConfigCache(config.cacheFilePathName);
                    if (!m_configCache)
                    {
                        LOG_ERROR("Initialize: Failed to create local config cache");
                        ECS_THROW(ECS_ERROR_OUT_OF_MEMORY);
                    }

                    m_ecsClientConfiguration = config;

					m_EXPCommon.m_status = EXP_INITIALIZED;
                    TRACE("Initialize: ECSClient successfully initialized");
                }

                bool ECSClient::AddListener(IECSClientCallback *listener)
                {
                    TRACE("AddListener[%d]: ECSClient=0x%x, listener=0x%x", __LINE__, this, listener);

                    LOCKGUARD(m_EXPCommon.m_smalllock);

                    if (m_listeners.find(listener) == m_listeners.end())
                    {
                        m_listeners.insert(listener);
                        return true;
                    }

                    return false;
                }

                bool ECSClient::RemoveListener(IECSClientCallback *listener)
                {
                    TRACE("RemoveListener[%d]: ECSClient=0x%x, listener=0x%x", __LINE__, this, listener);

                    LOCKGUARD(m_EXPCommon.m_smalllock);

                    if (m_listeners.find(listener) != m_listeners.end())
                    {
                        m_listeners.erase(listener);
                        return true;
                    }

                    return false;
                }

                bool ECSClient::RegisterLogger(ILogger* pLogger, const string& agentName)
                {
                    TRACE("RegisterLogger[%d]: this=0x%x, ILogger=0x%x, agent=%s", __LINE__, this, pLogger, agentName.c_str());
					
					m_EXPCommon.RegisterLogger(pLogger, agentName);
					                   
                    // Update the logger with the ECS configuration info like Etag if this function is called after ECSClient is started
                    LOCKGUARD(m_EXPCommon.m_lock);

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
								TRACE("_UpdateLoggerWithEXPConfig: SetEventExperimentIds eventName=%s, eventConfigIds=%s", events[i].c_str(), eventConfigIds.c_str());
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
					LOCKGUARD(m_EXPCommon.m_lock);
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
							// TODO: notify listener if the Etag of the active configuration is different from default
							LOCKGUARD(m_EXPCommon.m_smalllock);

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

                    RetryTimes retryTimes;
                    retryTimes.name = Retry_Queue_Name;
                    retryTimes.timeOuts = backoffTimes;
					m_EXPCommon.Start(retryTimes);

                    // log EVENT_TYPE_ECSCLIENT_STATE_CHANGE event to all registered logger to indicate the ECSClient state 
					_LogEXPCleintStateChangeEvent(EXP_STARTED);

                    TRACE("Start: EcsClient successfully started");

                    return true;
                }

                /******************************************************************************
                * ECSClient::Resume
                *
                * Resume the ECS client to retrieve configuration from ECS server
                *
                ******************************************************************************/
                bool ECSClient::Resume()
                {
                    LOCKGUARD(m_EXPCommon.m_lock);

                    // check status first, simply return if it hasn't been initialzied or has already started
                    if (m_EXPCommon.m_status != EXP_SUSPENDED)
                    {
                        LOG_ERROR("Resume: ExpCommon wasn't suspended");
                        return false;
                    }

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
                    LOCKGUARD(m_EXPCommon.m_lock);
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
					LOCKGUARD(m_EXPCommon.m_lock);

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
                    TRACE("SetUserId[%d]: ECSClient=0x%x, userId=%u", __LINE__, this, userId.c_str());

                    LOCKGUARD(m_EXPCommon.m_lock);

                    if (m_configActiveUserId.compare(userId) == 0)
                    {
                        LOG_ERROR("SetUserId: User Id is the same");
                        return false;
                    }

                    m_configActiveUserId = userId;
					m_EXPCommon.m_configActiveRequestParams[EXPCLIENT_RP_KEY_ID] = m_configActiveUserId;
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
                    TRACE("SetDeviceId[%d]: ECSClient=0x%x, userId=%u", __LINE__, this, deviceId.c_str());

                    LOCKGUARD(m_EXPCommon.m_lock);

                    if (m_configActiveDeviceId.compare(deviceId) == 0)
                    {
                        LOG_ERROR("SetDeviceId: Device Id is the same");
                        return false;
                    }

                    m_configActiveDeviceId = deviceId;
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
                    TRACE("SetRequestParameters[%d]: ECSClient=0x%x, request parameters count=%u", __LINE__, this, requestParams.size());
					
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
					TRACE("FireClientEvent[%d]:  ECSClient=0x%x, listener count=%u", __LINE__, this, m_listeners.size());

					TRACE("FireClientEvent[%d]:  EventType=%d, ConfigUpdateFromECS=%d", evtType, fConfigUpdateFromServer);

					// notify listners if the active config is either updated on ECS server or changed to a different one
					IECSClientCallback::ECSClientEventContext evtContext = {};

					evtContext.clientName = m_ecsClientConfiguration.clientName;
					evtContext.clientVersion = m_ecsClientConfiguration.clientVersion;
					evtContext.userId = m_configActiveUserId;
					evtContext.deviceId = m_configActiveDeviceId;
					evtContext.requestParameters = m_EXPCommon.m_configActiveRequestParams;
					evtContext.configExpiryTimeInSec = (unsigned int)m_configActive->GetExpiryTimeInSec();
					evtContext.configUpdateFromECS = fConfigUpdateFromServer;
					
					//pre-condition: m_smalllock is held in caller while this function is called
					//LOCKGUARD(m_smalllock);
					IECSClientCallback::ECSClientEventType eventTypeLocal = IECSClientCallback::ECSClientEventType::ET_CONFIG_UPDATE_SUCCEEDED;
					if (evtType == CommonClientEventType::ET_CONFIG_UPDATE_FAILED)
					{
						eventTypeLocal = IECSClientCallback::ECSClientEventType::ET_CONFIG_UPDATE_FAILED;
					}

					for (std::set<IECSClientCallback *>::iterator it = m_listeners.begin(); it != m_listeners.end(); it++)
					{
						IECSClientCallback* ecsclientCallback = *it;

						TRACE("_FireECSClientEvent[%d]:: EcsClient=0x%x, listener=0x%x", __LINE__, this, ecsclientCallback);
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
                    TRACE("_HandleHttpCallback: HTTPstack error=%u, HTTP status code=%u, HCM RetryFailedTimes=%u",
                        msg.httpstackError, msg.statusCode, msg.retryFailedTimes);

                    isActiveConfigUpdatedOnECS = false;
                    isActiveConfigUpdatedOnECSSaveNeeded = false;

                    if (msg.userData != m_configActive)
                    {
                        TRACE("_HandleHttpCallback: HTTP response received is not targeted for the current active config, ignored");
                        return;
                    }

                    switch (msg.statusCode)
                    {
                    case 200:
                        // Config retrieved successfully from ECS, update the local cache
                        TRACE("_HandleHttpCallback: config retrieved from ECS, ETag=%s", msg.headers["ETag"].c_str());
                        m_configActive->expiryUtcTimestamp = common::GetCurrentTimeStamp() + _GetExpiryTimeInSecFromHeader(msg);

                        m_configActive->etag = msg.headers["ETag"];
                        //Update the active config version after getting new one from ECS server
                        m_configActive->clientVersion = m_ecsClientConfiguration.clientVersion;
						m_configActive->configSettings = json::parse(msg.body.c_str());

                        // notify listners if the active config is updated on ECS server
                        isActiveConfigUpdatedOnECS = true;
                        isActiveConfigUpdatedOnECSSaveNeeded = true;
                        break;

                    case 304:
                        // ECS server returns HTTP status code 304 and empty configuration in case it generated 
                        // the same ETag as the one that was passed in the 'If-None-Match' header value of request
                        assert(isActiveConfigUpdatedOnECS == false);

                        // Config settings not modified but the expiry time does, which requires a save to local cache
                        m_configActive->expiryUtcTimestamp = common::GetCurrentTimeStamp() + _GetExpiryTimeInSecFromHeader(msg);
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
                    TRACE("_HandleConfigReloadAndRefetch: Reload/Re-fetch config for RequestName=%s", msg.requestName.c_str());

                    if (m_configCache)
                    {
                        assert(msg.type == MT_RELOAD_CONFIG);

                        isActiveConfigSwitched = false;
                        isActiveConfigSwitchedSaveNeeded = false;

                        // Reload config from local cache and set it as active config if necessary
                        ECSConfig* pConfig = m_configCache->GetConfigByRequestName(msg.requestName);
                        if (pConfig == NULL)
                        {
                            ECSConfig config;
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

						if (m_EXPCommon.m_httpClientManager->SendRequestAsync(IRequest::GET, url.c_str(),
							m_EXPCommon.m_configActiveHeaders,
							m_EXPCommon.m_configActiveRequestParams,
							NULL,
							0,
							m_configActive,
                            vector<string>({""}),
							-1,
							true,
							&m_EXPCommon,
							Retry_Queue_Name,
							Retry_Queue_Name))
						{
							TRACE("_HandleConfigReloadAndRefetch: Config refetch request successfully sent to EXP.");
						}
						else
						{
							LOG_ERROR("_HandleConfigReloadAndRefetch: Failed to send config refetch request to EXP");
						}
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
					//pre-condition: m_smalllock is held in caller while this function is called
					//LOCKGUARD(m_smalllock);

					EventProperties evtProperties(EVENT_TYPE_ECSCLIENT_CONFIG_UPDATE);

					evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTNAME, m_ecsClientConfiguration.clientName);
					evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTVERSION, m_ecsClientConfiguration.clientVersion);
					evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CONFIG_RESULT, ExpCommon::EXPConfigUpdateResult2STR[result]);
					evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CONFIG_SOURCE, ExpCommon::EXPConfigUpdateSource2STR[source]);

					m_EXPCommon._LogEXPConfigEvent(evtProperties);
				}

				void ECSClient::_LogEXPCleintStateChangeEvent(EXPClientStatus status)
				{

					LOCKGUARD(m_EXPCommon.m_smalllock);

					EventProperties evtProperties(EVENT_TYPE_ECSCLIENT_STATE_CHANGE);

					evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTNAME, m_ecsClientConfiguration.clientName);
					evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_CLIENTVERSION, m_ecsClientConfiguration.clientVersion);

					evtProperties.SetProperty(EVENT_FIELD_ECSCLIENT_STATE, ExpCommon::EXPClientStatus2STR[status]);

					m_EXPCommon._LogEXPConfigEvent(evtProperties);
				}


				unsigned int ECSClient::GetExpiryTimeInSec()
				{
					return static_cast<unsigned int>(m_configActive->GetExpiryTimeInSec());
				}
                              
                json ECSClient::_GetActiveConfigVariant()
                {
                    return (m_configActive != NULL) ? m_configActive->configSettings : json();
                }

                // IECSClient APIs
                string ECSClient::GetETag()
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    return (m_configActive != NULL) ? m_configActive->etag : "";
                }

                std::vector<std::string> ECSClient::GetKeys(
                    const std::string& agentName,
                    const std::string& keysPath)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, keysPath, '/');

                    return common::JsonHelper::GetKeys(_GetActiveConfigVariant(), fullPath);
                }

                string ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const std::string& defaultValue)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, settingPath, '/');

                    return common::JsonHelper::GetValueString(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                bool ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const bool defaultValue)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, settingPath, '/');

                    return common::JsonHelper::GetValueBool(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                int ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const int defaultValue)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, settingPath, '/');

                    return common::JsonHelper::GetValueInt(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                double ECSClient::GetSetting(
                    const std::string& agentName,
                    const std::string& settingPath,
                    const double defaultValue)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, settingPath, '/');

                    return common::JsonHelper::GetValueDouble(_GetActiveConfigVariant(), fullPath, defaultValue);
                }

                std::vector<std::string> ECSClient::GetSettings(
                    const std::string& agentName,
                    const std::string& settingPath)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, settingPath, '/');

                    return common::JsonHelper::GetValuesString(_GetActiveConfigVariant(), fullPath);
                }

                std::vector<int> ECSClient::GetSettingsAsInts(
                    const std::string& agentName,
                    const std::string& settingPath)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, settingPath, '/');

                    return common::JsonHelper::GetValuesInt(_GetActiveConfigVariant(), fullPath);
                }

                std::vector<double> ECSClient::GetSettingsAsDbls(
                    const std::string& agentName,
                    const std::string& settingPath)
                {
					//LOCKGUARD(m_EXPCommon.m_smalllock);

                    string fullPath = common::Combine(agentName, settingPath, '/');

                    return common::JsonHelper::GetValuesDouble(_GetActiveConfigVariant(), fullPath);
                }

                std::int64_t ECSClient::_GetExpiryTimeInSecFromHeader(Message& msg)
                {
                    std::string expireInHeaderStr = msg.headers["Expires"];
                    std::string dateInHeaderStr = msg.headers["Date"];

                    if (expireInHeaderStr.empty() || dateInHeaderStr.empty())
                    {
                        LOG_WARN("_GetExpiryTimeInSecFromHeader: Expires time or date in response header is empty, use the default expiry time.");
                        return DEFAULT_CONFIG_REFETCH_INTERVAL_IN_SECONDS;
                    }

                    std::int64_t expireInHeader = common::ParseTime(expireInHeaderStr);
                    std::int64_t dateInHeader = common::ParseTime(dateInHeaderStr);

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