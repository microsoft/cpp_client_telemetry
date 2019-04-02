#pragma once
#include "mat/config.h"
#ifdef HAVE_MAT_EXP

#include "modules/exp/EXPCommonClient.hpp"

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

        virtual void Initialize(const AFDClientConfiguration& config) override;

        virtual bool AddListener(IAFDClientCallback* listener) override;

        virtual bool RemoveListener(IAFDClientCallback* listener) override;

        // Register a logger to auto-tag events sent by the logger with AFD configuration infos like ETag

        virtual bool RegisterLogger(MAT::ILogger* pLoger, const std::string& agentName) override;
             
        virtual bool SetRequestParameters(const std::map<std::string, std::string>& requestParams) override;
        
        virtual bool SetRequestHeaders(const std::map<std::string, std::string>& headerParams) override;

        virtual bool Start() override;

        virtual bool Stop() override;

        virtual bool Suspend() override;

        virtual bool Resume(bool fetchConfig = true) override;

        virtual std::string GetETag() override;
        
        virtual std::vector<std::string> GetFlights() override;
        
        virtual std::vector<std::string> GetFeatures() override;

        virtual std::map<std::string, std::string> GetConfigs() override;

        virtual std::string GetSetting(const std::string& agentName, const std::string& settingPath, const std::string& defaultValue) override;

        virtual bool GetSetting(const std::string& agentName, const std::string& settingPath, const bool defaultValue) override;

        virtual int GetSetting(const std::string& agentName, const std::string& settingPath, const int defaultValue) override;

        virtual double GetSetting(const std::string& agentName, const std::string& settingPath, const double defaultValue) override;

        virtual std::vector<std::string> GetSettings(const std::string& agentName, const std::string& settingPath) override;

        virtual std::vector<int> GetSettingsAsInts(const std::string& agentName, const std::string& settingPath) override;

        virtual std::vector<double> GetSettingsAsDbls(const std::string& agentName, const std::string& settingPath) override;

        std::vector<std::string> GetKeys(const std::string& agentName, const std::string& keysPath) override;

        virtual void HandleHttpCallback(Message& msg, bool& isActiveConfigUpdatedOnAFD, bool& isActiveConfigUpdatedOnAFDSaveNeeded) override;

        virtual void FireClientEvent(CommonClientEventType evtType, bool fConfigUpdateFromServer) override;

        virtual void HandleConfigReload(Message& msg, bool& isActiveConfigSwitched, bool& isActiveConfigSwitchedSaveNeeded) override;

        virtual void HandleConfigSave(bool isActiveConfigSwitchedSaveNeeded, bool isActiveConfigUpdatedOnEXPSaveNeeded) override;

        virtual void HandleUpdateClient(bool isActiveConfigSwitched, bool isActiveConfigUpdatedOnEXP, bool isActiveConfigUpdatedOnEXPSaveNeeded) override;

        virtual bool FetchFromServerIfRequired() override;

        virtual unsigned int GetExpiryTimeInSec() override;

        virtual nlohmann::json GetActiveConfigVariant() override;

        virtual std::string GetAFDConfiguration() override;

        virtual void SetRetryTimeFactor(int time) override;

    private:
        nlohmann::json _GetActiveConfigVariant();
        void _ValidateAFDClientConfiguration(const AFDClientConfiguration& config);
        void _LogEXPConfigUpdateEvent( EXPConfigUpdateResult result, EXPConfigUpdateSource source);
        void _LogEXPCleintStateChangeEvent(EXPClientStatus status);
        void _UpdateLoggerWithEXPConfig(MAT::ILogger* pLogger, const std::string& agentName);

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
#endif