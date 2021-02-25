//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "objc_begin.h"

/*!
 Enable analytics.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_ANALYTICS;

/*!
 Enable multitenant.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_MULTITENANT;

/*!
 Enable CRC-32 check.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_CRC32;

/*!
 Enable HMAC authentication.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_HMAC;

/*!
 Enable dropping events if DB file size exceeds its limit.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_DB_DROP_IF_FULL;

/*!
 Enable database compression.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_DB_COMPRESS;

/*!
 Enable WAL journal.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_WAL_JOURNAL;

/*!
 Enable network detector.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_NET_DETECT;

/*!
 The event collection URI.
*/
extern NSString * _Nonnull const ODWCFG_STR_COLLECTOR_URL;

/*!
 The cache file-path.
*/
extern NSString * _Nonnull const ODWCFG_STR_CACHE_FILE_PATH;

/*!
 the cache file size limit in bytes.
*/
extern NSString * _Nonnull const ODWCFG_INT_CACHE_FILE_SIZE;

/*!
 The RAM queue size limit in bytes.
*/
extern NSString * _Nonnull const ODWCFG_INT_RAM_QUEUE_SIZE;

/*!
 The size of the RAM queue buffers, in bytes.
*/
extern NSString * _Nonnull const ODWCFG_INT_RAM_QUEUE_BUFFERS;

/*!
 The trace level mask.
*/
extern NSString * _Nonnull const ODWCFG_INT_TRACE_LEVEL_MASK;

/*!
 The minimum trace level.
*/
extern NSString * _Nonnull const ODWCFG_INT_TRACE_LEVEL_MIN;

/*!
 Enable trace logs.
*/
extern NSString * _Nonnull const ODWCFG_BOOL_ENABLE_TRACE;

/*!
 The trace filepath.
*/
extern NSString * _Nonnull const ODWCFG_STR_TRACE_FOLDER_PATH;

/*!
 The SDK mode.
*/
extern NSString * _Nonnull const ODWCFG_INT_SDK_MODE;

/*!
 Set the provider group directly with a string (which will be converted to a GUID).
*/
extern NSString * _Nonnull const ODWCFG_STR_PROVIDER_GROUP_ID;

/*!
 The maximum teardown time.
*/
extern NSString * _Nonnull const ODWCFG_INT_MAX_TEARDOWN_TIME;

/*!
 The maximum number of pending HTTP requests.
*/
extern NSString * _Nonnull const ODWCFG_INT_MAX_PENDING_REQ;

/*!
 The maximum package drop on full.
*/
extern NSString * _Nonnull const ODWCFG_INT_MAX_PKG_DROP_ON_FULL;

/*!
 The cache file percentage full notification.
*/
extern NSString * _Nonnull const ODWCFG_INT_STORAGE_FULL_PCT;

/*!
 The minimum time (ms) between storage full notifications.
*/
extern NSString * _Nonnull const ODWCFG_INT_STORAGE_FULL_CHECK_TIME;

/*!
 The cache memory percentage full notification.
*/
extern NSString * _Nonnull const ODWCFG_INT_RAMCACHE_FULL_PCT;

/*!
 PRAGMA journal mode.
*/
extern NSString * _Nonnull const ODWCFG_STR_PRAGMA_JOURNAL_MODE;

/*!
 PRAGMA synchronous.
*/
extern NSString * _Nonnull const ODWCFG_STR_PRAGMA_SYNCHRONOUS;

extern NSString * _Nonnull const ODWCFG_STR_PRIMARY_TOKEN;

/*!
 Parameter that allows to apply custom transmit profile on SDK start
*/
extern NSString * _Nonnull const ODWCFG_STR_START_PROFILE_NAME;

/*!
 Parameter that allows to load a set of custom transmit profile on SDK start
*/
extern NSString * _Nonnull const ODWCFG_STR_TRANSMIT_PROFILES;

/*!
 IHttpClient override module
*/
extern NSString * _Nonnull const ODWCFG_MODULE_HTTP_CLIENT;

/*!
 ITaskDispatcher override module
*/
extern NSString * _Nonnull const ODWCFG_MODULE_TASK_DISPATCHER;

/*!
 IDataViewer override module
*/
extern NSString * _Nonnull const ODWCFG_MODULE_DATA_VIEWER;

/*!
 IDecorator override module
*/
extern NSString * _Nonnull const ODWCFG_MODULE_DECORATOR;

/*!
 IDecorator override module
*/
extern NSString * _Nonnull const ODWCFG_MODULE_PRIVACY_GUARD;

/*!
 IDecorator override module
*/
extern NSString * _Nonnull const ODWCFG_MODULE_OFFLINE_STORAGE;

/*!
 LogManagerFactory's name parameter
*/
extern NSString * _Nonnull const ODWCFG_STR_FACTORY_NAME;

/*!
 LogManagerFactory (and friends) config map
*/
extern NSString * _Nonnull const ODWCFG_MAP_FACTORY_CONFIG;

/*!
 sub-component in CFG_MAP_FACTORY_CONFIG: LogManagerFactory host parameter
*/
extern NSString * _Nonnull const ODWCFG_STR_FACTORY_HOST;

/*!
 sub-component in CFG_MAP_FACTORY_CONFIG: capi's scope parameter
*/
extern NSString * _Nonnull const ODWCFG_STR_CONTEXT_SCOPE;

/*!
 MetaStats configuration
*/
extern NSString * _Nonnull const ODWCFG_MAP_METASTATS_CONFIG;

/*!
 MetaStats configuration: time interval
*/
extern NSString * _Nonnull const ODWCFG_INT_METASTATS_INTERVAL;

/*!
 MetaStats configuration: time interval
*/
extern NSString * _Nonnull const ODWCFG_BOOL_METASTATS_SPLIT;

/*!
 Compatibility configuration
*/
extern NSString * _Nonnull const ODWCFG_MAP_COMPAT;

/*!
 Compatibility configuration: dot mode
*/
extern NSString * _Nonnull const ODWCFG_BOOL_COMPAT_DOTS;

/*!
 LogManagerFactory: is this log manager instance in host mode?
*/
extern NSString * _Nonnull const ODWCFG_BOOL_HOST_MODE;

/*!
 HTTP configuration map
*/
extern NSString * _Nonnull const ODWCFG_MAP_HTTP;

/*!
 HTTP configuration map: MS root check
*/
extern NSString * _Nonnull const ODWCFG_BOOL_HTTP_MS_ROOT_CHECK;

/*!
 HTTP configuration: compression
*/
extern NSString * _Nonnull const ODWCFG_BOOL_HTTP_COMPRESSION;

/*!
 TPM configuration map
*/
extern NSString * _Nonnull const ODWCFG_MAP_TPM;

/*!
 TPM configuration: max retry
*/
extern NSString * _Nonnull const ODWCFG_INT_TPM_MAX_RETRY;

/*!
 TPM configuration map
*/
extern NSString * _Nonnull const ODWCFG_STR_TPM_BACKOFF;

/*!
 TPM configuration map
*/
extern NSString * _Nonnull const ODWCFG_INT_TPM_MAX_BLOB_BYTES;

/*!
 TPM configuration map
*/
extern NSString * _Nonnull const ODWCFG_BOOL_TPM_CLOCK_SKEW_ENABLED;

/*!
 When enabled, the session timer is reset after session is completed, allowing for several session events in the duration of the SDK lifecycle
*/
extern NSString * _Nonnull const ODWCFG_BOOL_SESSION_RESET_ENABLED;

/*!
 The <b>ODWLogConfiguration</b> static class represents general logging properties.
*/
@interface ODWLogConfiguration : NSObject

/*!
@brief Return a copy of the default configuration
*/
+(nullable ODWLogConfiguration *)getLogConfigurationCopy;

/*!
@brief Sets the URI of the event collector.
@param eventCollectorUri A string for event collector uri.
*/
+(void)setEventCollectorUri:(nonnull NSString *)eventCollectorUri;

/*!
@brief Returns the URI of the event collector.
*/
+(nullable NSString *)eventCollectorUri;

/*!
@brief Sets the RAM queue size limit in bytes.
@param cacheMemorySizeLimitInBytes  A long value for memory size limit in bytes.
*/
+(void)setCacheMemorySizeLimitInBytes:(uint64_t)cacheMemorySizeLimitInBytes;

/*!
@brief return the RAM queue size limit in bytes.
*/
+(uint64_t)cacheMemorySizeLimitInBytes;

/*!
@brief Sets the size limit of the disk file used to cache events on the client side.
@param cacheFileSizeLimitInBytes  A long value for cache file size limit.
*/
+(void)setCacheFileSizeLimitInBytes:(uint64_t)cacheFileSizeLimitInBytes;

/*!
@brief Returns the size limit of the disk file used to cache events on the client side.
*/
+(uint64_t)cacheFileSizeLimitInBytes;

/*!
@brief Sets max teardown upload time in seconds.
@param maxTeardownUploadTimeInSec An integer that time in seconds.
*/
+(void)setMaxTeardownUploadTimeInSec:(int)maxTeardownUploadTimeInSec;

/*!
@brief Sets if trace logging to file is enabled.
@param enableTrace True if tracing is enabled.
*/
+(void)setEnableTrace:(bool)enableTrace;

/*!
@brief Sets if console logging from the iOS wrapper is enabled.
@param enableConsoleLogging True if logging is enabled.
*/
+(void)setEnableConsoleLogging:(bool)enableConsoleLogging;

/*!
@brief Sets the internal SDK debugging trace level.
@param TraceLevel one of the ACTTraceLevel values.
*/
+(void)setTraceLevel:(int)TraceLevel;

/*!
@brief Returns true if tracing is enabled.
*/
+(bool)enableTrace;

/*!
@brief Returns true if console logging is enabled.
*/
+(bool)enableConsoleLogging;

/*!
@brief Sets if inner C++ exceptions should be surfaced to Wrapper consumers.
@param surfaceCppExceptions True if C++ exceptions should be surfaced.
*/
+(void)setSurfaceCppExceptions:(bool)surfaceCppExceptions;

/*!
@brief Returns true if inner C++ exceptions are surfaced to Wrapper consumers.
*/
+(bool)surfaceCppExceptions;

/*!
@brief Sets if session timestamp should be reset after a session ends
@param enableSessionReset True if session should be reset on session end.
*/
+(void)setEnableSessionReset:(bool)enableSessionReset;

/*!
@brief Returns true if session will be reset on session end.
*/
+(bool)enableSessionReset;

/*!
@brief Sets the cache file path.
@param cacheFilePath A string for the cache path.
*/
+(void)setCacheFilePath:(nonnull NSString *)cacheFilePath;

/*!
@brief Returns the cache file path.
*/
+(nullable NSString *)cacheFilePath;

/*!
@brief Sets a config key to a string value for the copied config
@param key A key.
@param value A value.
*/
-(void)set:(nonnull NSString *)key withValue:(nonnull NSString *)value;

@end

#include "objc_end.h"
