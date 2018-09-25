#ifndef ARIA_ILOGCONFIGURATION_HPP
#define ARIA_ILOGCONFIGURATION_HPP
// Copyright (c) Microsoft. All rights reserved.

#include "Version.hpp"

#include "Enums.hpp"
#include "ctmacros.hpp"
#include "stdint.h"

#include <string>
#include <Variant.hpp>

namespace ARIASDK_NS_BEGIN
{

    class IHttpClient;

    /// Default collector url to send events to
    static constexpr const char* COLLECTOR_URL_PROD            = "https://self.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the United States collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_UNITED_STATES   = "https://noam.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the German collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_GERMANY         = "https://emea.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the Australian collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_AUSTRALIA       = "https://apac.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the Japanese collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_JAPAN           = "https://apac.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The URI of the European collector.
    /// </summary>
    static constexpr const char* COLLECTOR_URL_EUROPE          = "https://emea.events.data.microsoft.com/OneCollector/1.0/";

    /// <summary>
    /// The real-time transmit profile.
    /// </summary>
    static constexpr const char* const TRANSMITPROFILE_REALTIME     = "REAL_TIME";

    /// <summary>
    /// The near real-time transmit profile.
    /// </summary>
    static constexpr const char* const TRANSMITPROFILE_NEARREALTIME = "NEAR_REAL_TIME";

    /// <summary>
    /// The best effort transmit profile.
    /// </summary>
    static constexpr const char* const TRANSMITPROFILE_BESTEFFORT   = "BEST_EFFORT";

    /// <summary>
    /// Enable analytics.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_ANALYTICS    = "enableLifecycleSession";

    /// <summary>
    /// Enable multitenant.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_MULTITENANT  = "multiTenantEnabled";

    /// <summary>
    /// Enable CRC-32 check.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_CRC32 = "enableCRC32";

    /// <summary>
    /// Enable HMAC authentication.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_HMAC = "enableHMAC";

    /// <summary>
    /// Enable database compression.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_DB_COMPRESS = "enableDBCompression";

    /// <summary>
    /// Enable WAL journal.
    /// </summary>
    static constexpr const char* const CFG_BOOL_ENABLE_WAL_JOURNAL = "enableWALJournal";

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
    /// The SDK mode.
    /// </summary>
    static constexpr const char* const CFG_INT_SDK_MODE = "sdkmode";

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
    static constexpr const char* const CFG_STR_START_PROFILE_NAME  = "startProfileName";

    /// <summary>
    /// Parameter that allows to load a set of custom transmit profile on SDK start
    /// </summary>
    static constexpr const char* const CFG_STR_TRANSMIT_PROFILES   = "transmitProfiles";

    /// <summary>
    /// The ILogConfiguration is the interface for configuring the telemetry logging system.
    /// </summary>
    typedef VariantMap ILogConfiguration;

} ARIASDK_NS_END
#endif 
