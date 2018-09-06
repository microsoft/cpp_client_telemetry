// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "api/IRuntimeConfig.hpp"

#if defined(__linux__) || defined(__gnu_linux__)
#define STATS_TOKEN_PROD "37f99178d02b46a4815783b92b8a5c91-4dc3ce45-9251-4684-9874-40f475c74a32-6908"
#else
#define STATS_TOKEN_PROD "ead4d35d9f17486581d6c09afbe41263-01b1a12d-a157-460b-8efd-af9a10b09682-7259"
#endif
#define STATS_TOKEN_INT  "8130ef8ff472405d89d6f420038927ea-0c0d561e-cca5-4c81-90ed-0aa9ad786a03-7166"

namespace ARIASDK_NS_BEGIN {

    static ILogConfiguration defaultRuntimeConfig
    {
        { CFG_INT_TRACE_LEVEL_MIN,       ACTTraceLevel::ACTTraceLevel_Error },
        { CFG_INT_SDK_MODE,              SdkModeTypes::SdkModeTypes_Aria },
        { CFG_BOOL_ENABLE_ANALYTICS,     false },
        { CFG_INT_CACHE_FILE_SIZE,       3145728 },
        { CFG_INT_RAM_QUEUE_SIZE,        524288 },
        { CFG_BOOL_ENABLE_MULTITENANT,   true },
        { CFG_INT_MAX_TEARDOWN_TIME,     1 },
        { CFG_INT_MAX_PENDING_REQ,       4 },
        { CFG_INT_RAM_QUEUE_BUFFERS,     3 },
        { CFG_INT_TRACE_LEVEL_MASK,      0 },
        { CFG_STR_COLLECTOR_URL,         COLLECTOR_URL_PROD },
        { CFG_INT_STORAGE_FULL_PCT,      75 },
        { CFG_INT_RAMCACHE_FULL_PCT,     75 },
        { "stats",
            {
                { "interval",            60 },
                { "tokenProd",           STATS_TOKEN_PROD },
                { "tokenInt",            STATS_TOKEN_INT }
            }
        },
        { "http",
            {
                { "compress",            true} // false}
            }
        },
        { "tpm",
            {
                { "maxBlobSize",        2097152 },
                { "maxRetryCount",      5},
                { "clockSkewEnabled",   true},
                { "backoffConfig",      "E,3000,300000,2,1" },
            }
        },
        {
            "sample",
            {
                { "rate",               0 }
            }
        }
    };

    // TODO: [MG] - add ability to re-Configure with new custom config on-demand
    
/// <summary>
/// This class overlays a custom configuration provided by the customer
/// on top of default configuration above (defaultRuntimeConfig)
/// </summary>
/// <seealso cref="IRuntimeConfig" />
    class RuntimeConfig_Default : public IRuntimeConfig {

    protected:

        ILogConfiguration config;

    public:

        RuntimeConfig_Default(ILogConfiguration& customConfig) :
            RuntimeConfig_Default()
        {
            for (const auto &kv : customConfig)
            {
                config[kv.first] = kv.second;
            }
        };

        RuntimeConfig_Default()
        {
            config.insert(defaultRuntimeConfig.begin(), defaultRuntimeConfig.end());
        }

        virtual std::string GetCollectorUrl() override
        {
            const char * url = config[CFG_STR_COLLECTOR_URL];
            return std::string(url);
        }

        virtual void DecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName) override
        {
            UNREFERENCED_PARAMETER(extension);
            UNREFERENCED_PARAMETER(experimentationProject);
            UNREFERENCED_PARAMETER(eventName);
        };

        virtual EventLatency GetEventLatency(std::string const& tenantId = std::string(), std::string const& eventName = std::string()) override
        {
            UNREFERENCED_PARAMETER(tenantId);
            UNREFERENCED_PARAMETER(eventName);
            return EventLatency_Normal;
        };

        virtual std::string GetMetaStatsTenantToken() override
        {
            const char* token = config["stats"]["tokenProd"];
            return std::string(token);
        };

        virtual unsigned GetMetaStatsSendIntervalSec() override
        {
            return config["stats"]["interval"];
        }

        virtual unsigned GetOfflineStorageMaximumSizeBytes() override
        {
            return config[CFG_INT_CACHE_FILE_SIZE];
        };

        virtual unsigned GetOfflineStorageResizeThresholdPct() override
        {
            // FIXME: [MG] - add parameter for that
            return 99;
        }

        virtual unsigned GetMaximumRetryCount() override
        {
            return config["tpm"]["maxRetryCount"];
        }

        virtual std::string GetUploadRetryBackoffConfig() override
        {
            return config["tpm"]["backoffConfig"];
        }

        virtual bool IsHttpRequestCompressionEnabled() override
        {
            return config["http"]["compress"];
        }

        virtual unsigned GetMinimumUploadBandwidthBps() override
        {
            // FIXME: [MG] - add parameter for that
            return 0;
        }

        virtual unsigned GetMaximumUploadSizeBytes() override
        {
            return config["tpm"]["maxBlobSize"];
        }

        virtual void SetEventLatency(std::string const& tenantId, std::string const& eventName, EventLatency latency) override
        {
            // TODO: [MG] - currently we don't allow to override the event latency via ECS or runtime config tree
            UNREFERENCED_PARAMETER(tenantId);
            UNREFERENCED_PARAMETER(eventName);
            UNREFERENCED_PARAMETER(latency);
        }

        virtual bool IsClockSkewEnabled() override
        {
            return config["tpm"]["clockSkewEnabled"];
        }

        uint32_t GetTeardownTime() override
        {
            return config[CFG_INT_MAX_TEARDOWN_TIME];
        }

        virtual Variant & operator[](const char* key)
        {
            return config[key]; // FIXME: [MG] - Error #116: LEAK 32 bytes
        }

    };

} ARIASDK_NS_END
