#pragma once

#include "../../EXPCommonClient.hpp"
#include "IECSClient.hpp"
#include "ECSConfigCache.hpp"
#include "json.hpp"
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>


const std::string Retry_Queue_Name = "ECSRetryQueue";
/// max retry times, 
/// if all endpoints failed more than max_retry_times, invoke callback with error
const static int MAX_RETRY_TIMES = 5;

/// default retry time factor, retry time will be 4, 16, 64 ...
const static int DEFAULT_RETRY_TIME_FACTOR = 8;
namespace Microsoft { namespace Applications { namespace Experimentation { namespace ECS {

    class ECSClient : public IECSClient, public IExpCommonClient
    {
    public:
        ECSClient();
        virtual ~ECSClient();

        virtual void Initialize(const ECSClientConfiguration& config) override;

        virtual bool AddListener(IECSClientCallback* listener) override;

        virtual bool RemoveListener(IECSClientCallback* listener) override;

        // Register a logger to auto-tag events sent by the logger with ECS configuration infos like ETag
        virtual bool RegisterLogger(MAT::ILogger* pLoger, const std::string& agentName) override;

        virtual bool SetUserId(const std::string& userId) override;

        virtual bool SetDeviceId(const std::string& deviceId) override;

        virtual bool SetRequestParameters(const std::map<std::string, std::string>& requestParams) override;

        virtual bool Start() override;

        virtual bool Stop() override;

        virtual bool Suspend() override;

        virtual bool Resume(bool fetchConfig = false) override;

        virtual std::string GetETag() override;

        virtual std::string GetConfigs() override;

        virtual bool TryGetSetting(const std::string& agentName, const std::string& settingPath, std::string& value) override;

        virtual bool TryGetBoolSetting(const std::string& agentName, const std::string& settingPath, bool& value) override;

        virtual bool TryGetIntSetting(const std::string& agentName, const std::string& settingPath, int& value) override;

        virtual bool TryGetLongSetting(const std::string& agentName, const std::string& settingPath, long& value) override;

        virtual bool TryGetDoubleSetting(const std::string& agentName, const std::string& settingPath, double& value) override;

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

        virtual void SetRetryTimeFactor(int time) override;


    private:
		nlohmann::json _GetActiveConfigVariant();
        void _ValidateECSClientConfiguration(const ECSClientConfiguration& config);
		void _LogEXPConfigUpdateEvent(EXPConfigUpdateResult result, EXPConfigUpdateSource source);
		void _LogEXPCleintStateChangeEvent(EXPClientStatus status);
		void _UpdateLoggerWithEXPConfig(MAT::ILogger* pLogger, const std::string& agentName);
		void _UpdateLoggersWithEXPConfig();
		std::int64_t _GetExpiryTimeInSecFromHeader(Message& msg);

        
        ECSClientConfiguration  m_ecsClientConfiguration;
		std::set<IECSClientCallback *>      m_listeners;
        ECSConfigCache*         m_configCache;
        ECSConfig*              m_configActive;
        std::string             m_inProgressRequestName;
		ExpCommon               m_EXPCommon;
		unsigned int            m_minExpireTimeInSecs;		
     

#ifdef _USE_TEST_INJECTION_ECSCLIENT_
        _USE_TEST_INJECTION_ECSCLIENT_
#endif
    };

}}}}
