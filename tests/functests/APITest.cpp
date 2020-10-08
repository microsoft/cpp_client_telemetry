//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "mat/config.h"

#ifdef _MSC_VER
#pragma warning (disable : 4389)
#endif

//#include "gtest/gtest.h"
#include "common/Common.hpp"

#include "CsProtocol_types.hpp"

#include <atomic>
#include <cassert>
#include <LogManager.hpp>

#include "PayloadDecoder.hpp"

#include "mat.h"
#include "IDecorator.hpp"

#ifdef HAVE_MAT_JSONHPP
#include "json.hpp"
#endif

#include "CorrelationVector.hpp"

#include "http/HttpClientFactory.hpp"

#include <list>

using namespace MAT;

LOGMANAGER_INSTANCE

// 1DSCppSdkTest sandbox key
#define TEST_TOKEN      "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"

#define KILLED_TOKEN    "deadbeefdeadbeefdeadbeefdeadbeef-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"
#define TEST_TOKEN2     "0ae6cd22d8264818933f4857dd3c1472-eea5f30e-e0ed-4ab0-8ed0-4dc0f5e156e0-7385"

class TestDebugEventListener : public DebugEventListener {

public:
    std::atomic<bool>       netChanged;
    std::atomic<unsigned>   eps;
    std::atomic<unsigned>   numLogged0;
    std::atomic<unsigned>   numLogged;
    std::atomic<unsigned>   numSent;
    std::atomic<unsigned>   numDropped;
    std::atomic<unsigned>   numReject;
    std::atomic<unsigned>   numHttpError;
    std::atomic<unsigned>   numHttpOK;
    std::atomic<unsigned>   numCached;
    std::atomic<unsigned>   numFiltered;
    std::atomic<unsigned>   logLatMin;
    std::atomic<unsigned>   logLatMax;
    std::atomic<unsigned>   storageFullPct;
    std::atomic<bool>       storageFailed;

    std::function<void(::CsProtocol::Record &)> OnLogX;

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

    virtual void OnLogXDefault(::CsProtocol::Record &)
    {

    };

    void resetOnLogX()
    {
        OnLogX = [this](::CsProtocol::Record & record)
        {
            OnLogXDefault(record);
        };
    }

    virtual void OnDebugEvent(DebugEvent &evt)
    {
        switch (evt.type) {
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
                ::CsProtocol::Record & record = *static_cast<::CsProtocol::Record *>(evt.data);
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

/// <summary>
/// Add all event listeners
/// </summary>
/// <param name="listener"></param>
void addAllListeners(DebugEventListener& listener)
{
    LogManager::AddEventListener(DebugEventType::EVT_LOG_EVENT, listener);
    LogManager::AddEventListener(DebugEventType::EVT_LOG_SESSION, listener);
    LogManager::AddEventListener(DebugEventType::EVT_REJECTED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_SEND_FAILED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_SENT, listener);
    LogManager::AddEventListener(DebugEventType::EVT_DROPPED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_HTTP_OK, listener);
    LogManager::AddEventListener(DebugEventType::EVT_HTTP_ERROR, listener);
    LogManager::AddEventListener(DebugEventType::EVT_SEND_RETRY, listener);
    LogManager::AddEventListener(DebugEventType::EVT_SEND_RETRY_DROPPED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_CACHED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_NET_CHANGED, listener);
    LogManager::AddEventListener(DebugEventType::EVT_STORAGE_FULL, listener);
    LogManager::AddEventListener(DebugEventType::EVT_FILTERED, listener);
}

/// <summary>
/// Remove all event listeners
/// </summary>
/// <param name="listener"></param>
void removeAllListeners(DebugEventListener& listener)
{
    LogManager::RemoveEventListener(DebugEventType::EVT_LOG_EVENT, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_LOG_SESSION, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_REJECTED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_SEND_FAILED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_SENT, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_DROPPED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_OK, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_HTTP_ERROR, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_SEND_RETRY, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_SEND_RETRY_DROPPED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_CACHED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_NET_CHANGED, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_STORAGE_FULL, listener);
    LogManager::RemoveEventListener(DebugEventType::EVT_FILTERED, listener);
}

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

/// <summary>
/// Perform simple Initialize and FlushAndTeardown
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(APITest, LogManager_Initialize_Default_Test)
{
    ILogger *result = LogManager::Initialize(TEST_TOKEN);
    EXPECT_EQ(true, (result != NULL));
    LogManager::FlushAndTeardown();
}

/// <summary>
/// Perform Initialize and FlushAndTeardown with some options
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
TEST(APITest, LogManager_Initialize_Custom)
{
    auto &configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128; // API calls + Global mask for general messages - less SQL
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Trace;
    configuration[CFG_STR_COLLECTOR_URL] = "https://127.0.0.1/";
    ILogger *result = LogManager::Initialize(TEST_TOKEN, configuration);
    EXPECT_EQ(true, (result != NULL));
    LogManager::FlushAndTeardown();
}

#define TEST_STORAGE_FILENAME   "offlinestorage.db"

static std::string GetStoragePath()
{
    std::string fileName = MAT::GetTempDirectory();
#ifdef _WIN32
    fileName += "\\";
#else
    fileName += "/";
#endif
    fileName += TEST_STORAGE_FILENAME;
    return fileName;
}

static void CleanStorage()
{
    std::remove(GetStoragePath().c_str());
}

#if 0
/* TODO: [maxgolov] - Issue #150: test needs to be reworked. Invalid tokens might noe get sporadically 'black-holed' with 200 OK */
TEST(APITest, LogManager_KilledEventsAreDropped)
{
    constexpr static unsigned MAX_ITERATIONS = 100;
    TestDebugEventListener debugListener;

    auto &configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128; // API calls + Global mask for general messages - less SQL
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Info;
    configuration[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
    configuration[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0; // avoid sending stats for this test
    configuration[CFG_STR_CACHE_FILE_PATH] = GetStoragePath();
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 5;

    CleanStorage();
    ILogger *result = LogManager::Initialize(KILLED_TOKEN, configuration);

    addAllListeners(debugListener);

    for (int i = 0; i < 2; i++)
    {
        // Log some foo
        size_t numIterations = MAX_ITERATIONS;
        EventProperties eventToLog{ "foo1" };
        eventToLog.SetLevel(DIAG_LEVEL_REQUIRED);
        while (numIterations--)
            result->LogEvent(eventToLog);
        LogManager::UploadNow();                                    // Try to upload whatever we got
        PAL::sleep(2000);                                           // Give enough time to upload at least one event
        if (i == 0)
        {
            EXPECT_EQ(MAX_ITERATIONS, debugListener.numLogged);
            // TODO: it is possible that collector would return 503 here, in that case we may not get the 'kill-tokens' hint.
            // If it ever happens, the test would fail because the second iteration might try to upload.
            EXPECT_EQ(1u, debugListener.numHttpError);
            debugListener.numCached = 0;
        }
        if (i == 1)
        {
            // At this point we should get the error response from collector because we ingested with invalid tokens.
            // Collector should have also asked us to ban that token... Check the counts
            EXPECT_EQ(2 * MAX_ITERATIONS, debugListener.numLogged);
            EXPECT_EQ(0u, debugListener.numCached);
            EXPECT_EQ(MAX_ITERATIONS, debugListener.numDropped);
        }
    }
    LogManager::FlushAndTeardown();
    EXPECT_EQ(0u, debugListener.numCached);
    debugListener.printStats();
    removeAllListeners(debugListener);
}
#endif

TEST(APITest, LogManager_Initialize_DebugEventListener)
{
    constexpr static unsigned MAX_ITERATIONS = 100;

    TestDebugEventListener debugListener;

    auto &configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128;     // API calls + Global mask for general messages - less SQL
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;    // Don't log too much on a slow machine
    configuration[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
    configuration[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0; // avoid sending stats for this test
    configuration[CFG_STR_CACHE_FILE_PATH] = GetStoragePath();
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 5;
    configuration[CFG_INT_CACHE_FILE_SIZE] = 1024000; // 1MB
    configuration[CFG_INT_STORAGE_FULL_PCT] = 1; // 1%
    configuration[CFG_INT_STORAGE_FULL_CHECK_TIME] = 0; // 0ms
    configuration[CFG_INT_RAM_QUEUE_SIZE] = 524288; // Requires default ram queue size otherwise skips events

    EventProperties eventToLog{ "foo1" };
    eventToLog.SetLevel(DIAG_LEVEL_REQUIRED);

    CleanStorage();
    addAllListeners(debugListener);
    {
        LogManager::Initialize(TEST_TOKEN, configuration);
        LogManager::PauseTransmission();
        size_t numIterations = MAX_ITERATIONS * 1000; // 100K events
        while (numIterations--)
        {
            LogManager::GetLogger()->LogEvent(eventToLog);
        }
        LogManager::Flush();
        EXPECT_GE(debugListener.storageFullPct.load(), (unsigned)100);
        LogManager::FlushAndTeardown();

        debugListener.storageFullPct = 0;
        LogManager::Initialize(TEST_TOKEN, configuration);
        LogManager::FlushAndTeardown();
        EXPECT_EQ(debugListener.storageFullPct.load(), 0u);

    }
    debugListener.numCached = 0;
    debugListener.numSent   = 0;
    debugListener.numLogged = 0;

    CleanStorage();
    ILogger *result = LogManager::Initialize(TEST_TOKEN, configuration);

    // Log some foo
    size_t numIterations = MAX_ITERATIONS;
    while (numIterations--)
        result->LogEvent(eventToLog);
    // Check the counts
    EXPECT_EQ(MAX_ITERATIONS, debugListener.numLogged);
    EXPECT_EQ(0u, debugListener.numDropped);
    EXPECT_EQ(0u, debugListener.numReject);

    LogManager::UploadNow();             // Try to upload whatever we got
    PAL::sleep(1000);                    // Give enough time to upload at least one event
    EXPECT_NE(0u, debugListener.numSent); // Some posts must succeed within 500ms
    LogManager::PauseTransmission();     // There could still be some pending at this point
    LogManager::Flush();                 // Save all pending to disk

    numIterations = MAX_ITERATIONS;
    debugListener.numLogged = 0;         // Reset the logged counter
    debugListener.numCached = 0;         // Reset the flush counter
    EventProperties eventToStore{ "bar2" };
    eventToStore.SetLevel(DIAG_LEVEL_REQUIRED);
    while (numIterations--)
        result->LogEvent(eventToStore);  // New events go straight to offline storage
    EXPECT_EQ(MAX_ITERATIONS, debugListener.numLogged);

    LogManager::Flush();
    EXPECT_EQ(MAX_ITERATIONS, debugListener.numCached);

    LogManager::SetTransmitProfile(TransmitProfile_RealTime);
    LogManager::ResumeTransmission();
    LogManager::FlushAndTeardown();

    // Check that we sent all of logged + whatever left overs
    // prior to PauseTransmission
    EXPECT_GE(debugListener.numSent, debugListener.numLogged);
    debugListener.printStats();
    removeAllListeners(debugListener);
}

#ifdef _WIN32
TEST(APITest, LogManager_UTCSingleEventSent) {
    auto &configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128; // API calls + Global mask for general messages - less SQL
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Info;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;
    configuration[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
    configuration[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0; // avoid sending stats for this test
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 5;

    EventProperties event;

    std::string evtType = "My.Record.BaseType"; // default v1 legacy behaviour: custom.my_record_basetype
    event.SetName("MyProduct.TaggedEvent");
    event.SetType(evtType);
    event.SetProperty("result", "Success");
    event.SetProperty("random", rand());
    event.SetProperty("secret", 5.6872);
    event.SetProperty(COMMONFIELDS_EVENT_PRIVTAGS, PDT_BrowsingHistory);
    event.SetLatency(EventLatency_Normal);
    event.SetLevel(DIAG_LEVEL_REQUIRED);

    ILogger *logger = LogManager::Initialize(TEST_TOKEN, configuration);
    logger->LogEvent(event);
    LogManager::FlushAndTeardown();
}
#endif

TEST(APITest, LogManager_SemanticAPI)
{
    bool failed = false;
    try
    {
        ILogger *result = LogManager::Initialize(TEST_TOKEN);
        // ISemanticContext *context = result->GetSemanticContext();

        {
            AggregatedMetricData data("agg_metric_1", 10, 10);
            for (size_t i = 0; i < 10; i++)
                data.aggregates[AggregateType_Sum] = 0;
            EventProperties props("agg_metric_props");
            result->LogAggregatedMetric(data, props);
        }

        {
            EventProperties props("lifecycle_props");
            result->LogAppLifecycle(AppLifecycleState_Suspend, props);
            result->LogAppLifecycle(AppLifecycleState_Resume, props);
        }

        {
            EventProperties props("failure_props");
            result->LogFailure("failure", "unknown", props);
        }

        {
            EventProperties props("page_action_props");
            PageActionData data("page_action", ActionType_Unknown);
            result->LogPageAction(data, props);
        }

        LogManager::FlushAndTeardown();
    }
    catch (...)
    {
        failed = true;
    }

    /* gtest on Linux internally casts boolean to int, which results in a compiler warning with gcc */
    EXPECT_EQ(0, static_cast<int>(failed));
}

constexpr static unsigned MAX_ITERATIONS = 2000;

unsigned StressSingleThreaded(ILogConfiguration& config)
{
    TestDebugEventListener debugListener;

    addAllListeners(debugListener);
    ILogger *result = LogManager::Initialize(TEST_TOKEN, config);
    size_t numIterations = MAX_ITERATIONS;
    while (numIterations--)
    {
        EventProperties props = testing::CreateSampleEvent("event_name", EventPriority_Normal);
        result->LogEvent(props);
    }
    LogManager::FlushAndTeardown();

    unsigned retVal = debugListener.numLogged;
    removeAllListeners(debugListener);

    return retVal;
}

TEST(APITest, LogManager_Stress_SingleThreaded)
{
    auto &config = LogManager::GetLogConfiguration();
    EXPECT_GE(StressSingleThreaded(config), MAX_ITERATIONS);
}

constexpr static unsigned MAX_ITERATIONS_MT = 100;
constexpr static unsigned MAX_THREADS = 25;
/// <summary>
/// Stresses the Upload vs Teardown multi-threaded.
/// </summary>
/// <param name="config">The configuration.</param>
void StressUploadLockMultiThreaded(ILogConfiguration& config)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    TestDebugEventListener debugListener;

    addAllListeners(debugListener);
    size_t numIterations = MAX_ITERATIONS_MT;

    std::mutex m_threads_mtx;
    std::atomic<unsigned> threadCount(0);

    while (numIterations--)
    {
        ILogger *result = LogManager::Initialize(TEST_TOKEN, config);
        // Keep spawning UploadNow threads while the main thread is trying to perform
        // Initialize and Teardown, but no more than MAX_THREADS at a time.
        for (size_t i = 0; i < MAX_THREADS; i++)
        {
            if (threadCount++ < MAX_THREADS)
            {
                auto t = std::thread([&]()
                {
                    std::this_thread::yield();
                    LogManager::UploadNow();
                    const auto randTimeSub2ms = std::rand() % 2;
                    PAL::sleep(randTimeSub2ms);
                    threadCount--;
                });
                t.detach();
            }
        };
        EventProperties props = testing::CreateSampleEvent("event_name", EventPriority_Normal);
        result->LogEvent(props);
        LogManager::FlushAndTeardown();
    }
    removeAllListeners(debugListener);
}

TEST(APITest, LogManager_StressUploadLock_MultiThreaded)
{
    auto &config = LogManager::GetLogConfiguration();
    config[CFG_INT_MAX_TEARDOWN_TIME] = 0;
    StressUploadLockMultiThreaded(config);
    // Basic expectation here is just that we do not crash..
    // We can add memory utilization metric in here as well.
}


TEST(APITest, LogManager_Reinitialize_Test)
{
    size_t numIterations = 5;
    while (numIterations--)
    {
        ILogger *result = LogManager::Initialize(TEST_TOKEN);
        EXPECT_EQ(true, (result != NULL));
        LogManager::FlushAndTeardown();
    }
}

#define EVENT_NAME_PURE_C   "Event.Name.Pure.C"
#define JSON_CONFIG(...)    #__VA_ARGS__
TEST(APITest, C_API_Test)
{
    TestDebugEventListener debugListener;

    // Using some cool macro-magic trick to populate well-formed JSON in a neat way.
    // Unfortunately that trick does not allow to use variables or other macros in
    // config, but generally well-suited for illustrative purposes, to create easy-
    // to-read JSON config file. Note __VA_ARGS__ substitution is a C++11 feature
    // that isn't avail in C99
    const char* config = JSON_CONFIG(
        {
            "cacheFilePath": "MyOfflineStorage.db",
            "config" : {
                "host": "*"
            },
            "stats" : {
                "interval": 0
            },
            "name" : "C-API-Client-0",
            "version" : "1.0.0",
            "primaryToken" : "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991",
            "maxTeardownUploadTimeInSec" : 5,
            "hostMode" : false,
            "minimumTraceLevel" : 0,
            "sdkmode" : 0
        }
    );

    std::time_t now = time(0);
    MAT::time_ticks_t ticks(&now);

    evt_prop event[] = TELEMETRY_EVENT
    (
        // Part A/B fields
        _STR(COMMONFIELDS_EVENT_NAME, EVENT_NAME_PURE_C),                  // Event name
        _INT(COMMONFIELDS_EVENT_TIME, static_cast<int64_t>(now * 1000L)),  // Epoch time in millis, ms since Jan 01 1970. (UTC)
        _DBL("popSample", 100.0),                                          // Effective sample rate
        _STR(COMMONFIELDS_IKEY, TEST_TOKEN),                               // iKey to send this event to
        _INT(COMMONFIELDS_EVENT_POLICYFLAGS, 0xffffffff),                  // UTC policy bitflags (optional)
        _INT(COMMONFIELDS_EVENT_PRIORITY, static_cast<int64_t>(EventPriority_Immediate)),
        _INT(COMMONFIELDS_EVENT_LATENCY, static_cast<int64_t>(EventLatency_Max)),
        _INT(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_REQUIRED),
        // Customer Data fields go as part of userdata
        _STR("strKey", "value1"),
        _INT("intKey", 12345),
        _DBL("dblKey", 3.14),
        _BOOL("boolKey", true),
        _GUID("guidKey", "{01020304-0506-0708-090a-0b0c0d0e0f00}" ),
        _TIME("timeKey", ticks.ticks),                          // .NET ticks
        // All Pii types get treated as strings by the backend
        PII_STR("piiKey", "secret", (int)PiiKind_Identity)
    );
    // event[2].value.as_double = 100.0f;

    unsigned totalEvents = 0;
    debugListener.OnLogX = [&](::CsProtocol::Record & record)
    {
        totalEvents++;
        // Verify event name
        EXPECT_EQ(record.name,  EVENT_NAME_PURE_C);
        // Verify event time
        auto recordTimeTicks = MAT::time_ticks_t(record.time);
        EXPECT_EQ(record.time, int64_t(recordTimeTicks.ticks) );
        // Verify event iKey
        std::string iToken_o = "o:";
        iToken_o += TEST_TOKEN;
        EXPECT_THAT(iToken_o, testing::HasSubstr(record.iKey));
        // Verify string
        ASSERT_STREQ(record.data[0].properties["strKey"].stringValue.c_str(), "value1");
        // Verify integer
        ASSERT_EQ(record.data[0].properties["intKey"].longValue, 12345);
        // Verify double
        ASSERT_EQ(record.data[0].properties["dblKey"].doubleValue, 3.14);
        // Verify boolean
        ASSERT_EQ(record.data[0].properties["boolKey"].longValue, 1);
        // Verify GUID
        auto guid = record.data[0].properties["guidKey"].guidValue[0].data();
        auto guidStr = GUID_t(guid).to_string();
        std::string guidStr2 = "01020304-0506-0708-090a-0b0c0d0e0f00";
        ASSERT_STRCASEEQ(guidStr.c_str(), guidStr2.c_str());
        // Verify time
        ASSERT_EQ(record.data[0].properties["timeKey"].longValue, (int64_t)ticks.ticks);
    };

    evt_handle_t handle = evt_open(config);
    ASSERT_NE(handle, 0);

    capi_client *client = capi_get_client(handle);
    ASSERT_NE(client, nullptr);
    ASSERT_NE(client->logmanager, nullptr);

    // Bind from C API LogManager instance to C++ DebugEventListener
    // to verify event contents. Currently we do not support registering
    // debug callbacks via C API, so we obtain the ILogManager first,
    // then register event listener on it.
    client->logmanager->AddEventListener(EVT_LOG_EVENT, debugListener);

    // Ingest 5 events via C API
    for (size_t i = 0; i < 5; i++)
    {
        evt_log(handle, event);
    }
    EXPECT_EQ(totalEvents, 5u);

    evt_flush(handle);
    evt_upload(handle);

    // Must remove event listener befor closing the handle!
    client->logmanager->RemoveEventListener(EVT_LOG_EVENT, debugListener);
    evt_close(handle);
    ASSERT_EQ(capi_get_client(handle), nullptr);

    // Re-open with the same configuration
    handle = evt_open(config);
    ASSERT_NE(handle, 0);
    client = capi_get_client(handle);
    ASSERT_NE(client, nullptr);
    ASSERT_NE(client->logmanager, nullptr);

    // Re-close
    evt_close(handle);
    ASSERT_EQ(capi_get_client(handle), nullptr);
}

#ifdef HAVE_MAT_JSONHPP
#if defined(_WIN32)
TEST(APITest, UTC_Callback_Test)
{
    TestDebugEventListener debugListener;

    auto configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;

    std::time_t now = time(0);
    MAT::time_ticks_t ticks(&now);

    LogManager::AddEventListener(EVT_LOG_EVENT, debugListener);
    auto logger = LogManager::Initialize(TEST_TOKEN);
    unsigned totalEvents = 0;
    debugListener.OnLogX = [&](::CsProtocol::Record & record)
    {
        totalEvents++;
        // Verify event name
        EXPECT_EQ(record.name, "MyProduct.UtcEvent");
        // Verify event time
        auto recordTimeTicks = MAT::time_ticks_t(record.time);
        EXPECT_EQ(record.time, int64_t(recordTimeTicks.ticks));
        // Verify event iKey
        std::string iToken_o = "o:";
        iToken_o += TEST_TOKEN;
        EXPECT_THAT(iToken_o, testing::HasSubstr(record.iKey));
        // Verify string
        ASSERT_STREQ(record.data[0].properties["strKey"].stringValue.c_str(), "value1");
        // Verify integer
        ASSERT_EQ(record.data[0].properties["intKey"].longValue, 12345);
        // Verify double
        ASSERT_EQ(record.data[0].properties["dblKey"].doubleValue, 3.14);
        // Verify boolean
        ASSERT_EQ(record.data[0].properties["boolKey"].longValue, 1);
        // Verify GUID
        auto guid = record.data[0].properties["guidKey"].guidValue[0].data();
        auto guidStr = GUID_t(guid).to_string();
        std::string guidStr2 = "01020304-0506-0708-090a-0b0c0d0e0f00";
        ASSERT_STRCASEEQ(guidStr.c_str(), guidStr2.c_str());
        // Verify time
        ASSERT_EQ(record.data[0].properties["timeKey"].longValue, (int64_t)ticks.ticks);

        // Transform to JSON and print
        std::string s;
        exporters::DecodeRecord(record, s);
        printf(
            "*************************************** Event %u ***************************************\n%s\n",
            totalEvents,
            s.c_str()
        );
    };

    // Ingest 3 events via C++ API in UTC mode. Callback function above intercepts
    // these events as CsRecord, then invokes PayloadDecoder to represent as JSON.
    for (size_t i = 0; i < 3; i++)
    {
        EventProperties event("MyProduct.UtcEvent",
        {
            { "strKey", "value1" },
            { "intKey", 12345 },
            { "dblKey", 3.14 },
            { "boolKey", true },
            { "guidKey", GUID_t("{01020304-0506-0708-090a-0b0c0d0e0f00}") },
            { "timeKey", ticks }
        });
        event.SetTimestamp((int64_t)(now * 1000L));
        logger->LogEvent(event);
    }
    LogManager::FlushAndTeardown();
    LogManager::RemoveEventListener(EVT_LOG_EVENT, debugListener);
}
#endif

TEST(APITest, Pii_DROP_Test)
{
    TestDebugEventListener debugListener;

    auto config = LogManager::GetLogConfiguration();
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    config[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0;        // avoid sending stats for this test
    config[CFG_INT_MAX_TEARDOWN_TIME] = 1;  // give enough time to upload

    // register a listener
    LogManager::AddEventListener(EVT_LOG_EVENT, debugListener);
    auto logger = LogManager::Initialize(TEST_TOKEN);
    unsigned totalEvents = 0;
    std::string realDeviceId;

    // verify that we get one regular event with real device id,
    // as well as more events with anonymous random device id.
    debugListener.OnLogX = [&](::CsProtocol::Record & record)
    {
        totalEvents++;
        if (record.name == "Regular.Event")
        {
            // usual event with proper SDK-obtained localId
            realDeviceId = record.extDevice[0].localId;
            EXPECT_STREQ(record.extUser[0].localId.c_str(), "c:1234567890");
            return;
        }

        ASSERT_EQ(record.extProtocol[0].ticketKeys.size(), 0ul);
        // more events with random device id
        EXPECT_STRNE(record.extDevice[0].localId.c_str(), realDeviceId.c_str());
        EXPECT_STREQ(record.extDevice[0].authId.c_str(), "");
        EXPECT_STREQ(record.extDevice[0].authSecId.c_str(), "");
        EXPECT_STREQ(record.extDevice[0].id.c_str(), "");
        // ext.user.localId stripped
        EXPECT_STREQ(record.extUser[0].localId.c_str(), "");
        EXPECT_STREQ(record.extUser[0].authId.c_str(), "");
        EXPECT_STREQ(record.extUser[0].id.c_str(), "");
        // SDK tracking cookies stripped
        EXPECT_EQ(record.extSdk[0].seq, 0);
        EXPECT_STREQ(record.extSdk[0].epoch.c_str(), "");
        EXPECT_STREQ(record.extSdk[0].installId.c_str(), "");
        // cV stripped
        EXPECT_STREQ(record.cV.c_str(), "");
    };

    auto context = logger->GetSemanticContext();
    context->SetUserId("c:1234567890");

    // Set some random cV
    CorrelationVector m_appCV;
    m_appCV.SetValue("jj9XLhDw7EuXoC2L");
    // Extend that value.
    m_appCV.Extend();
    // Get the next value, log it and/or pass it to your downstream dependency.
    std::string curCV = m_appCV.GetNextValue();

    logger->LogEvent("Regular.Event");

    for (size_t i = 0; i < 3; i++)
    {
        EventProperties event("PiiDrop.Event",
        {
            { "strKey", "some string" }
        });

        event.SetPolicyBitFlags(MICROSOFT_EVENTTAG_DROP_PII);
        event.SetProperty(CorrelationVector::PropertyName, curCV);
        logger->LogEvent(event);
    }

    LogManager::FlushAndTeardown();
    ASSERT_EQ(totalEvents, 4u);
    LogManager::RemoveEventListener(EVT_LOG_EVENT, debugListener);

}

#endif

TEST(APITest, SemanticContext_Test)
{
    TestDebugEventListener debugListener;

    auto config = LogManager::GetLogConfiguration();
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    config[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0;        // avoid sending stats for this test
    config[CFG_INT_MAX_TEARDOWN_TIME] = 1;  // give enough time to upload

    // register a listener
    LogManager::AddEventListener(EVT_LOG_EVENT, debugListener);

    CleanStorage();
    auto logger = LogManager::Initialize(TEST_TOKEN);
    unsigned totalEvents = 0;

    // Verify that semantic context fields have been set on record
    debugListener.OnLogX = [&](::CsProtocol::Record& record) {
        totalEvents++;
        if (record.name == "LoggerContext.Event")
        {
            // App extension
            EXPECT_STREQ(record.extApp[0].env.c_str(), "dev");
            EXPECT_STREQ(record.extApp[0].id.c_str(), "myAppId");
            EXPECT_STREQ(record.extApp[0].locale.c_str(), "en-US");
            EXPECT_STREQ(record.extApp[0].name.c_str(), "myAppName");
            EXPECT_STREQ(record.extApp[0].ver.c_str(), "1.2.3");
            // Device extension
            EXPECT_STREQ(record.extDevice[0].deviceClass.c_str(), "Custom.Desktop");
            EXPECT_STREQ(record.extDevice[0].localId.c_str(), "c:1234567890");
            // Legacy schema quirk that forces SDK to send devMake and devModel under protocol extension
            EXPECT_STREQ(record.extProtocol[0].devMake.c_str(), "Make");
            EXPECT_STREQ(record.extProtocol[0].devModel.c_str(), "Model");
            // Commercial Id
            EXPECT_STREQ(record.extM365a[0].enrolledTenantId.c_str(), "1-2-3-4-5");
            // Network extension
            EXPECT_STREQ(record.extNet[0].cost.c_str(), "Unmetered");
            EXPECT_STREQ(record.extNet[0].provider.c_str(), "Provider");
            EXPECT_STREQ(record.extNet[0].type.c_str(), "Wifi");
            // OS extension. 1DS SDK maps OS Build semantic context field to ext.os.ver
            EXPECT_STREQ(record.extOs[0].ver.c_str(), "os-build");
            EXPECT_STREQ(record.extOs[0].name.c_str(), "os-name");
            // User extension
            EXPECT_STREQ(record.extUser[0].localId.c_str(), "localUserId");
            EXPECT_STREQ(record.extUser[0].locale.c_str(), "en-US");
            EXPECT_STREQ(record.extLoc[0].timezone.c_str(), "+01:00");
        }
    };

    auto context = logger->GetSemanticContext();
    // App extension
    context->SetAppEnv("dev");
    context->SetAppId("myAppId");
    context->SetAppLanguage("en-US");
    context->SetAppName("myAppName");
    context->SetAppVersion("1.2.3");
    // Device extension
    context->SetDeviceClass("Custom.Desktop");
    context->SetDeviceId("c:1234567890");
    context->SetDeviceMake("Make");
    context->SetDeviceModel("Model");
    // Commercial Id aka. Office Enrolled Tenant Id
    context->SetCommercialId("1-2-3-4-5");
    // Network extension
    context->SetNetworkCost(NetworkCost_Unmetered);
    context->SetNetworkProvider("Provider");
    context->SetNetworkType(NetworkType_Wifi);
    // OS extension
    context->SetOsBuild("os-build");
    context->SetOsName("os-name");
    context->SetOsVersion("1.0.0");
    // User extension
    context->SetUserId("localUserId");
    context->SetUserLanguage("en-US");
    context->SetUserTimeZone("+01:00");

    logger->LogEvent("LoggerContext.Event");

    LogManager::FlushAndTeardown();

    ASSERT_EQ(totalEvents, 1u);
    LogManager::RemoveEventListener(EVT_LOG_EVENT, debugListener);
}

static void logBenchMark(const char * label)
{
#ifdef DEBUG_PERF
    static int64_t lastTime = GetUptimeMs();
    int64_t currTime = GetUptimeMs();
    printf("%s: ... %lld\n", label, (currTime - lastTime));
    lastTime = currTime;
#else
    UNREFERENCED_PARAMETER(label);
#endif
}

TEST(APITest, LogManager_Reinitialize_UploadNow)
{
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    size_t numIterations = 10;
    const size_t shutdownSec = 20; // Must complete 10 runs in 20 seconds on Release
                                   // Max teardown time is ~1s per run, maybe up to 2s
                                   // This time may be longer on a slow box, e.g. build server

    bool flipFlop = true;
    while (numIterations--)
    {
        logBenchMark("started");
        auto& config = LogManager::GetLogConfiguration();
        logBenchMark("created");

        config[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF;
        config[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Trace;
        config[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
        config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
        config[CFG_INT_MAX_TEARDOWN_TIME] = 1;
        logBenchMark("config ");

        ILogger *logger = LogManager::Initialize(TEST_TOKEN, config);
        logBenchMark("inited ");

        logger->LogEvent("test");
        logBenchMark("logged ");

        // Try to switch transmit profile
        LogManager::SetTransmitProfile(TRANSMITPROFILE_REALTIME);
        logBenchMark("profile");

        if (flipFlop)
        {
            LogManager::PauseTransmission();
        }
        else
        {
            LogManager::ResumeTransmission();
        }
        logBenchMark("flipflp");

        logBenchMark("upload ");
        LogManager::UploadNow();

        logBenchMark("flush  ");
        LogManager::FlushAndTeardown();

        logBenchMark("done   ");
        flipFlop = !flipFlop;
    }

    std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
    uint64_t total_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

    EXPECT_GE(shutdownSec + 1, total_time);
}

TEST(APITest, LogManager_BadStoragePath_Test)
{
    TestDebugEventListener debugListener;
    auto &config = LogManager::GetLogConfiguration();
    config[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF; // API calls + Global mask for general messages - less SQL
    config[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Trace;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 16;

    std::vector<std::string> paths =
    {
#ifdef _WIN32
        "T:\\invalid\\file\\path",               // This fails - no offline storage
        u8"C:\\неправильный\\каталог\\utf-8",    // This fails - no offline storage
        u8"C:\\Проверка-проверка 1 2 3\\файл.db" // This should pass if dir exists
#else
        "/invalid/file/path",
        u8"/неправильный/каталог/utf-8",         // This fails - no offline storage
        u8"/Проверка-проверка 1 2 3/файл.db"     // This should pass if dir exists
#endif
    };

    for (const auto &path : paths)
    {
        debugListener.storageFailed = false;
        config[CFG_STR_CACHE_FILE_PATH] = path.c_str();
        ILogger *result = LogManager::Initialize(TEST_TOKEN, config);
        LogManager::AddEventListener(DebugEventType::EVT_STORAGE_FAILED, debugListener);
        EXPECT_EQ(true, (result != NULL));
        result->LogEvent("test");
        LogManager::Flush();
        LogManager::FlushAndTeardown();
        LogManager::RemoveEventListener(DebugEventType::EVT_STORAGE_FAILED, debugListener);
        EXPECT_EQ(true, debugListener.storageFailed);
    }

}

#ifdef HAVE_MAT_WININET_HTTP_CLIENT
/* This test requires WinInet HTTP client */
TEST(APITest, LogConfiguration_MsRoot_Check)
{
    TestDebugEventListener debugListener;
    std::list<std::tuple<std::string, bool, unsigned>> testParams =
        {
            {"https://v10.events.data.microsoft.com/OneCollector/1.0/", false, 1},   // MS-Rooted, no MS-Root check:     post succeeds
            {"https://v10.events.data.microsoft.com/OneCollector/1.0/", true, 1},    // MS-Rooted, MS-Root check:        post succeeds
            {"https://self.events.data.microsoft.com/OneCollector/1.0/", false, 1},  // Non-MS rooted, no MS-Root check: post succeeds
            {"https://self.events.data.microsoft.com/OneCollector/1.0/", true, 0}    // Non-MS rooted, MS-Root check:    post fails
        };

    // 4 test runs
    for (const auto& params : testParams)
    {
        CleanStorage();

        auto& config = LogManager::GetLogConfiguration();
        config[CFG_MAP_METASTATS_CONFIG][CFG_INT_METASTATS_INTERVAL] = 0;  // avoid sending stats for this test, just customer events
        config[CFG_STR_COLLECTOR_URL] = std::get<0>(params);
        config[CFG_MAP_HTTP][CFG_BOOL_HTTP_MS_ROOT_CHECK] = std::get<1>(params);  // MS root check depends on what URL we are sending to
        config[CFG_INT_MAX_TEARDOWN_TIME] = 1;                // up to 1s wait to perform HTTP post on teardown
        config[CFG_STR_CACHE_FILE_PATH] = GetStoragePath();
        auto expectedHttpCount = std::get<2>(params);

        auto logger = LogManager::Initialize(TEST_TOKEN, config);

        debugListener.reset();
        addAllListeners(debugListener);
        logger->LogEvent("fooBar");
        LogManager::FlushAndTeardown();
        removeAllListeners(debugListener);

        // Connection is a best-effort, occasionally we can't connect,
        // but we MUST NOT connect to end-point that doesn't have the
        // right cert.
        EXPECT_LE(debugListener.numHttpOK, expectedHttpCount);
    }
}
#endif
TEST(APITest, LogManager_BadNetwork_Test)
{
    auto& config = LogManager::GetLogConfiguration();

    // Clean temp file first
    const char *cacheFilePath = "bad-network.db";
    std::string fileName = MAT::GetTempDirectory();
    fileName += "\\";
    fileName += cacheFilePath;
    printf("remove %s\n", fileName.c_str());
    std::remove(fileName.c_str());

    for (auto url : {
#if 0 /* [MG}: Temporary change to avoid GitHub Actions crash #92 */
        "https://0.0.0.0/",
        "https://127.0.0.1/",
#endif
        "https://1ds.pipe.int.trafficmanager.net/OneCollector/1.0/",
        "https://invalid.host.name.microsoft.com/"
        })
    {
        printf("--- trying %s", url);
        config[CFG_STR_CACHE_FILE_PATH] = cacheFilePath;
        config[CFG_INT_TRACE_LEVEL_MASK] = 0;
        config[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn;
        config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
        config[CFG_INT_MAX_TEARDOWN_TIME] = 0;
        config[CFG_STR_COLLECTOR_URL] = url;
        size_t numIterations = 5;
        while (numIterations--)
        {
            printf(".");
            EXPECT_GE(StressSingleThreaded(config), MAX_ITERATIONS);
        }
        printf("\n");
    }
}

TEST(APITest, LogManager_GetLoggerSameLoggerMultithreaded)
{
    auto& config = LogManager::GetLogConfiguration();

    auto logger0 = LogManager::Initialize(TEST_TOKEN, config);
    auto logger1 = LogManager::GetLogger();
    auto logger2 = LogManager::GetLogger("my_source");
    auto logger3 = LogManager::GetLogger(TEST_TOKEN, "my_source");

    EXPECT_EQ(logger0, logger1);
    EXPECT_EQ(logger2, logger3);
    EXPECT_NE(logger0, logger2);

    EXPECT_NE(logger0, nullptr);
    EXPECT_NE(logger1, nullptr);
    EXPECT_NE(logger2, nullptr);
    EXPECT_NE(logger3, nullptr);

    logBenchMark("created");

    size_t numThreads = 10;
    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; i++)
    {
        threads.push_back(std::thread([logger1, logger2, logger3]() {
            size_t count = 1000;
            while (count--)
            {
                auto myLogger1 = LogManager::GetLogger();
                auto myLogger2 = LogManager::GetLogger("my_source");
                auto myLogger3 = LogManager::GetLogger(TEST_TOKEN, "my_source");
                EXPECT_EQ(myLogger1, logger1);
                EXPECT_EQ(myLogger2, logger2);
                EXPECT_EQ(myLogger3, logger3);
            }
        }));
    }
    // Wait for completion
    for (auto &thread : threads)
        thread.join();
    logBenchMark("destroyed");
    LogManager::FlushAndTeardown();

    LogManager::GetLogger();
}

TEST(APITest, LogManager_DiagLevels)
{
    TestDebugEventListener eventListener;

    auto& config = LogManager::GetLogConfiguration();

    // default
    auto logger0 = LogManager::Initialize(TEST_TOKEN, config);
    LogManager::GetEventFilters().UnregisterAllFilters();

    // inherit diagnostic level from parent (basic)
    auto logger1 = LogManager::GetLogger();

    // set diagnostic level to optional
    auto logger2 = LogManager::GetLogger(TEST_TOKEN, "my_optional_source");
    logger2->SetLevel(DIAG_LEVEL_OPTIONAL);

    // set diagnostic level to a custom value
    auto logger3 = LogManager::GetLogger("my_custom_source");
    logger3->SetLevel(5);
    
    std::set<uint8_t> logNone  = { DIAG_LEVEL_NONE };
    std::set<uint8_t> logAll   = { };
    std::set<uint8_t> logRequired = { DIAG_LEVEL_REQUIRED };

    auto filters = { logNone, logAll, logRequired };

    size_t expectedCounts[] = { 12, 0, 8 };

    addAllListeners(eventListener);

    // Filter test
    size_t i = 0;
    for (auto filter : filters)
    {
        // Specify diagnostic level filter
        LogManager::SetLevelFilter(DIAG_LEVEL_DEFAULT, filter);
        for (auto logger : { logger0, logger1, logger2, logger3 })
        {
            EventProperties defLevelEvent("My.DefaultLevelEvent");
            logger->LogEvent(defLevelEvent);   // inherit from logger

            EventProperties requiredEvent("My.RequiredEvent");
            requiredEvent.SetLevel(DIAG_LEVEL_REQUIRED);
            logger->LogEvent(requiredEvent);   // required

            EventProperties customEvent("My.CustomEvent");
            customEvent.SetLevel(5);
            logger->LogEvent(customEvent);
        }
        EXPECT_EQ(eventListener.numFiltered, expectedCounts[i]);
        eventListener.numFiltered = 0;
        i++;
    }
    removeAllListeners(eventListener);
    LogManager::FlushAndTeardown();
}

TEST(APITest, Pii_Kind_E2E_Test)
{
    auto& config = LogManager::GetLogConfiguration();
    LogManager::Initialize(TEST_TOKEN, config);
    // Log detailed event with various properties
    EventProperties detailed_event("MyApp.DetailedEvent.Pii",
        {
#ifdef _MSC_VER
            // Log compiler version
            { "_MSC_VER", _MSC_VER },
#endif
            // Pii-typed fields
            { "piiKind.None",               EventProperty("field_value",  PiiKind_None) },
            { "piiKind.DistinguishedName",  EventProperty("/CN=Jack Frost,OU=PIE,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
            { "piiKind.GenericData",        EventProperty("generic_data",  PiiKind_GenericData) },
            { "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
            { "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
            { "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
            { "piiKind.PhoneNumber",        EventProperty("+1-425-829-5875", PiiKind_PhoneNumber) },
            { "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
            { "piiKind.SipAddress",         EventProperty("sip:info@microsoft.com", PiiKind_SipAddress) },
            { "piiKind.SmtpAddress",        EventProperty("Jack Frost <jackfrost@fabrikam.com>", PiiKind_SmtpAddress) },
            { "piiKind.Identity",           EventProperty("Jack Frost", PiiKind_Identity) },
            { "piiKind.Uri",                EventProperty("http://www.microsoft.com", PiiKind_Uri) },
            { "piiKind.Fqdn",               EventProperty("www.microsoft.com", PiiKind_Fqdn) },
            // Various typed key-values
            { "strKey1",  "hello1" },
            { "strKey2",  "hello2" },
            { "int64Key", (int64_t)1L },
            { "dblKey",   3.14 },
            { "boolKey",  false },
            { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
            { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
            { "guidKey2", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
            { "timeKey1",  time_ticks_t((uint64_t)0) },     // time in .NET ticks
        });
    auto logger = LogManager::GetLogger();
    EXPECT_NE(logger, nullptr);
    logger->LogEvent(detailed_event);
    LogManager::FlushAndTeardown();
    // Verify that contents get hashed by server
}

class CustomDecorator: public IDecoratorModule
{
public:
    /***
     * Note that C++ bond definition for extensions assumes that every extension is a vector,
     * such as a record that may have 0 or 1 elements in that vector. Collector only respects
     * the first item in extension vector, the others are discarded. Please do not try to
     * populate the extX[1], as it would be effectively discarded.
     */
    virtual bool decorate(CsProtocol::Record& record) override
    {
        // App Environment
        record.extApp[0].env = "insiders";

        // UTC flags override for direct upload mode only.
        // Resize with care: by default SDK does not populate that extension.
        record.extUtc.resize(1);
        record.extUtc[0].flags = 0x12345;

        // Device Org Id, e.g. obtained via InTune API
        record.extDevice[0].orgId = "00010203-0405-0607-0809-0A0B0C0D0E0F";

        // App-specific Corellation Vector to track certain scenarios.
        // Ref: https://github.com/microsoft/CorrelationVector/blob/master/cV%20-%203.0.md
        record.cV = "A.PmvzQKgYek6Sdk/T5sWaqw.B";

        // Iterate over Part C (custom) data.
        forEachPartC(record);

        // Custom global decorator is invoked before sending event further for serialization.
        return true;
    }

    void forEachPartC(CsProtocol::Record& record)
    {
        printf("Event Name: %s\n", record.name.c_str());
        if (record.data.size() == 1)
        {
            for (const auto& props : record.data[0].properties)
            {
                auto key = props.first.c_str();
                auto val = props.second;
                switch (val.type)
                {

                case CsProtocol::ValueKind::ValueString:
                {
                    // Print string values
                    printf("- %s=%s\n", key, val.stringValue.c_str());
                    break;
                }

                case CsProtocol::ValueKind::ValueGuid:
                {
                    // Extract from byte array and print GUID values as string
                    uint8_t guidBytes[16];
                    std::copy(val.guidValue[0].begin(), val.guidValue[0].end(), guidBytes);
                    // We use GUID_t::to_string(...) converter to print string
                    GUID_t guid(guidBytes);
                    printf("- %s=%s\n", key, guid.to_string().c_str());
                    break;
                }

                default:
                    // Ignore all numeric types for now
                    break;
                }
            }
        }
    }

};

CustomDecorator myDecorator;

TEST(APITest, Custom_Decorator)
{
    auto& config = LogManager::GetLogConfiguration();
    config.AddModule(CFG_MODULE_DECORATOR, std::make_shared<CustomDecorator>(myDecorator) );
    LogManager::Initialize(TEST_TOKEN, config);
    LogManager::GetLogger()->LogEvent("foobar");
    EventProperties myEvent2("MyEvent.With.Props",
    {
        {"keyString", "Hello World!"},
        {"keyGuid", GUID_t("{76ce7649-3a58-4861-8202-7d7fdfaed483}")}
    });
    LogManager::GetLogger()->LogEvent(myEvent2);
    LogManager::FlushAndTeardown();
    // In-lieu of RemoveModule(...) the current solution is to set the module to nullptr.
    // This is functionally nearly equivalent to unsetting it since GetModule(CFG_MODULE_DECORATOR)
    // for non-existing module also returns nullptr.
    config.AddModule(CFG_MODULE_DECORATOR, nullptr);
}

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

// TEST_PULL_ME_IN(APITest)

