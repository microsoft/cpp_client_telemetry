//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#define SERVICE_NAME "TelemetryAgent"
#define LOG_TAG "TelemetryClient"

#include <errno.h>

#include "CommonFields.h"
#include "Version.hpp"
#include "ctmacros.hpp"
#include "mat.h"

#include <android/log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/TextOutput.h>

#include <utils/Vector.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include "LoggingMacros.h"

#include "EvtPropConverter.hpp"

using namespace MAT;

#define EFAIL -1
#define STATUS_SUCCESS 0

#include "BnTelemetryAgent.h"
#include "BpTelemetryAgent.h"

using namespace com::microsoft::telemetry;
using namespace android;

// Helper function to get a hold of the "TelemetryAgent" service.
sp<ITelemetryAgent> getTelemetryAgent()
{
    ::android::sp<IServiceManager> sm = defaultServiceManager();
    ASSERT(sm != 0);
    sp<IBinder> binder = sm->getService(String16(SERVICE_NAME));
    // TODO: If the "TelemetryAgent" service is not running, then getService times out and binder == 0.
    ASSERT(binder != 0);

    sp<ITelemetryAgent> agent = interface_cast<ITelemetryAgent>(binder);
    ASSERT(agent != 0);

    INFO("TelemetryAgent client started.");

    return agent;
};

/**
 * Simple stable backwards- / forward- compatible ABI interface
 */
extern "C" evt_status_t evt_api_call_binder(evt_context_t* ctx)
{
    static sp<ITelemetryAgent> agent = getTelemetryAgent();

    evt_status_t result = EFAIL;

    if (ctx != nullptr)
    {
        switch (ctx->call)
        {
        case EVT_OP_LOAD:
        {
            result = ENOTSUP;
            break;
        };

        case EVT_OP_UNLOAD:
        {
            result = ENOTSUP;
            break;
        };

        case EVT_OP_OPEN:
        {
            int64_t handle = 0;
            const char* ctx_config = (const char*)(ctx->data);
            String16 config(ctx_config);
            agent->open(config, &handle);
            ctx->handle = handle;
            result = STATUS_SUCCESS;
            break;
        };

        case EVT_OP_OPEN_WITH_PARAMS:
        {
            int64_t handle = 0;
            const char* ctx_config = (const char*)(ctx->data);
            String16 config(ctx_config);
            agent->open(config, &handle);
            ctx->handle = handle;
            result = STATUS_SUCCESS;
            break;
        };

        case EVT_OP_CLOSE:
        {
            int64_t binder_result = 0;
            int64_t handle = ctx->handle;
            agent->close(handle, &binder_result);
            result = STATUS_SUCCESS;
            break;
        }

        case EVT_OP_CONFIG:
            result = ENOTSUP;
            break;

        case EVT_OP_LOG:
        {
            auto prop = reinterpret_cast<evt_prop*>(ctx->data);
            int64_t binder_result = 0;
            // TODO: add different content types. For now we pass 0
            std::vector<uint8_t> eventData;
            EvtPropConverter::serialize(prop, eventData);
            INFO(LOG_TAG "::writeEvent - snd data. size=%u", eventData.size());
            agent->writeEvent(ctx->handle, 0, eventData, &binder_result);
            result = STATUS_SUCCESS;
            break;
        }

        case EVT_OP_PAUSE:
            result = ENOTSUP;
            break;

        case EVT_OP_RESUME:
            result = ENOTSUP;
            break;

        case EVT_OP_UPLOAD:
            result = ENOTSUP;
            break;

        case EVT_OP_FLUSH:
            result = ENOTSUP;
            break;

        case EVT_OP_VERSION:
            result = ENOTSUP;
            break;
            // Add more OPs here

        default:
            result = ENOTSUP;
            break;
        }
    }
    return result;
};

/************************************************************************************************************************/
/* This code is identical to usual Desktop C API. The only difference is that all API calls get routed over Binder IPC  */
/* to remote Telemetry Agent process. Events get serialized in super-lean MessagePack-alike compact binary encoding.    */
/************************************************************************************************************************/
#define API_KEY "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"

#define JSON_CONFIG(...) (#__VA_ARGS__)

// Default configuration in JSON format. Telemetry by default is sent to "1DSCppSdkTest" tenant.
// Please change "primaryToken" parameter below to send telemetry to your subscription.
// FIXME:
// - REAL COLLECTOR   --- "eventCollectorUri" : "https://self.events.data.microsoft.com/OneCollector/1.0/",
// - Android Emulator --- "eventCollectorUri" : "http://10.0.2.2/OneCollector/1.0/"
const char* defaultConfig = static_cast<const char*> JSON_CONFIG(
    {
        "cacheFileFullNotificationPercentage" : 75,
        "cacheFilePath" : "storage.db",
        "cacheFileSizeLimitInBytes" : 3145728,
        "cacheMemoryFullNotificationPercentage" : 75,
        "cacheMemorySizeLimitInBytes" : 524288,
        "compat" : {
            "dotType" : true
        },
        "enableLifecycleSession" : false,
        "eventCollectorUri" : "https://self.events.data.microsoft.com/OneCollector/1.0/",
        "forcedTenantToken" : null,
        "hostMode" : true,
        "http" : {
            "compress" : true
        },
        "maxDBFlushQueues" : 3,
        "maxPendingHTTPRequests" : 4,
        "maxTeardownUploadTimeInSec" : 1,
        "minimumTraceLevel" : 4,
        "multiTenantEnabled" : true,
        "primaryToken" : "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991",
        "sample" : {
            "rate" : 0
        },
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
        "utc" : {
            "providerGroupId" : "780dddc8-18a1-5781-895a-a690464fa89c"
        }
    });

void test_c_api()
{
    evt_handle_t handle;
    evt_prop event[] = TELEMETRY_EVENT(
        // Common Data Extensions.Envelope - reserved keywords that C API should not use.
        // Alternate solution is to declare a special macro for envelope 'root' namespace props,
        // such as $STR, $INT, etc.
        _STR("name", "Event.Name.Pure.C.Android"),  // Represents the uniquely qualified name for the event
        _STR("ver", "3.0"),                         // Represents the major and minor version of the extension
        _STR("time", "1979-08-12"),                 // Represents the event date time in Coordinated Universal Time(UTC) when the event was generated on the client.This should be in ISO 8601 format
        _INT("popSample", 100),                     // Represents the effective sample rate for this event at the time it was generated by a client
        _STR("iKey", API_KEY),                      // Represents an ID for applications or other logical groupings of events.
        _INT("flags", 0xffffffff),                  // Represents a collection of bits that describe how the event should be processed ...
        _STR("cV", "12345"),                        // Represents the Correlation Vector : A single field for tracking partial order of related telemetry events across component boundaries.
        // Customer Data fields go as part of userdata
        _STR("strKey", "value1"),
        _INT("intKey", 12345),
        //        PII_STR("piiKey", "secret", 1),
        // Part "X" demo - populating CS extension props
        // Common Data Extensions.App
        //        PII_STR("ext.app.userId", "jackfrost@microsoft.com", 1),
        _STR("ext.app.ver", "1.0.0"));

    printf("Testing C API...\n");

    // Note that client API does not have to have a JSON parser at all!
    // Only remote IPC server (Telemetry Agent) should have it.
    handle = evt_open(defaultConfig);

    // Ref. https://docs.microsoft.com/en-us/windows/privacy/basic-level-windows-diagnostic-events-and-fields for description of Common Schema fields
    evt_log(handle, event);

    // Save event to disk
    evt_flush(handle);

    // Try to upload event
    evt_upload(handle);

    // Close telemetry instance. FIXME: not closing should be fine as well.
    evt_close(handle);
};

void startClient()
{
    printf("Starting client...\n");

    // Override default C API implementation with Binder IPC implementation
    evt_api_call = evt_api_call_binder;

    // Run code instrumented with telemetry event
    test_c_api();
};
