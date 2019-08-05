// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#pragma once
#include "api/IRuntimeConfig.hpp"


namespace ARIASDK_NS_BEGIN {

    static ILogConfiguration defaultRuntimeConfig
    {
        { CFG_INT_TRACE_LEVEL_MIN,       ACTTraceLevel::ACTTraceLevel_Error },
        { CFG_INT_SDK_MODE,              SdkModeTypes::SdkModeTypes_CS },
        { CFG_BOOL_ENABLE_ANALYTICS,     false },
        { CFG_INT_CACHE_FILE_SIZE,       3145728 },
        { CFG_INT_RAM_QUEUE_SIZE,        524288 },
        { CFG_BOOL_ENABLE_MULTITENANT,   true },
        { CFG_INT_MAX_TEARDOWN_TIME,     1 },
        { CFG_INT_MAX_PENDING_REQ,       4 },
        { CFG_INT_RAM_QUEUE_BUFFERS,     3 },
        { CFG_INT_TRACE_LEVEL_MASK,      0 },
        { CFG_BOOL_ENABLE_TRACE,         true },
        { CFG_STR_COLLECTOR_URL,         COLLECTOR_URL_PROD },
        { CFG_INT_STORAGE_FULL_PCT,      75 },
        { CFG_INT_RAMCACHE_FULL_PCT,     75 },
        { CFG_BOOL_ENABLE_NET_DETECT,    true },
        { "stats",
            {
                /* Parameter that allows to split stats events by tenant */
                { "split",               false },
                { "interval",            1800 },
                { "tokenProd",           STATS_TOKEN_PROD },
                { "tokenInt",            STATS_TOKEN_INT }
            }
        },
        {"utc",
            {
#ifdef HAVE_MAT_UTC
                { "providerGroupId",            "780dddc8-18a1-5781-895a-a690464fa89c" },
                {CFG_BOOL_UTC_ENABLED,          true},
                {CFG_BOOL_UTC_ACTIVE,           false},
                {CFG_BOOL_UTC_LARGE_PAYLOADS,   false}
#else
                {CFG_BOOL_UTC_ENABLED,          false}
#endif
            }
        },
        { "http",
            {
#ifdef HAVE_MAT_ZLIB
                { "compress",            true }
#else
                { "compress",            false }
#endif
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
        { "compat",
            {
                { "dotType",            true } // false: v1 backwards-compat: event.SetType("My.Custom.Type") => custom.my_custom_type
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

        ILogConfiguration& config;

    public:

        RuntimeConfig_Default(ILogConfiguration& customConfig) :
            config(customConfig)
        {
            Variant::merge_map(customConfig, defaultRuntimeConfig);
        };

        virtual ~RuntimeConfig_Default()
        {

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

        virtual const char* GetProviderGroupId() override
        {
            return config[CFG_STR_UTC][CFG_STR_PROVIDER_GROUP_ID];
        }

        virtual Variant & operator[](const char* key) override
        {
            return config[key]; // FIXME: [MG] - Error #116: LEAK 32 bytes
        }

    };

} ARIASDK_NS_END
