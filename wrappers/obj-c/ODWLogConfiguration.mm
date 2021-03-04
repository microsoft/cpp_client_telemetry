//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#import <Foundation/Foundation.h>
#import "ODWLogConfiguration.h"
#import "ODWLogConfiguration_private.h"
#import "ODWLogger_private.h"
#import "LogManager.hpp"

using namespace Microsoft::Applications::Events;

/*!
  Enable analytics.
*/
NSString *const ODWCFG_BOOL_ENABLE_ANALYTICS = @"enableLifecycleSession";

/*!
 Enable multitenant.
*/
NSString *const ODWCFG_BOOL_ENABLE_MULTITENANT = @"multiTenantEnabled";

/*!
 Enable CRC-32 check.
*/
NSString *const ODWCFG_BOOL_ENABLE_CRC32 = @"enableCRC32";

/*!
 Enable HMAC authentication.
*/
NSString *const ODWCFG_BOOL_ENABLE_HMAC = @"enableHMAC";

/*!
 Enable dropping events if DB file size exceeds its limit.
*/
NSString *const ODWCFG_BOOL_ENABLE_DB_DROP_IF_FULL = @"enableDbDropIfFull";

/*!
 Enable database compression.
*/
NSString *const ODWCFG_BOOL_ENABLE_DB_COMPRESS = @"enableDBCompression";

/*!
 Enable WAL journal.
*/
NSString *const ODWCFG_BOOL_ENABLE_WAL_JOURNAL = @"enableWALJournal";

/*!
 Enable network detector.
*/
NSString *const ODWCFG_BOOL_ENABLE_NET_DETECT = @"enableNetworkDetector";

/*!
 The event collection URI.
*/
NSString *const ODWCFG_STR_COLLECTOR_URL = @"eventCollectorUri";

/*!
 The cache file-path.
*/
NSString *const ODWCFG_STR_CACHE_FILE_PATH = @"cacheFilePath";

/*!
 the cache file size limit in bytes.
*/
NSString *const ODWCFG_INT_CACHE_FILE_SIZE = @"cacheFileSizeLimitInBytes";

/*!
 The RAM queue size limit in bytes.
*/
NSString *const ODWCFG_INT_RAM_QUEUE_SIZE = @"cacheMemorySizeLimitInBytes";

/*!
 The size of the RAM queue buffers, in bytes.
*/
NSString *const ODWCFG_INT_RAM_QUEUE_BUFFERS = @"maxDBFlushQueues";

/*!
 The trace level mask.
*/
NSString *const ODWCFG_INT_TRACE_LEVEL_MASK = @"traceLevelMask";

/*!
 The minimum trace level.
*/
NSString *const ODWCFG_INT_TRACE_LEVEL_MIN = @"minimumTraceLevel";

/*!
 Enable trace logs.
*/
NSString *const ODWCFG_BOOL_ENABLE_TRACE = @"enableTrace";

/*!
 The trace filepath.
*/
NSString *const ODWCFG_STR_TRACE_FOLDER_PATH = @"traceFolderPath";

/*!
 The SDK mode.
*/
NSString *const ODWCFG_INT_SDK_MODE = @"sdkmode";

/*!
 Set the provider group directly with a string (which will be converted to a GUID).
*/
NSString *const ODWCFG_STR_PROVIDER_GROUP_ID = @"providerGroupId";

/*!
 The maximum teardown time.
*/
NSString *const ODWCFG_INT_MAX_TEARDOWN_TIME = @"maxTeardownUploadTimeInSec";

/*!
 The maximum number of pending HTTP requests.
*/
NSString *const ODWCFG_INT_MAX_PENDING_REQ = @"maxPendingHTTPRequests";

/*!
 The maximum package drop on full.
*/
NSString *const ODWCFG_INT_MAX_PKG_DROP_ON_FULL = @"maxPkgDropOnFull";

/*!
 The cache file percentage full notification.
*/
NSString *const ODWCFG_INT_STORAGE_FULL_PCT = @"cacheFileFullNotificationPercentage";

/*!
 The minimum time (ms) between storage full notifications.
*/
NSString *const ODWCFG_INT_STORAGE_FULL_CHECK_TIME = @"cacheFullNotificationIntervalTime";

/*!
 The cache memory percentage full notification.
*/
NSString *const ODWCFG_INT_RAMCACHE_FULL_PCT = @"cacheMemoryFullNotificationPercentage";

/*!
 PRAGMA journal mode.
*/
NSString *const ODWCFG_STR_PRAGMA_JOURNAL_MODE = @"PRAGMA_journal_mode";

/*!
 PRAGMA synchronous.
*/
NSString *const ODWCFG_STR_PRAGMA_SYNCHRONOUS = @"PRAGMA_synchronous";

NSString *const ODWCFG_STR_PRIMARY_TOKEN = @"primaryToken";

/*!
 Parameter that allows to apply custom transmit profile on SDK start
*/
NSString *const ODWCFG_STR_START_PROFILE_NAME = @"startProfileName";

/*!
 Parameter that allows to load a set of custom transmit profile on SDK start
*/
NSString *const ODWCFG_STR_TRANSMIT_PROFILES = @"transmitProfiles";

/*!
 IHttpClient override module
*/
NSString *const ODWCFG_MODULE_HTTP_CLIENT = @"httpClient";

/*!
 ITaskDispatcher override module
*/
NSString *const ODWCFG_MODULE_TASK_DISPATCHER = @"taskDispatcher";

/*!
 IDataViewer override module
*/
NSString *const ODWCFG_MODULE_DATA_VIEWER = @"dataViewer";

/*!
 IDecorator override module
*/
NSString *const ODWCFG_MODULE_DECORATOR = @"decorator";

/*!
 IDecorator override module
*/
NSString *const ODWCFG_MODULE_PRIVACY_GUARD = @"privacy_guard";

/*!
 IDecorator override module
*/
NSString *const ODWCFG_MODULE_OFFLINE_STORAGE = @"offlineStorage";

/*!
 LogManagerFactory's name parameter
*/
NSString *const ODWCFG_STR_FACTORY_NAME = @"name";

/*!
 LogManagerFactory (and friends) config map
*/
NSString *const ODWCFG_MAP_FACTORY_CONFIG = @"config";

/*!
 sub-component in CFG_MAP_FACTORY_CONFIG: LogManagerFactory host parameter
*/
NSString *const ODWCFG_STR_FACTORY_HOST = @"host";

/*!
 sub-component in CFG_MAP_FACTORY_CONFIG: capi's scope parameter
*/
NSString *const ODWCFG_STR_CONTEXT_SCOPE = @"scope";

/*!
 MetaStats configuration
*/
NSString *const ODWCFG_MAP_METASTATS_CONFIG = @"stats";

/*!
 MetaStats configuration: time interval
*/
NSString *const ODWCFG_INT_METASTATS_INTERVAL = @"interval";

/*!
 MetaStats configuration: time interval
*/
NSString *const ODWCFG_BOOL_METASTATS_SPLIT = @"split";

/*!
 Compatibility configuration
*/
NSString *const ODWCFG_MAP_COMPAT = @"compat";

/*!
 Compatibility configuration: dot mode
*/
NSString *const ODWCFG_BOOL_COMPAT_DOTS = @"dotType";

/*!
 LogManagerFactory: is this log manager instance in host mode?
*/
NSString *const ODWCFG_BOOL_HOST_MODE = @"hostMode";

/*!
 HTTP configuration map
*/
NSString *const ODWCFG_MAP_HTTP = @"http";

/*!
 HTTP configuration map: MS root check
*/
NSString *const ODWCFG_BOOL_HTTP_MS_ROOT_CHECK = @"msRootCheck";

/*!
 HTTP configuration: compression
*/
NSString *const ODWCFG_BOOL_HTTP_COMPRESSION = @"compress";

/*!
 TPM configuration map
*/
NSString *const ODWCFG_MAP_TPM = @"tpm";

/*!
 TPM configuration: max retry
*/
NSString *const ODWCFG_INT_TPM_MAX_RETRY = @"maxRetryCount";

/*!
 TPM configuration map
*/
NSString *const ODWCFG_STR_TPM_BACKOFF = @"backoffConfig";

/*!
 TPM configuration map
*/
NSString *const ODWCFG_INT_TPM_MAX_BLOB_BYTES = @"maxBlobSize";

/*!
 TPM configuration map
*/
NSString *const ODWCFG_BOOL_TPM_CLOCK_SKEW_ENABLED = @"clockSkewEnabled";

/*!
 When enabled, the session timer is reset after session is completed, allowing for several session events in the duration of the SDK lifecycle
*/
NSString *const ODWCFG_BOOL_SESSION_RESET_ENABLED = @"sessionResetEnabled";

@implementation ODWLogConfiguration
    static bool _enableTrace;
    static bool _enableConsoleLogging;
    static bool _enableSessionReset;
    static bool _surfaceCppExceptions;
    ILogConfiguration* _wrappedConfiguration;

-(instancetype)initWithILogConfiguration:(ILogConfiguration*)config
{
    self = [super init];
    if(self) {
        _wrappedConfiguration = config;
    }
    return self;
}

-(nullable ILogConfiguration*)getWrappedConfiguration
{
    return _wrappedConfiguration;
}

+(nullable ODWLogConfiguration *)getLogConfigurationCopy
{
    auto& config = LogManager::GetLogConfiguration();
    static ILogConfiguration configCopy(config);
    return [[ODWLogConfiguration alloc] initWithILogConfiguration: &configCopy];
}

+(void)setEventCollectorUri:(nonnull NSString *)eventCollectorUri
{
    std::string strCollectorUri = std::string([eventCollectorUri UTF8String]);
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_STR_COLLECTOR_URL] = strCollectorUri;
}

+(nullable NSString *)eventCollectorUri
{
    auto& config = LogManager::GetLogConfiguration();
    std::string strCollectorUri = config[CFG_STR_COLLECTOR_URL];
    if (strCollectorUri.empty())
    {
        return nil;
    }
    return [NSString stringWithCString:strCollectorUri.c_str() encoding:NSUTF8StringEncoding];
}

+(void)setCacheMemorySizeLimitInBytes:(uint64_t)cacheMemorySizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_RAM_QUEUE_SIZE] = cacheMemorySizeLimitInBytes;
}

+(uint64_t)cacheMemorySizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    return config[CFG_INT_RAM_QUEUE_SIZE];
}

+(void)setCacheFileSizeLimitInBytes:(uint64_t)cacheFileSizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_CACHE_FILE_SIZE] = cacheFileSizeLimitInBytes;
}

+(uint64_t)cacheFileSizeLimitInBytes
{
    auto& config = LogManager::GetLogConfiguration();
    return config[CFG_INT_CACHE_FILE_SIZE];
}

+(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_MAX_TEARDOWN_TIME] = maxTeardownUploadTimeInSec;
}

+(void)setEnableTrace:(bool)enableTrace
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_BOOL_ENABLE_TRACE] = enableTrace;
    _enableTrace = enableTrace;
}

+(void)setEnableConsoleLogging:(bool)enableConsoleLogging
{
    _enableConsoleLogging = enableConsoleLogging;
}

+(void)setTraceLevel:(int)traceLevel
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_INT_TRACE_LEVEL_MIN] = traceLevel;
}

+(bool)enableTrace
{
    return _enableTrace;
}

+(bool)enableConsoleLogging
{
    return _enableConsoleLogging;
}

+(void)setSurfaceCppExceptions:(bool)surfaceCppExceptions
{
    _surfaceCppExceptions = surfaceCppExceptions;
}

+(bool)surfaceCppExceptions
{
    return _surfaceCppExceptions;
}

+(void)setEnableSessionReset:(bool)enableSessionReset
{
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_BOOL_SESSION_RESET_ENABLED] = enableSessionReset;
    _enableSessionReset = enableSessionReset;
}

+(bool)enableSessionReset
{
    return _enableSessionReset;
}

+(void)setCacheFilePath:(nonnull NSString *)cacheFilePath
{
    std::string strCacheFilePath = std::string([cacheFilePath UTF8String]);
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_STR_CACHE_FILE_PATH] = strCacheFilePath;
}

+(nullable NSString *)cacheFilePath
{
    auto& config = LogManager::GetLogConfiguration();
    std::string strCacheFilePath = config[CFG_STR_CACHE_FILE_PATH];
    if (strCacheFilePath.empty())
    {
        return nil;
    }
    return [NSString stringWithCString:strCacheFilePath.c_str() encoding:NSUTF8StringEncoding];
}

-(void)set:(nonnull NSString *)key withValue:(nonnull NSString *)value
{
    ILogConfiguration wrappedConfig = *_wrappedConfiguration;
    wrappedConfig[[key UTF8String]] = [value UTF8String];
}

@end
