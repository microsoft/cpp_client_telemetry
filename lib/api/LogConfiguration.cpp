#include "LogConfiguration.hpp"

#include <json.hpp>

using json = nlohmann::json;

namespace ARIASDK_NS_BEGIN {

    static ILogConfiguration currentConfig {
        { CFG_INT_TRACE_LEVEL_MIN,          ACTTraceLevel::ACTTraceLevel_Error },
        { CFG_INT_SDK_MODE,                 SdkModeTypes::SdkModeTypes_Aria },
        { CFG_BOOL_ENABLE_ANALYTICS,        false },
        { CFG_INT_CACHE_FILE_SIZE,          3145728 },
        { CFG_INT_RAM_QUEUE_SIZE,           524288 },
        { CFG_BOOL_ENABLE_MULTITENANT,      true },
        { CFG_INT_MAX_TEARDOWN_TIME,        0 },
        { CFG_INT_MAX_PENDING_REQ,          4 },
        { CFG_INT_RAM_QUEUE_BUFFERS,        3 },
        { CFG_INT_TRACE_LEVEL_MASK,         0 },
        { CFG_STR_COLLECTOR_URL,            COLLECTOR_URL_PROD },
        { CFG_INT_STORAGE_FULL_PCT,         75 },
        { CFG_INT_RAMCACHE_FULL_PCT,        75 }
    };

    const ILogConfiguration& GetDefaultConfiguration()
    {
        return currentConfig;
    }

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
            { CFG_INT_STORAGE_FULL_PCT,     75 },
            { CFG_INT_RAMCACHE_FULL_PCT,    75 }
        };
        return result;
    }

    ILogConfiguration FromJSON(const char* configuration)
    {
        ILogConfiguration result;
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
                    parse(it.value(), dst[it.key()]);
                    break;
                case json::value_t::string:
                    std::string val = it.value();
                    dst[it.key()] = std::move(val);
                    break;
                }
            }
        };
        parse(src, result);
        return result;
    }


} ARIASDK_NS_END
