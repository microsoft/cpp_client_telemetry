#pragma once
#include "ctmacros.hpp"
#include "ILogger.hpp"

#include <string>
#include <vector>
#include <map>

namespace Microsoft { namespace Applications { namespace Experimentation { namespace ECS {

    // struct used to confiure the ECSClient
    struct ECSClientConfiguration
    {
        // [required] Name of the client whose the ECS configurations are to be retrieved
        std::string clientName;

        // [required] Version of the client whose the ECS configurations are to be retrieved
        std::string clientVersion;

        // [required] Path and name of the file to be used by ECS client to cache configuration locally
        std::string cacheFilePathName;

        // [optional] default time in mins to expire the configuration cached locally
        unsigned int defaultExpiryTimeInMin;

        // [optional] ECS server URLs, default will be used if not specified
        std::vector<std::string> serverUrls;
    };

    // Callback interface for ECSClient to notify its listener of events occurred
    class IECSClientCallback
    {
    public:
        enum ECSClientEventType
        {
            ET_CONFIG_UPDATE_SUCCEEDED = 0,
            ET_CONFIG_UPDATE_FAILED
        };

        struct ECSClientEventContext
        {
            std::string clientName;
            std::string clientVersion;
            std::string userId;
            std::string deviceId;
            std::map<std::string, std::string> requestParameters;

            unsigned int configExpiryTimeInSec;
            bool configUpdateFromECS;
        };

        /// <summary>
        /// Callback function for ECSClient to notify its listener of any configuration change
        /// </summary>
        /// <param name="evtType">type of the ECSClient event</param>
        /// <param name="evtContext">context information of the ECSClient event</param>
        virtual void OnECSClientEvent(ECSClientEventType evtType, ECSClientEventContext evtContext) = 0;
    };

    // IECSClient interface
    class ARIASDK_LIBABI IECSClient
    {
    public:
        /// <summary>
        /// Create a new instance of ECSClient
        /// </summary>
        static IECSClient* ARIASDK_LIBABI_CDECL CreateInstance();

        /// <summary>
        /// Destroy the specified ECSClient instance
        /// </summary>
        static void ARIASDK_LIBABI_CDECL DestroyInstance(IECSClient** ppECSClient);

        // Initialize the ECSClient with the specified configuration

        /// <summary>
        /// Initialize the ECSClient with the specified configuration
        /// </summary>
        /// <param name="config">Configuration of the ECSClient</param>
        virtual void Initialize(const ECSClientConfiguration& config) = 0;

        /// <summary>
        /// Add a listner to the ECSClient to be notified of any configuration changes
        /// </summary>
        /// <param name="listener">listener to be added to the ECSClient</param>
        /// <return>true if listener is added successfully, false otherwise</return>
        virtual bool AddListener(IECSClientCallback* listener) = 0;

        /// <summary>
        /// Remove the listner to stop recieving notification from the ECSClient
        /// </summary>
        /// <param name="listener">listener to be removed from the ECSClient</param>
        /// <return>true if listener is removed successfully, false otherwise</return>
        virtual bool RemoveListener(IECSClientCallback* listener) = 0;

        /// <summary>
        /// Register a logger to auto-tag events sent by the logger with ECS configuration information, like Etag.
        /// </summary>
        /// <param name="pLoger">logger to be registered with the ECS client</param>
        /// <param name="agentName">name of the agent whose experiments configIds will be auto-tagged to event sent by the logger</param>
        /// <return>true if logger is registered successfully, false otherwise</return>
        virtual bool RegisterLogger(Microsoft::Applications::Telemetry::ILogger* pLoger, const std::string& agentName) = 0;

        /// <summary>
        /// Specify a user Id to be used as the request parameter for retrieving configurations from ECS server
        /// Client can optionally pass this in the request so that configuration can be allocated on a per-user bases.
        /// The allocation persists for the user Id therefore good for allocation to follow the user account
        /// </summary>
        /// <param name="userId">login user Id to pass in the request</param>
        /// <return>true if it is set successfully, false otherwise</return>
        virtual bool SetUserId(const std::string& userId) = 0;

        /// <summary>
        /// Specify a device Id to be used as the request parameter for retrieving configurations from ECS server
        /// Client can optionally pass this in the request so that configuration can be allocated on a per-device bases.
        /// The allocation persists for the device therefore good for allocation that does not need to cross-device 
        /// and does not depend on the user's login state
        /// </summary>
        /// <param name="deviceId">Device Id to pass in the request</param>
        /// <return>true if it is set successfully, false otherwise</return>
        virtual bool SetDeviceId(const std::string& deviceId) = 0;

        /// <summary>
        /// Specify a list of custom paramters for the request to use to retrieve configurations from ECS server
        /// </summary>
        /// <param name="requestParams">list of paramters for the request</param>
        /// <return>true if parameters are set successfully, false otherwise</return>
        virtual bool SetRequestParameters(const std::map<std::string, std::string>& requestParams) = 0;

        /// <summary>
        /// Start the ECSClient to retrive configurations from ECS server
        /// </summary>
        /// <return>true if ECSClient is started successfully, false otherwise</return>
        virtual bool Start() = 0;

        /// <summary>
        /// Stop the ECSClient to retrive configurations from ECS server
        /// </summary>
        /// <return>true if ECSClient is stopped successfully, false otherwise</return>
        virtual bool Stop() = 0;

        /// <summary>
        /// Suspend the ECSClient to retrive configuration updates from ECS server
        /// </summary>
        /// <return>true if ECSClient is suspend successfully, false otherwise</return>
        virtual bool Suspend() = 0;

        /// <summary>
        /// Resume the ECSClient to retrive configuration updates from ECS server
        /// </summary>
        /// <return>true if ECSClient is resumed successfully, false otherwise</return>
        virtual bool Resume() = 0;

        /// <summary>
        /// Get the ETag of the current active ECS configuration
        /// </summary>
        /// <return>ETag string</return>
        virtual std::string GetETag() = 0;

        /// <summary>
        /// Get all the keys under the configuration path of the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration keys</param>
        /// <param name="keysPath">configuration path to retrieve keys</param>
        /// <return>list of configuration keys</return>
        virtual std::vector<std::string> GetKeys(const std::string& agentName, const std::string& keysPath) = 0;

        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as string, return default if no configuration setting found</return>
        virtual std::string GetSetting(const std::string& agentName, const std::string& settingPath, const std::string& defaultValue) = 0;

        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as bool, return default if no configuration setting found</return>
        virtual bool GetSetting(const std::string& agentName, const std::string& settingPath, const bool defaultValue) = 0;

        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as integer, return default if no configuration setting found</return>
        virtual int GetSetting(const std::string& agentName, const std::string& settingPath, const int defaultValue) = 0;

        /// <summary>
        /// Get the setting of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
        /// <param name="settingPath">configuration path to retrieve setting</param>
        /// <param name="defaultValue">default value to return if no configuration setting found</param>
        /// <return>setting as double, return default if no configuration setting found</return>
        virtual double GetSetting(const std::string& agentName, const std::string& settingPath, const double defaultValue) = 0;

        /// <summary>
        /// Get the list of settings of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration settings</param>
        /// <param name="settingPath">configuration path to retrieve settings</param>
        /// <return>list of settings as string</return>
        virtual std::vector<std::string> GetSettings(const std::string& agentName, const std::string& settingPath) = 0;

        /// <summary>
        /// Get the list of settings of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration settings</param>
        /// <param name="settingPath">configuration path to retrieve settings</param>
        /// <return>list of settings as integers</return>
        virtual std::vector<int> GetSettingsAsInts(const std::string& agentName, const std::string& settingPath) = 0;

        /// <summary>
        /// Get the list of settings of the configuration path for the specified agent
        /// </summary>
        /// <param name="agentName">name of the agent for which to retrieve configuration settings</param>
        /// <param name="settingPath">configuration path to retrieve settings</param>
        /// <return>list of settings as doubles</return>
        virtual std::vector<double> GetSettingsAsDbls(const std::string& agentName, const std::string& settingPath) = 0;
    };

}}}} // namespaces

