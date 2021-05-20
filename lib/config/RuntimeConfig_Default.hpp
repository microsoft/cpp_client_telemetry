//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#pragma once
#include "api/IRuntimeConfig.hpp"
#include "CommonFields.h"

namespace MAT_NS_BEGIN
{
    static ILogConfiguration defaultRuntimeConfig{
        {CFG_INT_TRACE_LEVEL_MIN, ACTTraceLevel::ACTTraceLevel_Error},
        {CFG_INT_SDK_MODE, SdkModeTypes::SdkModeTypes_CS},
        {CFG_BOOL_ENABLE_ANALYTICS, false},
        {CFG_INT_CACHE_FILE_SIZE, 3145728},
        {CFG_INT_RAM_QUEUE_SIZE, 524288},
        {CFG_BOOL_ENABLE_MULTITENANT, true},
        {CFG_BOOL_ENABLE_DB_DROP_IF_FULL, false},
        {CFG_INT_MAX_TEARDOWN_TIME, 1},
        {CFG_INT_MAX_PENDING_REQ, 4},
        {CFG_INT_RAM_QUEUE_BUFFERS, 3},
        {CFG_INT_TRACE_LEVEL_MASK, 0},
        {CFG_BOOL_ENABLE_TRACE, true},
        {CFG_STR_COLLECTOR_URL, COLLECTOR_URL_PROD},
        {CFG_INT_STORAGE_FULL_PCT, 75},
        {CFG_INT_STORAGE_FULL_CHECK_TIME, 5000},
        {CFG_INT_RAMCACHE_FULL_PCT, 75},
        {CFG_BOOL_ENABLE_NET_DETECT, true},
        {CFG_BOOL_SESSION_RESET_ENABLED, false},
        {CFG_MAP_METASTATS_CONFIG,
         {/* Parameter that allows to split stats events by tenant */
          {"split", false},
          {"interval", 1800},
          {"tokenProd", STATS_TOKEN_PROD},
          {"tokenInt", STATS_TOKEN_INT}}},
        {"utc",
         {
#ifdef HAVE_MAT_UTC
             {"providerGroupId", "780dddc8-18a1-5781-895a-a690464fa89c"},
             {CFG_BOOL_UTC_ENABLED, true},
             {CFG_BOOL_UTC_ACTIVE, false},
             {CFG_BOOL_UTC_LARGE_PAYLOADS, false}
#else
             {CFG_BOOL_UTC_ENABLED, false}
#endif
         }},
        {CFG_MAP_HTTP,
         {
#ifdef HAVE_MAT_ZLIB
             {CFG_BOOL_HTTP_COMPRESSION, true}
#else
             {CFG_BOOL_HTTP_COMPRESSION, false}
#endif
             ,
             {"contentEncoding", "deflate"},
             /* Optional parameter to require Microsoft Root CA */
             {CFG_BOOL_HTTP_MS_ROOT_CHECK, false}}},
        {CFG_MAP_TPM,
         {
             {CFG_INT_TPM_MAX_BLOB_BYTES, 2097152},
             {CFG_INT_TPM_MAX_RETRY, 5},
             {CFG_BOOL_TPM_CLOCK_SKEW_ENABLED, true},
             {CFG_STR_TPM_BACKOFF, "E,3000,300000,2,1"},
         }},
        {CFG_MAP_COMPAT,
         {
             {CFG_BOOL_COMPAT_DOTS, true}, // false: v1 backwards-compat: event.SetType("My.Custom.Type") => custom.my_custom_type
             {CFG_STR_COMPAT_PREFIX, EVENTRECORD_TYPE_CUSTOM_EVENT} // custom type prefix for Interchange / Geneva / Cosmos flow
         }},
        {"sample",
         {{"rate", 0}}}};

    /// <summary>
    /// This class overlays a custom configuration provided by the customer
    /// on top of default configuration above (defaultRuntimeConfig)
    /// </summary>
    /// <seealso cref="IRuntimeConfig" />
    class RuntimeConfig_Default : public IRuntimeConfig
    {
       protected:
        ILogConfiguration& config;

       public:
        RuntimeConfig_Default(ILogConfiguration& customConfig) :
            config(customConfig)
        {
            Variant::merge_map(*customConfig, *defaultRuntimeConfig);
        };

        virtual ~RuntimeConfig_Default()
        {
        }

        virtual std::string GetCollectorUrl() override
        {
            const char* url = config[CFG_STR_COLLECTOR_URL];
            if (url == nullptr)
            {
                return std::string(COLLECTOR_URL_PROD);
            }
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
            const char *defaultToken = STATS_TOKEN_PROD;
            if (config.HasConfig(CFG_MAP_METASTATS_CONFIG))
            {
                const char* token = config[CFG_MAP_METASTATS_CONFIG]["tokenProd"];
                if (token != nullptr)
                    return std::string(token);
            }
            return std::string(defaultToken);
        };

        virtual unsigned GetMetaStatsSendIntervalSec() override
        {
            return config[CFG_MAP_METASTATS_CONFIG]["interval"];
        }

        virtual unsigned GetOfflineStorageMaximumSizeBytes() override
        {
            return config[CFG_INT_CACHE_FILE_SIZE];
        };

        virtual unsigned GetOfflineStorageResizeThresholdPct() override
        {
            return 99;
        }

        virtual unsigned GetMaximumRetryCount() override
        {
            return config[CFG_MAP_TPM][CFG_INT_TPM_MAX_RETRY];
        }

        virtual std::string GetUploadRetryBackoffConfig() override
        {
            return config[CFG_MAP_TPM][CFG_STR_TPM_BACKOFF];
        }

        virtual bool IsHttpRequestCompressionEnabled() override
        {
            return config[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION];
        }

        virtual const std::string& GetHttpRequestContentEncoding() const override
        {
            return config[CFG_MAP_HTTP]["contentEncoding"];
        }

        virtual unsigned GetMinimumUploadBandwidthBps() override
        {
            return 0;
        }

        virtual unsigned GetMaximumUploadSizeBytes() override
        {
            return config[CFG_MAP_TPM][CFG_INT_TPM_MAX_BLOB_BYTES];
        }

        virtual void SetEventLatency(std::string const& tenantId, std::string const& eventName, EventLatency latency) override
        {
            UNREFERENCED_PARAMETER(tenantId);
            UNREFERENCED_PARAMETER(eventName);
            UNREFERENCED_PARAMETER(latency);
        }

        virtual bool IsClockSkewEnabled() override
        {
            return config[CFG_MAP_TPM][CFG_BOOL_TPM_CLOCK_SKEW_ENABLED];
        }

        uint32_t GetTeardownTime() override
        {
            return config[CFG_INT_MAX_TEARDOWN_TIME];
        }

        virtual const char* GetProviderGroupId() override
        {
            return config[CFG_STR_UTC][CFG_STR_PROVIDER_GROUP_ID];
        }

        virtual Variant& operator[](const char* key) override
        {
            return config[key];
        }

        virtual bool HasConfig(const char* key) override
        {
            return config.HasConfig(key);
        }
    };

}
MAT_NS_END

