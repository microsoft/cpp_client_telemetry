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


namespace PAL = ::Microsoft::Applications::Events::PAL;

namespace Microsoft { namespace Applications { namespace Experimentation { namespace ECS {

    class ECSClient : public IECSClient, public IExpCommonClient
    {
    public:
        ECSClient();
        virtual ~ECSClient();

        virtual void Initialize(const ECSClientConfiguration& config);

        virtual bool AddListener(IECSClientCallback* listener);

        virtual bool RemoveListener(IECSClientCallback* listener);

        // Register a logger to auto-tag events sent by the logger with ECS configuration infos like ETag
        virtual bool RegisterLogger(Microsoft::Applications::Events::ILogger* pLoger, const std::string& agentName);

        virtual bool SetUserId(const std::string& userId);

        virtual bool SetDeviceId(const std::string& deviceId);

        virtual bool SetRequestParameters(const std::map<std::string, std::string>& requestParams);

        virtual bool Start();

        virtual bool Stop();

        virtual bool Suspend();

        virtual bool Resume();

        virtual std::string GetETag();

        virtual std::string GetConfigs();

        virtual bool TryGetSetting(const std::string& agentName, const std::string& settingPath, std::string& value);

        virtual bool TryGetBoolSetting(const std::string& agentName, const std::string& settingPath, bool& value);

        virtual bool TryGetIntSetting(const std::string& agentName, const std::string& settingPath, int& value);

        virtual bool TryGetLongSetting(const std::string& agentName, const std::string& settingPath, long& value);

        virtual bool TryGetDoubleSetting(const std::string& agentName, const std::string& settingPath, double& value);

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

        virtual void SetRetryTimeFactor(int time);


    private:
		nlohmann::json _GetActiveConfigVariant();
        void _ValidateECSClientConfiguration(const ECSClientConfiguration& config);
		void _LogEXPConfigUpdateEvent(EXPConfigUpdateResult result, EXPConfigUpdateSource source);
		void _LogEXPCleintStateChangeEvent(EXPClientStatus status);
		void _UpdateLoggerWithEXPConfig(Microsoft::Applications::Events::ILogger* pLogger, std::string agentName);
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
