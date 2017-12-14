#pragma once

#include "../../exp/EXPCommonClient.hpp"

#include "IAFDClient.hpp"
#include "AFDConfigCache.hpp"
#include "json.hpp"

#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>


const std::string Retry_Queue_Name = "AFDRetryQueue";
/// max retry times, 
/// if all endpoints failed more than max_retry_times, invoke callback with error
const static int MAX_RETRY_TIMES = 5;

/// default retry time factor, retry time will be 4, 16, 64 ...
const static int DEFAULT_RETRY_TIME_FACTOR = 8;

namespace Microsoft { namespace Applications { namespace Experimentation { namespace AFD {

    class AFDClient : public IAFDClient, public IExpCommonClient
    {
    public:
        AFDClient();
        virtual ~AFDClient();

        virtual void Initialize(const AFDClientConfiguration& config);

        virtual bool AddListener(IAFDClientCallback* listener);

        virtual bool RemoveListener(IAFDClientCallback* listener);

        // Register a logger to auto-tag events sent by the logger with AFD configuration infos like ETag
        virtual bool RegisterLogger(Microsoft::Applications::Events ::ILogger* pLoger, const std::string& agentName);
             
        virtual bool SetRequestParameters(const std::map<std::string, std::string>& requestParams);
        
        virtual bool SetRequestHeaders(const std::map<std::string, std::string>& headerParams);

        virtual bool Start();

        virtual bool Stop();

        virtual bool Suspend();

        virtual bool Resume();

        virtual std::string GetETag();
        
        virtual std::vector<std::string> GetFlights();
        
        virtual std::vector<std::string> GetFeatures();

        virtual std::map<std::string, std::string> GetConfigs();

        virtual std::string GetSetting(const std::string& agentName, const std::string& settingPath, const std::string& defaultValue);

        virtual bool GetSetting(const std::string& agentName, const std::string& settingPath, const bool defaultValue);

        virtual int GetSetting(const std::string& agentName, const std::string& settingPath, const int defaultValue);

        virtual double GetSetting(const std::string& agentName, const std::string& settingPath, const double defaultValue);

        virtual std::vector<std::string> GetSettings(const std::string& agentName, const std::string& settingPath);

        virtual std::vector<int> GetSettingsAsInts(const std::string& agentName, const std::string& settingPath);

        virtual std::vector<double> GetSettingsAsDbls(const std::string& agentName, const std::string& settingPath);

        std::vector<std::string> GetKeys(const std::string& agentName, const std::string& keysPath);

        virtual void HandleHttpCallback(Message& msg, bool& isActiveConfigUpdatedOnAFD, bool& isActiveConfigUpdatedOnAFDSaveNeeded);
        virtual void FireClientEvent(CommonClientEventType evtType, bool fConfigUpdateFromServer);
        virtual void HandleConfigReload(Message& msg, bool& isActiveConfigSwitched, bool& isActiveConfigSwitchedSaveNeeded);
        virtual void HandleConfigSave(bool isActiveConfigSwitchedSaveNeeded, bool isActiveConfigUpdatedOnEXPSaveNeeded);
        virtual void HandleUpdateClient(bool isActiveConfigSwitched, bool isActiveConfigUpdatedOnEXP, bool isActiveConfigUpdatedOnEXPSaveNeeded);
        virtual bool FetchFromServerIfRequired();
        virtual unsigned int GetExpiryTimeInSec();
        virtual nlohmann::json GetActiveConfigVariant();

    private:
        nlohmann::json _GetActiveConfigVariant();
        void _ValidateAFDClientConfiguration(const AFDClientConfiguration& config);
        void _LogEXPConfigUpdateEvent( EXPConfigUpdateResult result, EXPConfigUpdateSource source);
        void _LogEXPCleintStateChangeEvent(EXPClientStatus status);
        void _UpdateLoggerWithEXPConfig(Microsoft::Applications::Events ::ILogger* pLogger, std::string agentName);
        void _UpdateLoggersWithEXPConfig();
        std::int64_t _GetExpiryTimeInSecFromHeader(Message& msg);
        void LoadActiveConfigs();
       
        
        
        AFDClientConfiguration  m_AFDClientConfiguration;
        std::set<IAFDClientCallback *>      m_listeners;
        AFDConfigCache*         m_configCache;
        AFDConfig*              m_configActive;
        ExpCommon               m_EXPCommon;
        unsigned int            m_minExpireTimeInSecs;
      
#ifdef _USE_TEST_INJECTION_AFDCLIENT_
        _USE_TEST_INJECTION_AFDCLIENT_
#endif
    };

}}}}
