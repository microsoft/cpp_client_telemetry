//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_ILOGCONFIGURATION_HPP
#define MAT_ILOGCONFIGURATION_HPP

#include "ctmacros.hpp"

#include "Enums.hpp"
#include "stdint.h"

#include <memory>
#include <string>

#include "IModule.hpp"
#include "Variant.hpp"

namespace MAT_NS_BEGIN
{
    class IModule;
    class IHttpClient;

    /// Default collector url to send events to
    static constexpr const char* COLLECTOR_URL_PROD = "https://mobile.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the United States collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_UNITED_STATES = "https://us-mobile.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the German collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_GERMANY = "https://eu-mobile.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the Australian collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_AUSTRALIA = "https://au-mobile.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the Japanese collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_JAPAN = "https://jp-mobile.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the European collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_EUROPE = "https://eu-mobile.events.data.microsoft.com/OneCollector/1.0/";

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
    /// SQLite DB will be checkpointed when flushing.
    /// </summary>
    static constexpr const char* const CFG_BOOL_CHECKPOINT_DB_ON_FLUSH = "checkpointDBOnFlush";

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
    /// Sets the provider name when in UTC mode.
    /// </summary>
    static constexpr const char* const CFG_STR_UTC_PROVIDER_NAME = "providerName";

    /// <summary>
    /// Set the provider group directly with a string (which will be converted to a GUID).
    /// </summary>
    static constexpr const char* const CFG_STR_PROVIDER_GROUP_ID = "providerGroupId";

    /// <summary>
    /// Skip registering the iKey with UTC, so that a pre-existing provider can be used.
    /// </summary>
    static constexpr const char* const CFG_STR_SKIP_IKEY_REGISTRATION = "skipIKeyRegistration";

    /// <summary>
    /// The maximum teardown time.
    /// </summary>
    static constexpr const char* const CFG_INT_MAX_TEARDOWN_TIME = "maxTeardownUploadTimeInSec";

    /// <summary>
    /// Disable zombie logger logic.
    /// </summary>
    static constexpr const char* const CFG_BOOL_DISABLE_ZOMBIE_LOGGERS = "disableZombieLoggers";

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
    /// The minimum time (ms) between storage full notifications.
    /// </summary>
    static constexpr const char* const CFG_INT_STORAGE_FULL_CHECK_TIME = "cacheFullNotificationIntervalTime";

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
    /// IDecorator override module
    /// </summary>
    static constexpr const char* const CFG_MODULE_DECORATOR = "decorator";

    /// <summary>
    /// IDecorator override module
    /// </summary>
    static constexpr const char* const CFG_MODULE_PRIVACY_GUARD = "privacy_guard";

    /// <summary>
    /// IDecorator override module
    /// </summary>
    static constexpr const char* const CFG_MODULE_OFFLINE_STORAGE = "offlineStorage";

    /// <summary>
    /// Pointer to the Android app's JavaVM
    /// </summary>
    static constexpr const char* const CFG_PTR_ANDROID_JVM = "android_jvm";

    /// <summary>
    /// JObject of the Android app's main activity
    /// </summary>
    static constexpr const char* const CFG_JOBJECT_ANDROID_ACTIVITY = "android_activity";

    /// <summary>
    /// LogManagerFactory's name parameter
    /// </summary>
    static constexpr const char* const CFG_STR_FACTORY_NAME = "name";

    /// <summary>
    /// LogManagerFactory (and friends) config map
    /// </summary>
    static constexpr const char* const CFG_MAP_FACTORY_CONFIG = "config";

    /// <summary>
    /// sub-component in CFG_MAP_FACTORY_CONFIG: LogManagerFactory host parameter
    /// </summary>
    static constexpr const char* const CFG_STR_FACTORY_HOST = "host";

    /// <summary>
    /// sub-component in CFG_MAP_FACTORY_CONFIG: capi's scope parameter
    /// </summary>
    static constexpr const char* const CFG_STR_CONTEXT_SCOPE = "scope";

    /// <summary>
    /// MetaStats configuration
    /// </summary>
    static constexpr const char* const CFG_MAP_METASTATS_CONFIG = "stats";

    /// <summary>
    /// MetaStats configuration: time interval
    /// </summary>
    static constexpr const char* const CFG_INT_METASTATS_INTERVAL = "interval";

    /// <summary>
    /// MetaStats configuration: time interval
    /// </summary>
    static constexpr const char* const CFG_BOOL_METASTATS_SPLIT = "split";

    /// <summary>
    /// Compatibility configuration
    /// </summary>
    static constexpr const char* const CFG_MAP_COMPAT = "compat";

    /// <summary>
    /// Compatibility configuration: dot mode
    /// </summary>
    static constexpr const char* const CFG_BOOL_COMPAT_DOTS = "dotType";

    /// <summary>
    /// Compatibility configuration: custom type prefix. Default value: "custom"
    /// </summary>
    static constexpr const char* const CFG_STR_COMPAT_PREFIX = "customTypePrefix";

    /// <summary>
    /// LogManagerFactory: is this log manager instance in host mode?
    /// </summary>
    static constexpr const char* const CFG_BOOL_HOST_MODE = "hostMode";

    /// <summary>
    /// HTTP configuration map
    /// </summary>
    static constexpr const char* const CFG_MAP_HTTP = "http";

    /// <summary>
    /// HTTP configuration map: MS root check
    /// </summary>
    static constexpr const char* const CFG_BOOL_HTTP_MS_ROOT_CHECK = "msRootCheck";

    /// <summary>
    /// HTTP configuration: compression
    /// </summary>
    static constexpr const char* const CFG_BOOL_HTTP_COMPRESSION = "compress";

    /// <summary>
    /// TPM configuration map
    /// </summary>
    static constexpr const char* const CFG_MAP_TPM = "tpm";

    /// <summary>
    /// TPM configuration: max retry
    /// </summary>
    static constexpr const char* const CFG_INT_TPM_MAX_RETRY = "maxRetryCount";

    /// <summary>
    /// TPM configuration map
    /// </summary>
    static constexpr const char* const CFG_STR_TPM_BACKOFF = "backoffConfig";

    /// <summary>
    /// TPM configuration map
    /// </summary>
    static constexpr const char* const CFG_INT_TPM_MAX_BLOB_BYTES = "maxBlobSize";

    /// <summary>
    /// TPM configuration map
    /// </summary>
    static constexpr const char* const CFG_BOOL_TPM_CLOCK_SKEW_ENABLED = "clockSkewEnabled";

    /// <summary>
    /// When enabled, the session timer is reset after session is completed, allowing for several session events in the duration of the SDK lifecycle
    /// </summary>
    static constexpr const char* const CFG_BOOL_SESSION_RESET_ENABLED = "sessionResetEnabled";

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
        std::shared_ptr<IModule> GetModule(const char* key);

        /// <summary>
        /// Access underlying modules map
        /// </summary>
        std::map<std::string, std::shared_ptr<IModule>>& GetModules();

        /// <summary>
        /// Check if a config value has been set
        /// </summary>
        /// <param name="key">Config name</param>
        /// <returns>True if config value exists, else false</returns>
        bool HasConfig(const char* key);

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
MAT_NS_END
#endif

