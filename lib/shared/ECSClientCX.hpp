//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "mat/config.h"
#ifdef HAVE_MAT_EXP

#include "PlatformHelpers.h"

#include "ctmacros.hpp"
#include "LoggerCX.hpp"

#include "IECSClient.hpp"

namespace MATW_NS_BEGIN {

    class ECSClientCallbackProxy;

    // struct used to confiure the ECSClient
    public value class ECSClientConfiguration sealed
    {
    public:
        // [required] Name of the client whose the ECS configurations are to be retrieved
        String ^ clientName;

        // [required] Version of the client whose the ECS configurations are to be retrieved
        String^ clientVersion;

        // [required] Path and name of the file to be used by ECS client to cache configuration locally
        String^ cacheFilePathName;

        // [optional] default time in mins to expire the configuration cached locally
        unsigned int defaultExpiryTimeInMin;

        // [optional] ECS server URLs, default will be used if not specified
        String^ serverUrls;
    };

    public value class ECSClientEventContext sealed
    {
    public:
        String ^ clientName;
        String^ clientVersion;
        String^ userId;
        String^ deviceId;
        String^ requestParameters;
        unsigned int configExpiryTimeInSec;
        bool configUpdateFromECS;
    };

    public enum class ECSClientEventType : int
    {
        ET_CONFIG_UPDATE_SUCCEEDED = 0,
        ET_CONFIG_UPDATE_FAILED
    };

    // Callback interface for ECSClient to notify its listener of events occurred
    public interface class IECSClientCallback
    {
    public:
        /// <summary>
        /// Callback function for ECSClient to notify its listener of any configuration change
        /// </summary>
        /// <param name="evtType">type of the ECSClient event</param>
        /// <param name="evtContext">context information of the ECSClient event</param>
        void OnECSClientEvent(ECSClientEventType evtType, ECSClientEventContext evtContext);
    };

    public ref class ECSClient sealed
    {
    private:
        MAEE::IECSClient *m_ecsClient;
        std::map<std::string, std::string> m_requestParams;

        std::vector<std::unique_ptr<ECSClientCallbackProxy>> m_listeners_native;
        std::vector<std::string> splitString(std::string str, char delimiter);

    public:
        ECSClient();
        virtual ~ECSClient();

        /// <summary>
        /// Initialize the ECSClient with the specified configuration
        /// </summary>
        /// <param name="config">Configuration of the ECSClient</param>
        void Initialize(ECSClientConfiguration config);

        /// <summary>
        /// Add a listner to the ECSClient to be notified of any configuration changes
        /// </summary>
        /// <param name="listener">listener to be added to the ECSClient</param>
        /// <return>true if listener is added successfully, false otherwise</return>
        bool AddListener(IECSClientCallback^ listener);

        /// <summary>
        /// Remove the listner to stop receiving notification from the ECSClient
        /// </summary>
        /// <param name="listener">listener to be removed from the ECSClient</param>
        /// <return>true if listener is removed successfully, false otherwise</return>
        bool RemoveListener(IECSClientCallback^ listener);

        /// <summary>
        /// Register a logger to auto-tag events sent by the logger with ECS configuration information, like Etag.
        /// </summary>
        /// <param name="pLoger">logger to be registered with the ECS client</param>
        /// <param name="agentName">name of the agent whose experiments configIds will be auto-tagged to event sent by the logger</param>
        /// <return>true if logger is registered successfully, false otherwise</return>
        bool RegisterLogger(MATW::ILogger^ pLoger, String^ agentName);

        /// <summary>
        /// Specify a user Id to be used as the request parameter for retrieving configurations from ECS server
        /// Client can optionally pass this in the request so that configuration can be allocated on a per-user bases.
        /// The allocation persists for the user Id therefore good for allocation to follow the user account
        /// </summary>
        /// <param name="userId">login user Id to pass in the request</param>
        /// <return>true if it is set successfully, false otherwise</return>
        bool SetUserId(String^ userId);

        /// <summary>
        /// Specify a device Id to be used as the request parameter for retrieving configurations from ECS server
        /// Client can optionally pass this in the request so that configuration can be allocated on a per-device bases.
        /// The allocation persists for the device therefore good for allocation that does not need to cross-device 
        /// and does not depend on the user's login state
        /// </summary>
        /// <param name="deviceId">Device Id to pass in the request</param>
        /// <return>true if it is set successfully, false otherwise</return>
        bool SetDeviceId(String^ deviceId);

        /// <summary>
        /// Clear custom parameters of the request to use to retrieve configurations from ECS server
        /// </summary>
        void ClearRequestParams();

        /// <summary>
        /// Specify a custom parameter for the request to use to retrieve configurations from ECS server
        /// </summary>
        /// <param name="requestParam">parameter name and value</param>
        /// <return>true if parameter is set successfully, false otherwise</return>
        bool SetRequestParameter(String^ name, String^ value);

        /// <summary>
        /// Start the ECSClient to retrive configurations from ECS server
        /// </summary>
        /// <return>true if ECSClient is started successfully, false otherwise</return>
        bool Start();

        /// <summary>
        /// Stop the ECSClient to retrive configurations from ECS server
        /// </summary>
        /// <return>true if ECSClient is stopped successfully, false otherwise</return>
        bool Stop();

        /// <summary>
        /// Suspend the ECSClient to retrive configuration updates from ECS server
        /// </summary>
        /// <return>true if ECSClient is suspend successfully, false otherwise</return>
        bool Suspend();

        /// <summary>
        /// Resume the ECSClient to retrive configuration updates from ECS server
        /// </summary>
        /// <return>true if ECSClient is resumed successfully, false otherwise</return>
        bool Resume(bool fetchConfig);

        /// <summary>
        /// Get the ETag of the current active ECS configuration
        /// </summary>
        /// <return>ETag String</return>
        String^ GetETag();

        /// <summary>
        /// Get the ETag of the current active ECS configuration
        /// </summary>
        /// <return>ETag String</return>
        String^ GetConfigs();

        /// <summary>
        /// Get all the keys under the configuration path of the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration keys</param>
        /// <param name="keysPath">configuration path to retrieve keys</param>
        /// <return>list of configuration keys</return>
        IPlatformVector^ GetKeys(String^ agentName, String^ keysPath);
#ifdef WIN10_CS
        [::Windows::Foundation::Metadata::DefaultOverloadAttribute]
#endif
        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as String, return default if no configuration setting found</return>
        String^ GetSetting(String^ agentName, String^ settingPath, String^ defaultValue);

        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as bool, return default if no configuration setting found</return>
        bool GetSetting(String^ agentName, String^ settingPath, bool defaultValue);

        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as integer, return default if no configuration setting found</return>
        int GetSetting(String^ agentName, String^ settingPath, int defaultValue);

        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as double, return default if no configuration setting found</return>
        double GetSetting(String^ agentName, String^ settingPath, double defaultValue);

        /// <summary>
        /// Get the list of settings of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration settings</param>
        /// <param name="settingPath">configuration path to retrieve settings</param>
        /// <return>list of settings as String</return>
        IPlatformVector^ GetSettings(String^ agentName, String^ settingPath);
    };

    class ECSClientCallbackProxy : public MAEE::IECSClientCallback {
    public:
        ECSClientCallbackProxy(MATW::IECSClientCallback^ listener);
        virtual void OnECSClientEvent(MAEE::IECSClientCallback::ECSClientEventType evtType, MAEE::IECSClientCallback::ECSClientEventContext evtContext);
        bool compare(MATW::IECSClientCallback^ listener);
    protected:
#ifdef _WINRT_DLL
        MATW::IECSClientCallback^ listener;
#else
        gcroot<MATW::IECSClientCallback^> listener;
#endif
    };

} MATW_NS_END

#endif
