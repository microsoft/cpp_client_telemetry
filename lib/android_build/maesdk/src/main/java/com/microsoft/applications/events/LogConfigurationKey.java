//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public enum LogConfigurationKey {
  /** Enable analytics */
  CFG_BOOL_ENABLE_ANALYTICS("enableLifecycleSession", Boolean.class),

  /** Enable multitenant. */
  CFG_BOOL_ENABLE_MULTITENANT("multiTenantEnabled", Boolean.class),

  /** Enable CRC-32 check. */
  CFG_BOOL_ENABLE_CRC32("enableCRC32", Boolean.class),

  /** Enable HMAC authentication. */
  CFG_BOOL_ENABLE_HMAC("enableHMAC", Boolean.class),

  /** Enable dropping events if DB file size exceeds its limit. */
  CFG_BOOL_ENABLE_DB_DROP_IF_FULL("enableDbDropIfFull", Boolean.class),

  /** Enable database compression. */
  CFG_BOOL_ENABLE_DB_COMPRESS("enableDBCompression", Boolean.class),

  /** Enable WAL journal. */
  CFG_BOOL_ENABLE_WAL_JOURNAL("enableWALJournal", Boolean.class),

  /** Enable network detector. */
  CFG_BOOL_ENABLE_NET_DETECT("enableNetworkDetector", Boolean.class),

  CFG_BOOL_TPM_CLOCK_SKEW_ENABLED("clockSkewEnabled", Boolean.class),

  /** Parameter that allows to check if the SDK is running on UTC mode */
  CFG_BOOL_UTC_ENABLED("enabled", Boolean.class),

  /** Parameter that allows to check if the SDK is running on UTC mode */
  CFG_BOOL_UTC_ACTIVE("active", Boolean.class),

  /**
   * Parameter that allows to check if the Windows 10 version SDK is using supports large payloads
   * on UTC
   */
  CFG_BOOL_UTC_LARGE_PAYLOADS("largePayloadsEnabled", Boolean.class),

  /** The event collection URI. */
  CFG_STR_COLLECTOR_URL("eventCollectorUri", String.class),

  /** The cache file-path. */
  CFG_STR_CACHE_FILE_PATH("cacheFilePath", String.class),

  /** the cache file size limit in bytes. */
  CFG_INT_CACHE_FILE_SIZE("cacheFileSizeLimitInBytes", Long.class),

  /** The RAM queue size limit in bytes. */
  CFG_INT_RAM_QUEUE_SIZE("cacheMemorySizeLimitInBytes", Long.class),

  /** The size of the RAM queue buffers, in bytes. */
  CFG_INT_RAM_QUEUE_BUFFERS("maxDBFlushQueues", Long.class),

  /** The trace level mask. */
  CFG_INT_TRACE_LEVEL_MASK("traceLevelMask", Long.class),

  /** The minimum trace level. */
  CFG_INT_TRACE_LEVEL_MIN("minimumTraceLevel", Long.class),

  /** Enable trace logs. */
  CFG_BOOL_ENABLE_TRACE("enableTrace", Boolean.class),

  /** The trace filepath. */
  CFG_STR_TRACE_FOLDER_PATH("traceFolderPath", String.class),

  /** The SDK mode. */
  CFG_INT_SDK_MODE("sdkmode", Long.class),

  /** UTC lives at the root of all UTC-specific configuration. */
  CFG_MAP_UTC("utc", ILogConfiguration.class),
  CFG_STR_UTC("utc", ILogConfiguration.class),

  /** Set the provider group directly with a string (which will be converted to a GUID). */
  CFG_STR_PROVIDER_GROUP_ID("providerGroupId", String.class),

  /** The maximum teardown time. */
  CFG_INT_MAX_TEARDOWN_TIME("maxTeardownUploadTimeInSec", Long.class),

  /** The maximum number of pending HTTP requests. */
  CFG_INT_MAX_PENDING_REQ("maxPendingHTTPRequests", Long.class),

  /** The maximum package drop on full. */
  CFG_INT_MAX_PKG_DROP_ON_FULL("maxPkgDropOnFull", Long.class),

  /** The cache file percentage full notification. */
  CFG_INT_STORAGE_FULL_PCT("cacheFileFullNotificationPercentage", Long.class),

  /** The cache file percentage full notification. */
  CFG_INT_STORAGE_FULL_CHECK_TIME("cacheFullNotificationIntervalTime", Long.class),

  /** The cache memory percentage full notification. */
  CFG_INT_RAMCACHE_FULL_PCT("cacheMemoryFullNotificationPercentage", Long.class),

  /** PRAGMA journal mode. */
  CFG_STR_PRAGMA_JOURNAL_MODE("PRAGMA_journal_mode", String.class),

  /** PRAGMA synchronous. */
  CFG_STR_PRAGMA_SYNCHRONOUS("PRAGMA_synchronous", String.class),

  /** Primary token for this log manager */
  CFG_STR_PRIMARY_TOKEN("primaryToken", String.class),

  /** Parameter that allows to apply custom transmit profile on SDK start */
  CFG_STR_START_PROFILE_NAME("startProfileName", String.class),

  /** Parameter that allows to load a set of custom transmit profile on SDK start */
  CFG_STR_TRANSMIT_PROFILES("transmitProfiles", String.class),

  /** LogManagerFactory manager name */
  CFG_STR_FACTORY_NAME("name", String.class),

  /** LogManagerFactory config sub-configuration map */
  CFG_MAP_FACTORY_CONFIG("config", ILogConfiguration.class),

  /** LogManagerFactory sub-config host name for shared managers */
  CFG_STR_FACTORY_HOST("host", String.class),

  /** LogManagerFactory sub-config context scope */
  CFG_STR_CONTEXT_SCOPE("scope", String.class),

  CFG_MAP_SAMPLE("sample", ILogConfiguration.class),

  CFG_INT_SAMPLE_RATE("rate", Long.class),

  /** Metastats config map */
  CFG_MAP_METASTATS_CONFIG("stats", ILogConfiguration.class),

  /** Metastats: interval in seconds */
  CFG_INT_METASTATS_INTERVAL("interval", Long.class),

  CFG_BOOL_METASTATS_SPLIT("split", Boolean.class),

  CFG_STR_METASTATS_TOKEN_INT("tokenInt", String.class),

  CFG_STR_METASTATS_TOKEN_PROD("tokenProd", String.class),

  CFG_MAP_COMPAT("compat", ILogConfiguration.class),

  CFG_BOOL_COMPAT_DOTS("dotType", Boolean.class),

  /** Compatibility configuration: custom type prefix. */
  CFG_STR_COMPAT_PREFIX("customTypePrefix", String.class),

  CFG_BOOL_HOST_MODE("hostMode", Boolean.class),

  CFG_MAP_HTTP("http", ILogConfiguration.class),

  CFG_BOOL_HTTP_MS_ROOT_CHECK("msRootCheck", Boolean.class),

  CFG_BOOL_HTTP_COMPRESSION("compress", Boolean.class),

  CFG_STR_HTTP_CONTENT_ENCODING("contentEncoding", String.class),

  CFG_MAP_TPM("tpm", ILogConfiguration.class),

  CFG_INT_TPM_MAX_RETRY("maxRetryCount", Long.class),

  CFG_STR_TPM_BACKOFF("backoffConfig", String.class),

  CFG_INT_TPM_MAX_BLOB_BYTES("maxBlobSize", Long.class),

  CFG_BOOL_SESSION_RESET_ENABLED("sessionResetEnabled", Boolean.class);

  private String key;
  private Class valueType;

  LogConfigurationKey(String key, Class valueType) {
    this.key = key;
    this.valueType = valueType;
  }

  public String getKey() {
    return key;
  }

  public Class getValueType() {
    return valueType;
  }
}

