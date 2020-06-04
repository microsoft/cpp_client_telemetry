#pragma once

#define JSON_CONFIG(...) (#__VA_ARGS__)

// Direct upload configuration
const char *CONFIG_DIRECT = static_cast<const char *> JSON_CONFIG({
  "cacheFileFullNotificationPercentage" : 75,
  "cacheFilePath" : "storage.db",
  "cacheFileSizeLimitInBytes" : 3145728,
  "cacheMemoryFullNotificationPercentage" : 75,
  "cacheMemorySizeLimitInBytes" : 524288,
  "compat" : {"dotType" : true},
  "enableLifecycleSession" : false,
  "eventCollectorUri" : "https://self.events.data.microsoft.com/OneCollector/1.0/",
  "forcedTenantToken" : null,
  "hostMode" : true,
  "http" : {"compress" : true},
  "maxDBFlushQueues" : 3,
  "maxPendingHTTPRequests" : 4,
  "maxTeardownUploadTimeInSec" : 1,
  "minimumTraceLevel" : 4,
  "multiTenantEnabled" : true,
  "primaryToken" : "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322",
  "sample" : {"rate" : 0},
  "sdkmode" : 0,
  "skipSqliteInitAndShutdown" : null,
  "stats" : {
    "interval" : 1800,
    "split" : false,
    "tokenInt" : "8130ef8ff472405d89d6f420038927ea-0c0d561e-cca5-4c81-90ed-0aa9ad786a03-7166",
    "tokenProd" : "4bb4d6f7cafc4e9292f972dca2dcde42-bd019ee8-e59c-4b0f-a02c-84e72157a3ef-7485"
  },
  "tpm" : {
    "backoffConfig" : "E,3000,300000,2,1",
    "clockSkewEnabled" : true,
    "maxBlobSize" : 2097152,
    "maxRetryCount" : 5
  },
  "traceLevelMask" : 0,
  "utc" : {"providerGroupId" : "780dddc8-18a1-5781-895a-a690464fa89c"}
});

// ETW upload configuration
const char *CONFIG_ETW = static_cast<const char *> JSON_CONFIG({
  "minimumTraceLevel" : 4,
  "multiTenantEnabled" : true,
  "primaryToken" : "6d084bbf6a9644ef83f40a77c9e34580",
  "sdkmode" : 3,
  "stats" : {"interval" : 0},
  "traceLevelMask" : 0
});
