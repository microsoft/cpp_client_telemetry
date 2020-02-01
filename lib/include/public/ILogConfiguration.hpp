// Copyright (c) Microsoft. All rights reserved.
#ifndef MAT_ILOGCONFIGURATION_HPP
#define MAT_ILOGCONFIGURATION_HPP

#include "Version.hpp"

#include "Enums.hpp"
#include "ctmacros.hpp"
#include "stdint.h"

#include <memory>
#include <string>

#include "IModule.hpp"
#include "Variant.hpp"

namespace ARIASDK_NS_BEGIN
{
    class IModule;
    class IHttpClient;

    /// Default collector url to send events to
    static constexpr const char* COLLECTOR_URL_PROD = "https://self.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the United States collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_UNITED_STATES = "https://noam.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the German collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_GERMANY = "https://emea.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the Australian collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_AUSTRALIA = "https://apac.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the Japanese collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_JAPAN = "https://apac.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the European collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_EUROPE = "https://emea.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The real-time transmit profile.
    /// </summary>
    static constexpr const char* const TRANSMITPROFILE_REALTIME = "REAL_TIME";

    /// <summary>
    /// The near real-time transmit profile.
    /// </summary>
    static constexpr const char* const TRANSMITPROFILE_NEARREALTIME = "NEAR_REAL_TIME";

    /// <summary>
    /// The best effort transmit profile.
    /// </summary>
    static constexpr const char* const TRANSMITPROFILE_BESTEFFORT = "BEST_EFFORT";

    /// <summary>
    /// Enable analytics.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_ANALYTICS = "enableLifecycleSession";

    /// <summary>
    /// Enable multitenant.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_MULTITENANT = "multiTenantEnabled";

    /// <summary>
    /// Enable CRC-32 check.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_CRC32 = "enableCRC32";

    /// <summary>
    /// Enable HMAC authentication.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_HMAC = "enableHMAC";

    /// <summary>
    /// Enable dropping events if DB file size exceeds its limit.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_DB_DROP_IF_FULL = "enableDbDropIfFull";

    /// <summary>
    /// Enable database compression.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_DB_COMPRESS = "enableDBCompression";

    /// <summary>
    /// Enable WAL journal.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_WAL_JOURNAL = "enableWALJournal";

    /// <summary>
    /// Enable network detector.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_NET_DETECT = "enableNetworkDetector";

    /// <summary>
    /// Parameter that allows to check if the SDK is running on UTC mode
    /// </summary>
    static constexpr const char* const CFG_BOOL_UTC_ENABLED = "enabled";

    /// <summary>
    /// Parameter that allows to check if the SDK is running on UTC mode
    /// </summary>
    static constexpr const char* const CFG_BOOL_UTC_ACTIVE = "active";

    /// <summary>
    /// Parameter that allows to check if the Windows 10 version SDK is using supports large payloads on UTC
    /// </summary>
    static constexpr const char* const CFG_BOOL_UTC_LARGE_PAYLOADS = "largePayloadsEnabled";

    /// <summary>
    /// The event collection URI.
    /// </summary>
    static constexpr const char* const CFG_STR_COLLECTOR_URL = "eventCollectorUri";

    /// <summary>
    /// The cache file-path.
    /// </summary>
    static constexpr const char* const CFG_STR_CACHE_FILE_PATH = "cacheFilePath";

    /// <summary>
    /// the cache file size limit in bytes.
    /// </summary>
    static constexpr const char* const CFG_INT_CACHE_FILE_SIZE = "cacheFileSizeLimitInBytes";

    /// <summary>
    /// The RAM queue size limit in bytes.
    /// </summary>
    static constexpr const char* const CFG_INT_RAM_QUEUE_SIZE = "cacheMemorySizeLimitInBytes";

    /// <summary>
    /// The size of the RAM queue buffers, in bytes.
    /// </summary>
    static constexpr const char* const CFG_INT_RAM_QUEUE_BUFFERS = "maxDBFlushQueues";

    /// <summary>
    /// The trace level mask.
    /// </summary>
    static constexpr const char* const CFG_INT_TRACE_LEVEL_MASK = "traceLevelMask";

    /// <summary>
    /// The minimum trace level.
    /// </summary>
    static constexpr const char* const CFG_INT_TRACE_LEVEL_MIN = "minimumTraceLevel";

    /// <summary>
    /// Enable trace logs.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_TRACE = "enableTrace";

    /// <summary>
    /// The trace filepath.
    /// </summary>
    static constexpr const char* const CFG_STR_TRACE_FOLDER_PATH = "traceFolderPath";

    /// <summary>
    /// The SDK mode.
    /// </summary>
    static constexpr const char* const CFG_INT_SDK_MODE = "sdkmode";

    /// <summary>
    /// UTC lives at the root of all UTC-specific configuration.
    /// </summary>
    static constexpr const char* const CFG_STR_UTC = "utc";

    /// <summary>
    /// Set the provider group directly with a string (which will be converted to a GUID).
    /// </summary>
    static constexpr const char* const CFG_STR_PROVIDER_GROUP_ID = "providerGroupId";

    /// <summary>
    /// The maximum teardown time.
    /// </summary>
    static constexpr const char* const CFG_INT_MAX_TEARDOWN_TIME = "maxTeardownUploadTimeInSec";

    /// <summary>
    /// The maximum number of pending HTTP requests.
    /// </summary>
    static constexpr const char* const CFG_INT_MAX_PENDING_REQ = "maxPendingHTTPRequests";

    /// <summary>
    /// The maximum package drop on full.
    /// </summary>
    static constexpr const char* const CFG_INT_MAX_PKG_DROP_ON_FULL = "maxPkgDropOnFull";

    /// <summary>
    /// The cache file percentage full notification.
    /// </summary>
    static constexpr const char* const CFG_INT_STORAGE_FULL_PCT = "cacheFileFullNotificationPercentage";

    /// <summary>
    /// The cache memory percentage full notification.
    /// </summary>
    static constexpr const char* const CFG_INT_RAMCACHE_FULL_PCT = "cacheMemoryFullNotificationPercentage";

    /// <summary>
    /// PRAGMA journal mode.
    /// </summary>
    static constexpr const char* const CFG_STR_PRAGMA_JOURNAL_MODE = "PRAGMA_journal_mode";

    /// <summary>
    /// PRAGMA synchronous.
    /// </summary>
    static constexpr const char* const CFG_STR_PRAGMA_SYNCHRONOUS = "PRAGMA_synchronous";

    static constexpr const char* const CFG_STR_PRIMARY_TOKEN = "primaryToken";

    /// <summary>
    /// Parameter that allows to apply custom transmit profile on SDK start
    /// </summary>
    static constexpr const char* const CFG_STR_START_PROFILE_NAME = "startProfileName";

    /// <summary>
    /// Parameter that allows to load a set of custom transmit profile on SDK start
    /// </summary>
    static constexpr const char* const CFG_STR_TRANSMIT_PROFILES = "transmitProfiles";

    /// <summary>
    /// IHttpClient override module
    /// </summary>
    static constexpr const char* const CFG_MODULE_HTTP_CLIENT = "httpClient";

    /// <summary>
    /// ITaskDispatcher override module
    /// </summary>
    static constexpr const char* const CFG_MODULE_TASK_DISPATCHER = "taskDispatcher";

    /// <summary>
    /// IDataViewer override module
    /// </summary>
    static constexpr const char* const CFG_MODULE_DATA_VIEWER = "dataViewer";

    /// <summary>
    /// IHttpPinger override module
    /// </summary>
    static constexpr const char* const CFG_MODULE_HTTP_PINGER = "httpPinger";

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif
    /// <summary>
    /// The ILogConfiguration class contains LogManager-specific configuration settings
    /// </summary>
    class MATSDK_LIBABI ILogConfiguration
    {
       public:
        /// <summary>
        /// Construct an empty configuration
        /// </summary>
        ILogConfiguration() = default;

        /// <summary>
        /// Construct a pre-populated configuration
        /// </summary>
        /// <param name="initList">Initializer list of key/value config settings</param>
        ILogConfiguration(const std::initializer_list<VariantMap::value_type>& initList);

        /// <summary>
        /// Add a module to the log configuration
        /// </summary>
        /// <param name="key">Module name</param>
        /// <param name="module">Module instance</param>
        void AddModule(const char* key, const std::shared_ptr<IModule>& module);

        /// <summary>
        /// Get a module by name
        /// </summary>
        /// <param name="key">Module name</param>
        /// <returns>Module instance if set, else null</returns>
        std::shared_ptr<IModule> GetModule(const char* key) const;

        /// <summary>
        /// Access underlying modules mpa
        /// </summary>
        std::map<std::string, std::shared_ptr<IModule>>& GetModules();

        /// <summary>
        /// Check if a config value has been set
        /// </summary>
        /// <param name="key">Config name</param>
        /// <returns>True if config value exists, else false</returns>
        bool HasConfig(const char* key) const;

        /// <summary>
        /// Get a config value by name, creating a new value if one doesn't already exist
        /// </summary>
        /// <param name="key">Config name</param>
        /// <returns>Config value</returns>
        Variant& operator[](const char* key);

        /// <summary>
        /// Access underlying VariantMap
        /// </summary>
        VariantMap& operator*();

       private:
        VariantMap m_configs;
        std::map<std::string, std::shared_ptr<IModule>> m_modules;
    };
#ifdef _MSC_VER
#pragma warning(pop)
#endif

}
ARIASDK_NS_END
#endif
