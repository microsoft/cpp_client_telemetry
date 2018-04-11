#pragma once

#include "json.hpp"   // KEEP THIS FIRST! (ILogger.hpp is defining max() and min() macros somewhere, which breaks json.hpp)
#include <IHttpClient.hpp>
#include "ILogger.hpp"
#include "pal/PAL.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <mutex>



namespace Microsoft {
    namespace Applications {
        namespace Experimentation {


            const unsigned int DEFAULT_CONFIG_REFETCH_INTERVAL_IN_SECONDS = 3600;     // 1hr
            const unsigned int DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN = 5 * 60;   // 5mins
            const unsigned int DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX = 24 * 3600;// 24hrs

            const std::string EXPCLIENT_CONFIG_UPDATE_SUCCEEDED = "Succeeded";
            const std::string EXPCLIENT_CONFIG_UPDATE_FAILED = "Failed";
            const std::string EXPCLIENT_CONFIG_UPDATE_TOBERETRIED = "ToBeRetried";

            const std::string EXPCLIENT_CONFIG_SOURCE_SERVER = "Server";
            const std::string EXPCLIENT_CONFIG_SOURCE_LOCAL = "Local";

            const std::string EXPCLIENT_STATE_UNKNOWN = "Unknown";
            const std::string EXPCLIENT_STATE_INITIALIZED = "Initialized";
            const std::string EXPCLIENT_STATE_STARTED = "Started";
            const std::string EXPCLIENT_STATE_SUSPENDED = "Suspended";
            const std::string EXPCLIENT_STATE_STOPPING = "Stopping";
            const std::string EXPCLIENT_STATE_STOPPED = "Stopped";
            const std::string EXPCLIENT_STATE_REQUEST_PARAM_CHANGED = "RequestParameterChanged";


            // parameter keys to use for the EXP server request

            // corresponds to ID Allocation in ECS Management Portal
            // Good for allocation that wants to follow the user account
            const std::string EXPCLIENT_RP_KEY_ID = "id";

            // Good for allocation that does not need to cross-device, pass DeviceId as the value
            const std::string EXPCLIENT_RP_KEY_CLIENT_NAME = "clientName";

            // Good for allocation that does not need to cross-device, pass DeviceId as the value
            const std::string EXPCLIENT_RP_KEY_CLIENTID = "clientId";

            // corresponds to IPRange in ECS Management Portal
            const std::string EXPCLIENT_RP_KEY_IPADDRESS = "ipaddress";

            // corresponds to Country in ECS Management Portal
            const std::string EXPCLIENT_RP_KEY_COUNTRYCODE = "countrycode";


            const std::string  CONFIG_IDS_KEY = "ConfigIDs";
            const std::string  DEFAULT_EVENT_TO_CONFIGIDS_MAPPING_AGENT_NAME = "EventToConfigIdsMapping";



            // enum return codes
            enum EXP_ERROR
            {
                EXP_ERROR_OK = 0,
                EXP_ERROR_INVALID_CONFIG,
                EXP_ERROR_INVALID_STATUS,
                EXP_ERROR_INVALID_PARAMETER,
                EXP_ERROR_INVALID_AUTHTOKEN,
                EXP_ERROR_INVALID_EXP_RESPONSE,
                EXP_ERROR_HTTPSTACK_INIT_FAILED,
                EXP_ERROR_LOAD_OFFLINESTORAGE_FAILED,
                EXP_ERROR_START_THREAD_FAILED,
                EXP_ERROR_OUT_OF_MEMORY,
                EXP_ERROR_UNKNOWN_FAILURE
            };

            enum CommonClientEventType
            {
                ET_CONFIG_UPDATE_SUCCEEDED = 0,
                ET_CONFIG_UPDATE_FAILED
            };

            enum MessageType
            {
                MT_UNKNOWN = 0,
                MT_RELOAD_CONFIG,
                MT_HTTP_CALLBACK
            };

            struct Message
            {
                MessageType         type;
                std::string         requestName;
                int                 httpstackError;
                int                 statusCode;
                MAT::HttpHeaders    headers;
                std::string         body;

                Message(MessageType type)
                    : type(type)
                {}
            };

            enum EXPClientStatus
            {
                EXP_UNKNOWN = 0,
                EXP_INITIALIZED,
                EXP_STARTED,
                EXP_SUSPENDED,
                EXP_STOPPING,
                EXP_STOPPED,
                EXP_REQUESTPARAMETER_CHANGED,
                EXP_COUNT
            };

            enum EXPConfigUpdateResult
            {
                EXP_CUR_SUCCEEDED = 0,
                EXP_CUR_FAILED,
                EXP_CUR_TOBERETRIED,
                EXP_CUR_COUNT
            };

            enum EXPConfigUpdateSource
            {
                EXP_CUS_SERVER = 0,
                EXP_CUS_LOCAL,
                EXP_CUS_COUNT
            };
            
            class IExpCommonClient
            {
            public:
                virtual void HandleHttpCallback(Message& msg, bool& isActiveConfigUpdatedOnEXP, bool& isActiveConfigUpdatedOnEXPSaveNeeded) = 0;
                virtual void FireClientEvent(CommonClientEventType evtType, bool fConfigUpdateFromECS) = 0;
                virtual void HandleConfigReload(Message& msg, bool& isActiveConfigSwitched, bool& isActiveConfigSwitchedSaveNeeded) = 0;
                virtual void HandleConfigSave(bool isActiveConfigSwitchedSaveNeeded, bool isActiveConfigUpdatedOnEXPSaveNeeded) = 0;
                virtual void HandleUpdateClient(bool isActiveConfigSwitched, bool isActiveConfigUpdatedOnEXP, bool isActiveConfigUpdatedOnEXPSaveNeeded) = 0;
                virtual bool FetchFromServerIfRequired() = 0;
                virtual unsigned int GetExpiryTimeInSec() = 0;
                virtual nlohmann::json GetActiveConfigVariant() = 0;
            };


            class ExpRequestContext
            {
            public:
                // Sending
                std::unique_ptr<MAT::IHttpRequest>        httpRequest;
                std::string                          httpRequestId;

                // Receiving
                std::unique_ptr<MAT::IHttpResponse const> httpResponse;
            };

            typedef ExpRequestContext* ExpRequestContextPtr;

            class ExpCommon : public MAT::IHttpResponseCallback
            {
            public:
                ExpCommon(IExpCommonClient* client, std::string retry_Queue_Name);
                ~ExpCommon();

                // Register a logger to auto-tag events sent by the logger with EXP configuration infos like ETag
                virtual bool RegisterLogger(MAT::ILogger* pLoger, const std::string& agentName);

                virtual bool Start(std::vector<int>& retryTimes);

                virtual bool Stop();

                virtual bool Suspend();

                virtual bool Resume();

                void _StopInternal();
                void _FireClientEvent(CommonClientEventType evtType, bool fConfigUpdateFromEXP);
                std::string GetRequestName(const std::string clientName);
                bool SetRequestParameters(const std::map<std::string, std::string>& requestParams, bool setUserId = true);

                void _DispatchMessage(const Message& msg);
                void _HandleMessages(std::queue<Message>& msgs, bool& isActiveConfigSwitched, bool& isActiveConfigSwitchedSaveNeeded,
                    bool& isActiveConfigUpdatedOnEXP, bool& isActiveConfigUpdatedOnEXPSaveNeeded);
                void _HandleHttpCallback(Message& msg, bool& isActiveConfigUpdatedOnEXP, bool& isActiveConfigUpdatedOnEXPSaveNeeded);
                void _HandleConfigRefetch(Message& msg, bool& isActiveConfigSwitched, bool& isActiveConfigSwitchedSaveNeeded);

                // IHttpClientManagerCallback
                virtual void OnHttpResponse(MAT::IHttpResponse* response) override;

                void SendRequestAsync(std::string const& url);
                bool CancelRequestsAsync();

                void handleMessageTask();				

                void _UpdateLoggerWithEXPConfig(MAT::ILogger* pLogger, const std::string& agentName, const std::string etag, const std::string configIds);
                void _UpdateLoggerWithEXPConfig(MAT::ILogger* pLogger, const std::string& agentName, const std::string etag, const std::map<std::string, std::string> eventconfigIds);
                void _LogEXPConfigEvent(MAT::EventProperties& evtProperties);

                void _ScheduleFetch(unsigned int seconds = 0);				

            
                EXPClientStatus                     m_status;
                std::mutex                          m_lock;
                std::mutex                          m_smalllock;
                std::vector<std::string>            m_serverUrls;
                std::map<MAT::ILogger*, std::string>     m_registeredLoggers;
                std::map<std::string, std::string>  m_configActiveRequestParams;
                std::map<std::string, std::string>  m_configActiveHeaders;
                std::string                         m_configActiveUserId;
                std::string                         m_configActiveDeviceId;
                // Use different URL for each HTTP retry if previous request failed
                unsigned int                        m_serverUrlIdx;
                std::string                         m_configActiveRequestName;
                std::queue<Message>                 m_messages;
                bool                                m_forceRefech;				
                int                                 m_retryTimeFactor;

                static const std::string EXPClientStatus2STR[EXP_COUNT];
                static const std::string EXPConfigUpdateResult2STR[EXP_CUR_COUNT];
                static const std::string EXPConfigUpdateSource2STR[EXP_CUS_COUNT];


            private:
                volatile bool                            m_isTimerCancelling;
                PAL::DeferredCallbackHandle              m_messageProcessingTask;
                bool                                     m_messageProcessingTaskScheduled;
                std::vector<int>                         m_retryBackoffTimes;
                unsigned int                             m_retrybackoffTimesIndex;
                unsigned int                             m_minExpireTimeInSEXP;
                IExpCommonClient*                        m_clientPtr;
                std::string                              m_retry_Queue_Name;
                std::unique_ptr<MAT::IHttpClient>             m_ownHttpClient;
                std::unique_ptr<MAT::IHttpResponseCallback>   m_httpCallback;

                MAT::IHttpClient*             m_httpClient;
                std::string              m_httpRequestId;


            protected:
                ARIASDK_LOG_DECL_COMPONENT_CLASS();
            };
        }
    }
}
