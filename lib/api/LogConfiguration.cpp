//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#include "LogConfiguration.hpp"

#ifdef HAVE_MAT_JSONHPP
#include <json.hpp>
using json = nlohmann::json;
#endif

namespace MAT_NS_BEGIN {

    static ILogConfiguration currentConfig {
        { CFG_INT_TRACE_LEVEL_MIN,          ACTTraceLevel::ACTTraceLevel_Error },
        { CFG_BOOL_ENABLE_TRACE,            true },
        { CFG_INT_SDK_MODE,                 SdkModeTypes::SdkModeTypes_CS },
        { CFG_BOOL_ENABLE_ANALYTICS,        false },
        { CFG_INT_CACHE_FILE_SIZE,          3145728 },
        { CFG_INT_RAM_QUEUE_SIZE,           524288 },
        { CFG_BOOL_ENABLE_MULTITENANT,      true },
        { CFG_BOOL_ENABLE_DB_DROP_IF_FULL,  false },
        { CFG_INT_MAX_TEARDOWN_TIME,        0 },
        { CFG_INT_MAX_PENDING_REQ,          4 },
        { CFG_INT_RAM_QUEUE_BUFFERS,        3 },
        { CFG_INT_TRACE_LEVEL_MASK,         0 },
        { CFG_STR_COLLECTOR_URL,            COLLECTOR_URL_PROD },
        { CFG_INT_STORAGE_FULL_PCT,         75 },
        { CFG_INT_STORAGE_FULL_CHECK_TIME,  5000 },
        { CFG_INT_RAMCACHE_FULL_PCT,        75 },
        { CFG_BOOL_ENABLE_NET_DETECT,       true },
    };

    const ILogConfiguration& GetDefaultConfiguration()
    {
        return currentConfig;
    }

    // 
    // v1 to v3 simple LogConfiguration migration helper. This migration helper
    // does not support SetProperty nor key-value map parameters previously
    // supported by v1. Most of these transient key-value parameters are also
    // no longer applicable to v3. Customers who'd like to populate these extra
    // props as a map should do this directly on ILogConfiguration object.
    ILogConfiguration FromLogConfiguration(MAT_v1::LogConfiguration &src)
    {
        ILogConfiguration result {
            { CFG_INT_TRACE_LEVEL_MIN,      src.minimumTraceLevel },
            { CFG_INT_SDK_MODE,             src.sdkmode },
            { CFG_BOOL_ENABLE_ANALYTICS,    src.enableLifecycleSession },
            { CFG_INT_CACHE_FILE_SIZE,      src.cacheFileSizeLimitInBytes },
            { CFG_INT_RAM_QUEUE_SIZE,       src.cacheMemorySizeLimitInBytes },
            { CFG_BOOL_ENABLE_MULTITENANT,  src.multiTenantEnabled },
            { CFG_INT_MAX_TEARDOWN_TIME,    src.maxTeardownUploadTimeInSec },
            { CFG_INT_MAX_PENDING_REQ,      src.maxPendingHTTPRequests },
            { CFG_INT_RAM_QUEUE_BUFFERS,    src.maxDBFlushQueues },
            { CFG_INT_TRACE_LEVEL_MASK,     src.traceLevelMask },
            { CFG_STR_COLLECTOR_URL,        src.eventCollectorUri.c_str() },

            { CFG_INT_STORAGE_FULL_PCT,     75 }, // v1 had these parameters inside STL map.
            { CFG_INT_RAMCACHE_FULL_PCT,    75 }, // Customers transitioning from v1 configuration to v3
                                                  // and using these two parameters (e.g. OTEL) should use
                                                  // ILogConfiguration class directly. It provides modern
                                                  // C++11 initializer list-based config tree.
            { CFG_INT_STORAGE_FULL_CHECK_TIME,  5000 },
        };
        return result;
    }

    ILogConfiguration FromJSON(const char* configuration)
    {
        ILogConfiguration result;
#ifdef HAVE_MAT_JSONHPP
        auto src = json::parse(configuration);
        std::function<void(json &src, VariantMap &dst)> parse;
        parse = [&parse](json &src, VariantMap &dst)->void {
            for (json::iterator it = src.begin(); it != src.end(); ++it) {
                auto t = it.value().type();
                switch (t)
                {
                case json::value_t::array:
                    // TODO
                    break;
                case json::value_t::boolean:
                    dst[it.key()] = (bool)(it.value());
                    break;
                case json::value_t::discarded:
                    // TODO
                    break;
                case json::value_t::null:
                    dst[it.key()] = Variant();
                    break;
                case json::value_t::number_float:
                    dst[it.key()] = (double)(it.value());
                    break;
                case json::value_t::number_integer:
                    dst[it.key()] = (int64_t)(it.value());
                    break;
                case json::value_t::number_unsigned:
                    dst[it.key()] = (uint64_t)(it.value());
                    break;
                case json::value_t::object:
                {
                    VariantMap sub;
                    parse(it.value(), sub);
                    dst[it.key()] = sub;
                    break;
                }
                case json::value_t::string:
                {       
                    std::string val = it.value();
                    dst[it.key()] = std::move(val);
                    break;
                }
                default:
                    // Ignore unsupported binary type values
                    break;
                }
            }
        };
        parse(src, *result);
#else
        (void)(configuration);
        assert(false /* json.hpp support is not enabled! */);
#endif
        return result;
    }

} MAT_NS_END

