//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "mat/config.h"

#ifdef _MSC_VER
#pragma warning(disable : 4389)
#endif

#include "CsProtocol_types.hpp"
#include "common/Common.hpp"

#include <LogManager.hpp>
#include <atomic>
#include <cassert>

#include "PayloadDecoder.hpp"

#include "IDecorator.hpp"
#include "mat.h"

#ifdef HAVE_MAT_JSONHPP
#include "json.hpp"
#endif

#include <list>

using namespace MAT;

// Allows to dump test events in their perfect Common Schema shape.
// #define ALLOW_RECORD_DECODER

// 1DSCppSdkTest sandbox key
#define TEST_TOKEN  "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"
#define DUMMY_TOKEN "ffffffffffffffffffffffffffffffff-ffffffff-ffff-ffff-ffff-ffffffffffff-ffff"

#define EVENT_NAME_HOST "Event.Name.Host"
#define EVENT_NAME_GUEST "Event.Name.Guest"

#define JSON_CONFIG(...) #__VA_ARGS__

class TestDebugEventListener : public DebugEventListener
{
   public:
    std::atomic<bool> netChanged;
    std::atomic<unsigned> eps;
    std::atomic<unsigned> numLogged0;
    std::atomic<unsigned> numLogged;
    std::atomic<unsigned> numSent;
    std::atomic<unsigned> numDropped;
    std::atomic<unsigned> numReject;
    std::atomic<unsigned> numHttpError;
    std::atomic<unsigned> numHttpOK;
    std::atomic<unsigned> numCached;
    std::atomic<unsigned> numFiltered;
    std::atomic<unsigned> logLatMin;
    std::atomic<unsigned> logLatMax;
    std::atomic<unsigned> storageFullPct;
    std::atomic<bool> storageFailed;

    std::function<void(::CsProtocol::Record&)> OnLogX;

    TestDebugEventListener() :
        netChanged(false),
        eps(0),
        numLogged0(0),
        numLogged(0),
        numSent(0),
        numDropped(0),
        numReject(0),
        numHttpError(0),
        numHttpOK(0),
        numCached(0),
        numFiltered(0),
        logLatMin(100),
        logLatMax(0),
        storageFullPct(0),
        storageFailed(false)
    {
        resetOnLogX();
    }

    void reset()
    {
        netChanged = false;
        eps = 0;
        numLogged0 = 0;
        numLogged = 0;
        numSent = 0;
        numDropped = 0;
        numReject = 0;
        numHttpError = 0;
        numHttpOK = 0;
        numCached = 0;
        numFiltered = 0;
        logLatMin = 100;
        logLatMax = 0;
        storageFullPct = 0;
        storageFailed = false;
        resetOnLogX();
    }

    virtual void OnLogXDefault(::CsProtocol::Record&){

    };

    void resetOnLogX()
    {
        OnLogX = [this](::CsProtocol::Record& record)
        {
            OnLogXDefault(record);
        };
    }

    virtual void OnDebugEvent(DebugEvent& evt)
    {
        switch (evt.type)
        {
        case EVT_LOG_EVENT:
        case EVT_LOG_LIFECYCLE:
        case EVT_LOG_FAILURE:
        case EVT_LOG_PAGEVIEW:
        case EVT_LOG_PAGEACTION:
        case EVT_LOG_SAMPLEMETR:
        case EVT_LOG_AGGRMETR:
        case EVT_LOG_TRACE:
        case EVT_LOG_USERSTATE:
        case EVT_LOG_SESSION:
        {
            /* Test-only code */
            ::CsProtocol::Record& record = *static_cast<::CsProtocol::Record*>(evt.data);
            numLogged++;
            OnLogX(record);
        }
        break;

        case EVT_REJECTED:
            numReject++;
            break;

        case EVT_ADDED:
            break;

            /* Event counts below would never overflow the size of unsigned int */
        case EVT_CACHED:
            numCached += (unsigned int)evt.param1;
            break;

        case EVT_DROPPED:
            numDropped += (unsigned int)evt.param1;
            break;

        case EVT_SENT:
            numSent += (unsigned int)evt.param1;
            break;

        case EVT_STORAGE_FULL:
            storageFullPct = (unsigned int)evt.param1;
            break;

        case EVT_STORAGE_FAILED:
            storageFailed = true;
            break;

        case EVT_CONN_FAILURE:
        case EVT_HTTP_FAILURE:
        case EVT_COMPRESS_FAILED:
        case EVT_UNKNOWN_HOST:
        case EVT_SEND_FAILED:

        case EVT_HTTP_ERROR:
            numHttpError++;
            break;

        case EVT_HTTP_OK:
            numHttpOK++;
            break;
        case EVT_FILTERED:
            numFiltered++;
            break;

        case EVT_SEND_RETRY:
        case EVT_SEND_RETRY_DROPPED:
            break;

        case EVT_NET_CHANGED:
            netChanged = true;
            break;

        case EVT_UNKNOWN:
        default:
            break;
        };
    };

    void printStats()
    {
        std::cerr << "[          ] netChanged   = " << netChanged << std::endl;
        std::cerr << "[          ] numLogged0   = " << numLogged0 << std::endl;
        std::cerr << "[          ] numLogged    = " << numLogged << std::endl;
        std::cerr << "[          ] numSent      = " << numSent << std::endl;
        std::cerr << "[          ] numDropped   = " << numDropped << std::endl;
        std::cerr << "[          ] numReject    = " << numReject << std::endl;
        std::cerr << "[          ] numCached    = " << numCached << std::endl;
        std::cerr << "[          ] numFiltered  = " << numFiltered << std::endl;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TestDebugEventListener debugListener;

const unsigned maxEventsCount = 1;

unsigned totalEvents = 0;

/*

//
// EXAMPLE #1: configure host instance via C API.
//

const char* hostConfig = JSON_CONFIG(
    {
        "cacheFilePath" : "MyOfflineStorage.db",
        "config" : {
            "host" : "Mesh-Core-C-API-Host",
            "scope" : "*"
        },
        "stats" : {
            "interval" : 0
        },
        "name" : "Mesh-Core-C-API-Host",
        "version" : "1.0.0",
        "primaryToken" : "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991",
        "maxTeardownUploadTimeInSec" : 1,
        "hostMode" : true,
        "minimumTraceLevel" : 0,
        "sdkmode" : 0,
        "disableZombieLoggers": true
    });

//
// EXAMPLE #2: configure host instance in C++ via JSON configuration above.
//

static ILogConfiguration hostConfiguration = MAT::FromJSON(hostConfig);
*/

const char* guestConfig = JSON_CONFIG(
    {
        "config" : {
            "host" : "C-API-Host",
            "scope" : "*"
        },
        "stats" : {
            "interval" : 0
        },
        "name" : "C-API-Guest",
        "version" : "1.0.0",
        "primaryToken" : "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991",
        "maxTeardownUploadTimeInSec" : 0,
        "hostMode" : false,
        "minimumTraceLevel" : 0,
        "sdkmode" : 0,
        "disableZombieLoggers" : true
    });

const char* guestConfigIsolation = JSON_CONFIG(
    {
        "config" : {
            "host" : "C-API-Host",
            "scope" : "-"
        },
        "stats" : {
            "interval" : 0
        },
        "name" : "C-API-Guest2",
        "version" : "1.0.0",
        "primaryToken" : "ffffffffffffffffffffffffffffffff-ffffffff-ffff-ffff-ffff-ffffffffffff-ffff",
        "maxTeardownUploadTimeInSec" : 0,
        "hostMode" : false,
        "minimumTraceLevel" : 0,
        "sdkmode" : 0,
        "disableZombieLoggers" : true
    });

std::time_t now = time(0);

MAT::time_ticks_t ticks(&now);

evt_prop guestContext[] = TELEMETRY_EVENT(
    _STR("ext.app.id", "com.Microsoft.Clippy"),
    _STR("ext.app.ver", "1.0.0"),
    _STR("ext.app.locale", "en-US"),
    _STR("ext.net.cost", "Unmetered"),
    _STR("ext.net.type", "QuantumLeap"));

// C-style definition of a guest event that contains iKey
evt_prop guestEvent[] = TELEMETRY_EVENT(
    // Part A/B
    _STR(COMMONFIELDS_EVENT_NAME, EVENT_NAME_GUEST),                   // Event name
    _INT(COMMONFIELDS_EVENT_TIME, static_cast<int64_t>(now * 1000L)),  // Epoch time
    _DBL("popSample", 100.0),                                          // Effective sample rate
    _STR(COMMONFIELDS_IKEY, TEST_TOKEN),                               // iKey to send this event to
//    _INT(COMMONFIELDS_EVENT_PRIORITY, static_cast<int64_t>(EventPriority_Immediate)),
//    _INT(COMMONFIELDS_EVENT_LATENCY, static_cast<int64_t>(EventLatency_Max)),
    _INT(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_REQUIRED),
    // Part C
    _STR("strKey", "value1"),
    _INT("intKey", 12345),
    _DBL("dblKey", 3.14),
    _BOOL("boolKey", true),
    _GUID("guidKey", "{01020304-0506-0708-090a-0b0c0d0e0f00}"),
    _TIME("timeKey", ticks.ticks));  // .NET ticks

// C-style definition of a guest event that uses its client instance iKey
evt_prop guestEventIsolated[] = TELEMETRY_EVENT(
    // Part A/B
    _STR(COMMONFIELDS_EVENT_NAME, EVENT_NAME_GUEST),                   // Event name
    _INT(COMMONFIELDS_EVENT_TIME, static_cast<int64_t>(now * 1000L)),  // Epoch time
    _DBL("popSample", 100.0),                                          // Effective sample rate
    _STR(COMMONFIELDS_IKEY, DUMMY_TOKEN),                              // iKey to send this event to
    // _INT(COMMONFIELDS_EVENT_PRIORITY, static_cast<int64_t>(EventPriority_Immediate)), // <-- Useful for realtime force-push
    // _INT(COMMONFIELDS_EVENT_LATENCY, static_cast<int64_t>(EventLatency_Max)),
    _INT(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_REQUIRED),
    // Part C
    _STR("strKey", "value1"),
    _INT("intKey", 12345),
    _DBL("dblKey", 3.14),
    _BOOL("boolKey", true),
    _GUID("guidKey", "{01020304-0506-0708-090a-0b0c0d0e0f00}"),
    _TIME("timeKey", ticks.ticks));  // .NET ticks

//////////////////////////////////////////////////////////////////////////////////////////
// HOST TEST
//////////////////////////////////////////////////////////////////////////////////////////
void createHostCpp()
{
    auto& cfg = LogManager::GetLogConfiguration();
    cfg["name"] = "C-API-Host";
    cfg["version"] = "1.0.0";
    cfg["config"]["host"] = "C-API-Host";
    cfg["hostMode"] = true;
    cfg["primaryToken"] = TEST_TOKEN;
    cfg[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
    cfg["stats"]["interval"] = 0;           // no stats events
    cfg["maxTeardownUploadTimeInSec"] = 0;  // fast teardown
    cfg["disableZombieLoggers"] = true;

    totalEvents = 0;
    debugListener.OnLogX = [&](::CsProtocol::Record& record)
    {
        totalEvents++;
        EXPECT_EQ(record.name, EVENT_NAME_HOST);                // Verify event name
        auto recordTimeTicks = MAT::time_ticks_t(record.time);  // Verify event time
        EXPECT_EQ(record.time, int64_t(recordTimeTicks.ticks));
        std::string iToken_o = "o:";
        iToken_o += TEST_TOKEN;
        EXPECT_THAT(iToken_o, testing::HasSubstr(record.iKey));                           // Verify event iKey
        ASSERT_STREQ(record.data[0].properties["strKey"].stringValue.c_str(), "value1");  // Verify string
        ASSERT_EQ(record.data[0].properties["intKey"].longValue, 12345);                  // Verify integer
        ASSERT_EQ(record.data[0].properties["dblKey"].doubleValue, 3.14);                 // Verify double
        ASSERT_EQ(record.data[0].properties["boolKey"].longValue, 1);                     // Verify boolean

        ASSERT_EQ(record.extDevice[0].localId, "a:4318b22fbc11ca8f");  // Verify ext.device.localId
        ASSERT_EQ(record.extProtocol[0].devMake, "Microsoft");         // NOTE the schema quirk == ext.device.make
        ASSERT_EQ(record.extProtocol[0].devModel, "Clippy");           // NOTE the schema quirk == ext.device.model
        ASSERT_EQ(record.extOs[0].name, "MS-DOS");                     // Verify ext.os.name
        ASSERT_EQ(record.extOs[0].ver, "2100");                        // Verify ext.os.ver
#ifdef ALLOW_RECORD_DECODER
        // Transform to JSON and print
        std::string s;
        exporters::DecodeRecord(record, s);
        printf(
            "*************************************** Event %u ***************************************\n%s\n",
            totalEvents,
            s.c_str());
#endif
    };

    // C++ syntax for populating Common Part A properties in context.
    const auto logger = LogManager::Initialize(TEST_TOKEN, cfg);
    EXPECT_NE(logger, nullptr);

    // Populate common Part A ext.* properties using `SetCommonField` API
    // These properties would be inherited by Guest instances.
    const auto context = LogManager::GetSemanticContext();
    context->SetCommonField("ext.device.localId", "a:4318b22fbc11ca8f");
    context->SetCommonField("ext.device.make", "Microsoft");
    context->SetCommonField("ext.device.model", "Clippy");
    context->SetCommonField("ext.os.name", "MS-DOS");
    context->SetCommonField("ext.os.ver", "2100");

    const auto instance = LogManager::GetInstance();
    EXPECT_NE(instance, nullptr);

    LogManager::AddEventListener(EVT_LOG_EVENT, debugListener);

    for (size_t i = 0; i < maxEventsCount; i++)
    {
        EventProperties props{
            EVENT_NAME_HOST,
            {{COMMONFIELDS_EVENT_TIME, static_cast<int64_t>(now * 1000L)},  // Epoch time
                               {"popSample", 100.0},                                          // Effective sample rate
                               {COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_REQUIRED},
                               {"strKey", "value1"},
                               {"intKey", 12345},
                               {"dblKey", 3.14},
                               {"boolKey", static_cast<bool>(true)}
                      }
        };
        logger->LogEvent(props);
    }

    EXPECT_EQ(totalEvents, maxEventsCount);

    // Remove debug listener
    LogManager::RemoveEventListener(EVT_LOG_EVENT, debugListener);
}

//////////////////////////////////////////////////////////////////////////////////////////
// GUEST TEST
//////////////////////////////////////////////////////////////////////////////////////////
void createGuest()
{
    totalEvents = 0;
    debugListener.OnLogX = [&](::CsProtocol::Record& record)
    {
        totalEvents++;
        EXPECT_EQ(record.name, EVENT_NAME_GUEST);               // Verify event name
        auto recordTimeTicks = MAT::time_ticks_t(record.time);  // Verify event time
        EXPECT_EQ(record.time, int64_t(recordTimeTicks.ticks));

        ASSERT_STREQ(record.data[0].properties["strKey"].stringValue.c_str(), "value1");  // Verify string
        ASSERT_EQ(record.data[0].properties["intKey"].longValue, 12345);                  // Verify integer
        ASSERT_EQ(record.data[0].properties["dblKey"].doubleValue, 3.14);                 // Verify double
        ASSERT_EQ(record.data[0].properties["boolKey"].longValue, 1);                     // Verify boolean
        auto guid = record.data[0].properties["guidKey"].guidValue[0].data();
        auto guidStr = GUID_t(guid).to_string();
        std::string guidStr2 = "01020304-0506-0708-090a-0b0c0d0e0f00";
        ASSERT_STRCASEEQ(guidStr.c_str(), guidStr2.c_str());                              // Verify GUID
        ASSERT_EQ(record.data[0].properties["timeKey"].longValue, (int64_t)ticks.ticks);  // Verify time

        // Host context properties must remain the same
        ASSERT_EQ(record.extDevice[0].localId, "a:4318b22fbc11ca8f");  // Verify ext.device.localId
        ASSERT_EQ(record.extProtocol[0].devMake, "Microsoft");         // NOTE the schema quirk == ext.device.make
        ASSERT_EQ(record.extProtocol[0].devModel, "Clippy");           // NOTE the schema quirk == ext.device.model
        ASSERT_EQ(record.extOs[0].name, "MS-DOS");                     // Verify ext.os.name
        ASSERT_EQ(record.extOs[0].ver, "2100");                        // Verify ext.os.ver

        // These new properties got appended by Guest
        ASSERT_EQ(record.extApp[0].id, "com.Microsoft.Clippy");  // Verify ext.app.id
        ASSERT_EQ(record.extApp[0].ver, "1.0.0");                // Verify ext.app.ver
        ASSERT_EQ(record.extApp[0].locale, "en-US");             // Verify ext.app.locale
        ASSERT_EQ(record.extNet[0].cost, "Unmetered");           // Verify ext.net.cost
        ASSERT_EQ(record.extNet[0].type, "QuantumLeap");         // Verify ext.net.type
#ifdef ALLOW_RECORD_DECODER
        // Transform to JSON and print
        std::string s;
        exporters::DecodeRecord(record, s);
        printf(
            "*************************************** Event %u ***************************************\n%s\n",
            totalEvents,
            s.c_str());
#endif
    };

    // Keep host LogManager running and attach Guest to it.
    const auto guestHandle = evt_open(guestConfig);
    ASSERT_NE(guestHandle, 0);
    evt_pause(guestHandle);
    const auto client = capi_get_client(guestHandle);
    ASSERT_NE(client, nullptr);
    ASSERT_NE(client->logmanager, nullptr);

    // Use parent LogManager context to append additional context variables.
    evt_set_logmanager_context(guestHandle, guestContext);

    client->logmanager->AddEventListener(EVT_LOG_EVENT, debugListener);

    for (size_t i = 0; i < maxEventsCount; i++)
    {
        evt_log(guestHandle, guestEvent);
    }

    EXPECT_EQ(totalEvents, maxEventsCount);
    evt_flush(guestHandle);

    // Remove debug listener
    client->logmanager->RemoveEventListener(EVT_LOG_EVENT, debugListener);

    // Close guest
    evt_close(guestHandle);
    ASSERT_EQ(capi_get_client(guestHandle), nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////
// GUEST TEST ISOLATED
//////////////////////////////////////////////////////////////////////////////////////////
void createGuestIsolated()
{
    // Reset total events
    totalEvents = 0;
    debugListener.OnLogX = [&](::CsProtocol::Record& record)
    {
        totalEvents++;
        EXPECT_EQ(record.name, EVENT_NAME_GUEST);               // Verify event name
        auto recordTimeTicks = MAT::time_ticks_t(record.time);  // Verify event time
        EXPECT_EQ(record.time, int64_t(recordTimeTicks.ticks));
        std::string iToken_o = "o:";
        iToken_o += DUMMY_TOKEN;
        EXPECT_THAT(iToken_o, testing::HasSubstr(record.iKey));                           // Verify event iKey
        ASSERT_STREQ(record.data[0].properties["strKey"].stringValue.c_str(), "value1");  // Verify string
        ASSERT_EQ(record.data[0].properties["intKey"].longValue, 12345);                  // Verify integer
        ASSERT_EQ(record.data[0].properties["dblKey"].doubleValue, 3.14);                 // Verify double
        ASSERT_EQ(record.data[0].properties["boolKey"].longValue, 1);                     // Verify boolean
        auto guid = record.data[0].properties["guidKey"].guidValue[0].data();
        auto guidStr = GUID_t(guid).to_string();
        std::string guidStr2 = "01020304-0506-0708-090a-0b0c0d0e0f00";
        ASSERT_STRCASEEQ(guidStr.c_str(), guidStr2.c_str());                              // Verify GUID
        ASSERT_EQ(record.data[0].properties["timeKey"].longValue, (int64_t)ticks.ticks);  // Verify time

        // Host context properties are NOT shared this time. We run in isolation.
        ASSERT_NE(record.extDevice[0].localId, "a:4318b22fbc11ca8f");  // Verify ext.device.localId
        ASSERT_NE(record.extProtocol[0].devMake, "Microsoft");         // NOTE the schema quirk == ext.device.make
        ASSERT_NE(record.extProtocol[0].devModel, "Clippy");           // NOTE the schema quirk == ext.device.model
        ASSERT_NE(record.extOs[0].name, "MS-DOS");                     // Verify ext.os.name
        ASSERT_NE(record.extOs[0].ver, "2100");                        // Verify ext.os.ver

        // These new properties got appended by Guest
        ASSERT_EQ(record.extApp[0].id, "com.Microsoft.Clippy");  // Verify ext.app.id
        ASSERT_EQ(record.extApp[0].ver, "1.0.0");                // Verify ext.app.ver
        ASSERT_EQ(record.extApp[0].locale, "en-US");             // Verify ext.app.locale
        ASSERT_EQ(record.extNet[0].cost, "Unmetered");           // Verify ext.net.cost
        ASSERT_EQ(record.extNet[0].type, "QuantumLeap");         // Verify ext.net.type
#ifdef ALLOW_RECORD_DECODER
        // Transform to JSON and print
        std::string s;
        exporters::DecodeRecord(record, s);
        printf(
            "*************************************** Event %u ***************************************\n%s\n",
            totalEvents,
            s.c_str());
#endif
    };

    // Keep host LogManager running and attach Guest to it.
    const auto guestHandle = evt_open(guestConfigIsolation);
    ASSERT_NE(guestHandle, 0);
    evt_pause(guestHandle);
    const auto client = capi_get_client(guestHandle);
    ASSERT_NE(client, nullptr);
    ASSERT_NE(client->logmanager, nullptr);

    // Guest uses its own context and not the parent context.
    evt_set_logger_context(guestHandle, guestContext);

    client->logmanager->AddEventListener(EVT_LOG_EVENT, debugListener);

    for (size_t i = 0; i < maxEventsCount; i++)
    {
        evt_log(guestHandle, guestEventIsolated);
    }

    EXPECT_EQ(totalEvents, maxEventsCount);
    evt_flush(guestHandle);

    // Remove debug listener
    client->logmanager->RemoveEventListener(EVT_LOG_EVENT, debugListener);

    // Close guest
    evt_close(guestHandle);
    ASSERT_EQ(capi_get_client(guestHandle), nullptr);
}

// Create and teardown host.
// Validate that events get emitted with right props.
TEST(HostGuestTest, C_API_CreateHost)
{
    createHostCpp();
    LogManager::FlushAndTeardown();
}

// Verify that we can create Host + Guest pair.
// Validate that events get emitted with right props.
TEST(HostGuestTest, C_API_CreateGuest)
{
    createHostCpp();
    createGuest();
    LogManager::FlushAndTeardown();
}

// Verify that we properly deallocated all resources:
// same Host + Guest pair can be recreated again.
// Validate that events get emitted with right props.
TEST(HostGuestTest, C_API_CreateGuestAgain)
{
    createHostCpp();
    createGuest();
    LogManager::FlushAndTeardown();
}

// Create Host with "sandboxed" Guest with limited scope
// that does not inherit its Host context.
// Validate that events get emitted with right props.
TEST(HostGuestTest, C_API_CreateGuestIsolated)
{
    createHostCpp();
    createGuestIsolated();
    LogManager::FlushAndTeardown();
}

// TEST_PULL_ME_IN(HostGuestTests)
