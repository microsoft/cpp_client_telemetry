#pragma once
#include "ctmacros.hpp"
#include "ILogger.hpp"

#include <string>
#include <vector>
#include <map>

namespace Microsoft { namespace Applications { namespace Experimentation { namespace AFD {

    // struct used to confiure the AFDClient
    struct AFDClientConfiguration
    {
        // [required] Hearder X-MSEDGE-CLIENTID  or parameter   &clientid=   Name of the client whose the AFD configurations are to be retrieved
        std::string clientId;

		// [optional] Header X-MSEDGE-IG  or parameter &ig=
		std::string impressionGuid;

		// [optional] Header X-MSEDGE-MARKET  or parameter  &mkt
		std::string market;
			
       // [optional] Header X-MSEDGE-EXISTINGUSER 
		int existingUser;

        // [optional] parameter &corpnet=0 
        int corpnet = 1 ;

        // [optional] parameter &setflight =
        std::string setflight;

        // [required] Version of the client whose the AFD configurations are to be retrieved
        std::string clientVersion;

        // [required] Path and name of the file to be used by AFD client to cache configuration locally
        std::string cacheFilePathName;

        // [optional] default time in mins to expire the configuration cached locally
        unsigned int defaultExpiryTimeInMin = 0;

        // [Required] AFD server URLs, default will be used if not specified
        std::vector<std::string> serverUrls;

		// [optional] Used for debugging flight from server URLs, default will be used if not specified
        bool verbose = false;

        // [optional] enabled AFD telemetry
        bool enableAFDClientTelemetry = false;
    };

    // Callback interface for AFDClient to notify its listener of events occurred
    class IAFDClientCallback
    {
    public:
       
		enum AFDClientEventType
		{
			ET_CONFIG_UPDATE_SUCCEEDED = 0,
			ET_CONFIG_UPDATE_FAILED
		};

        struct AFDClientEventContext
        {
            std::string clientId;
            std::string clientVersion;
			std::string impressionID;
			std::int64_t flightingVersion;
            std::map<std::string, std::string> requestHeaders;
            std::map<std::string, std::string> requestParameters;
			std::vector<std::string>     features;
			std::vector<std::string>     flights;

            unsigned int configExpiryTimeInSec;
            bool configUpdateFromAFD;
        };

        /// <summary>
        /// Callback function for AFDClient to notify its listener of any configuration change
        /// </summary>
        /// <param name="evtType">type of the AFDClient event</param>
        /// <param name="evtContext">context information of the AFDClient event</param>
        virtual void OnAFDClientEvent(AFDClientEventType evtType, AFDClientEventContext evtContext) = 0;
    };

    // IAFDClient interface
    class ARIASDK_LIBABI IAFDClient 
    {
    public:
        /// <summary>
        /// Create a new instance of AFDClient
        /// </summary>
        static IAFDClient* ARIASDK_LIBABI_CDECL CreateInstance();

        /// <summary>
        /// Destroy the specified AFDClient instance
        /// </summary>
        static void ARIASDK_LIBABI_CDECL DestroyInstance(IAFDClient** ppAFDClient);

        /// <summary>
        /// Initialize AFD Client
        /// </summary>
		virtual void Initialize(const AFDClientConfiguration& config) = 0;

        /// <summary>
        /// Adds listren
        /// </summary>
		virtual bool AddListener(IAFDClientCallback* listener) = 0;

        /// <summary>
        /// Removes listner
        /// </summary>
		virtual bool RemoveListener(IAFDClientCallback* listener) = 0;
		
		/// <summary>
		/// Register a logger to auto-tag events sent by the logger with AFD configuration information, like Etag.
		/// </summary>
		/// <param name="pLoger">logger to be registered with the AFD client</param>
		/// <param name="agentName">name of the agent whose experiments configIds will be auto-tagged to event sent by the logger</param>
		/// <returns>true if logger is registered successfully, false otherwise</returns>
		virtual bool RegisterLogger(Microsoft::Applications::Telemetry::ILogger* pLoger, const std::string& agentName) = 0;


		/// <summary>
		/// Specify a list of custom paramters for the request to use to retrieve configurations from AFD server
		/// </summary>
		/// <param name="requestParams">list of paramters for the request</param>
		/// <returns>true if parameters are set successfully, false otherwise</returns>
		virtual bool SetRequestParameters(const std::map<std::string, std::string>& requestParams) = 0;

        /// <summary>
		/// Specify a list of custom paramters for the request to use to retrieve configurations from AFD server
		/// </summary>
		/// <param name="headerParams">list of headers for the request</param>
		/// <returns>true if parameters are set successfully, false otherwise</returns>
		virtual bool SetRequestHeaders(const std::map<std::string, std::string>& headerParams) = 0;


		/// <summary>
		/// Start the AFDClient to retrive configurations from AFD server
		/// </summary>
		/// <returns>true if AFDClient is started successfully, false otherwise</returns>
		virtual bool Start() = 0;

		/// <summary>
		/// Stop the AFDClient to retrive configurations from AFD server
		/// </summary>
		/// <returns>true if AFDClient is stopped successfully, false otherwise</returns>
		virtual bool Stop() = 0;

		/// <summary>
		/// Suspend the AFDClient to retrive configuration updates from AFD server
		/// </summary>
		/// <returns>true if AFDClient is suspend successfully, false otherwise</returns>
		virtual bool Suspend() = 0;

		/// <summary>
		/// Resume the AFDClient to retrive configuration updates from AFD server
		/// </summary>
		/// <returns>true if AFDClient is resumed successfully, false otherwise</returns>
		virtual bool Resume() = 0;

		/// <summary>
		/// Get the Flights of the current active AFD configuration
		/// </summary>
		/// <returns>etag string</returns>
		virtual std::vector<std::string> GetFlights() = 0;

		/// <summary>
		/// get the Features of the current active AFD configuration
		/// </summary>
		/// <returns>ETag string</returns>
		virtual std::vector<std::string> GetFeatures() = 0;

		/// <summary>
		/// Get the ETag of the current active AFD configuration
		/// </summary>
		/// <returns>ETag string</returns>
		virtual std::string GetETag() = 0;

		/// <summary>
		/// Get all the keys under the configuration path of the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration keys</param>
		/// <param name="keysPath">configuration path to retrieve keys</param>
		/// <returns>list of configuration keys</returns>
		virtual std::vector<std::string> GetKeys(const std::string& agentName, const std::string& keysPath) = 0;

		/// <summary>
		/// Get the setting of the configuration path for the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
		/// <param name="settingPath">configuration path to retrieve setting</param>
		/// <param name="defaultValue">default value to return if no configuration setting found</param>
		/// <returns>setting as string, return default if no configuration setting found</returns>
		virtual std::string GetSetting(const std::string& agentName, const std::string& settingPath, const std::string& defaultValue) = 0;

		/// <summary>
		/// Get the setting of the configuration path for the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
		/// <param name="settingPath">configuration path to retrieve setting</param>
		/// <param name="defaultValue">default value to return if no configuration setting found</param>
		/// <returns>setting as bool, return default if no configuration setting found</returns>
		virtual bool GetSetting(const std::string& agentName, const std::string& settingPath, const bool defaultValue) = 0;

		/// <summary>
		/// Get the setting of the configuration path for the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
		/// <param name="settingPath">configuration path to retrieve setting</param>
		/// <param name="defaultValue">default value to return if no configuration setting found</param>
		/// <returns>setting as integer, return default if no configuration setting found</returns>
		virtual int GetSetting(const std::string& agentName, const std::string& settingPath, const int defaultValue) = 0;

		/// <summary>
		/// Get the setting of the configuration path for the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration setting</param>
		/// <param name="settingPath">configuration path to retrieve setting</param>
		/// <param name="defaultValue">default value to return if no configuration setting found</param>
		/// <returns>setting as double, return default if no configuration setting found</returns>
		virtual double GetSetting(const std::string& agentName, const std::string& settingPath, const double defaultValue) = 0;

		/// <summary>
		/// Get the list of settings of the configuration path for the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration settings</param>
		/// <param name="settingPath">configuration path to retrieve settings</param>
		/// <returns>list of settings as string</returns>
		virtual std::vector<std::string> GetSettings(const std::string& agentName, const std::string& settingPath) = 0;

		/// <summary>
		/// Get the list of settings of the configuration path for the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration settings</param>
		/// <param name="settingPath">configuration path to retrieve settings</param>
		/// <returns>list of settings as integers</returns>
		virtual std::vector<int> GetSettingsAsInts(const std::string& agentName, const std::string& settingPath) = 0;

		/// <summary>
		/// Get the list of settings of the configuration path for the specified agent
		/// </summary>
		/// <param name="agentName">name of the agent for which to retrieve configuration settings</param>
		/// <param name="settingPath">configuration path to retrieve settings</param>
		/// <returns>list of settings as doubles</returns>
		virtual std::vector<double> GetSettingsAsDbls(const std::string& agentName, const std::string& settingPath) = 0;
	};
   
}}}} // namespaces

