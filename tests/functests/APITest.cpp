#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif 
#pragma warning (disable : 4389)

//#include "gtest/gtest.h"
#include "common/Common.hpp"

#include <atomic>
#include <cassert>
#include <LogManager.hpp>

using namespace MAT;

LOGMANAGER_INSTANCE

constexpr static const char* TEST_TOKEN  = "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322";
constexpr static const char* TEST_TOKEN2 = "0ae6cd22d8264818933f4857dd3c1472-eea5f30e-e0ed-4ab0-8ed0-4dc0f5e156e0-7385";

class TestDebugEventListener : public DebugEventListener {

public:
    std::atomic<bool>       netChanged = false;
    std::atomic<unsigned>   eps        = 0;
    std::atomic<unsigned>   numLogged0 = 0;
    std::atomic<unsigned>   numLogged  = 0;
    std::atomic<unsigned>   numSent    = 0;
    std::atomic<unsigned>   numDropped = 0;
    std::atomic<unsigned>   numReject  = 0;
    std::atomic<unsigned>   numCached  = 0;
    std::atomic<unsigned>   logLatMin  = 100;
    std::atomic<unsigned>   logLatMax  = 0;

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
            numLogged++;
            break;

        case EVT_REJECTED:
            numReject++;
            break;

        case EVT_ADDED:
            break;

        /* Event counts below would never overflow the size of unsigned int */
        case EVT_CACHED:
            numCached+= (unsigned int)evt.param1;
            break;

        case EVT_DROPPED:
            numDropped+= (unsigned int)evt.param1;
            break;

        case EVT_SENT:
            numSent += (unsigned int)evt.param1;
            break;

        case EVT_STORAGE_FULL:
            break;

        case EVT_CONN_FAILURE:
        case EVT_HTTP_FAILURE:
        case EVT_COMPRESS_FAILED:
        case EVT_UNKNOWN_HOST:
        case EVT_SEND_FAILED:
        case EVT_HTTP_ERROR:
        case EVT_HTTP_OK:
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
        std::cerr << "[          ] netChanged = " << netChanged << std::endl;
        std::cerr << "[          ] numLogged0 = " << numLogged0 << std::endl;
        std::cerr << "[          ] numLogged  = " << numLogged  << std::endl;
        std::cerr << "[          ] numSent    = " << numSent    << std::endl;
        std::cerr << "[          ] numDropped = " << numDropped << std::endl;
        std::cerr << "[          ] numReject  = " << numReject  << std::endl;
        std::cerr << "[          ] numCached  = " << numCached  << std::endl;
    }
};

/// <summary>
/// Create sample event of a given priority
/// </summary>
/// <param name="name">event name</param>
/// <param name="prio">priority</param>
/// <returns></returns>
EventProperties CreateSampleEvent(const char *name, EventPriority prio)
{
    GUID win_guid;
    win_guid.Data1 = 0;
    win_guid.Data2 = 1;
    win_guid.Data3 = 2;

    for (uint8_t i = 0; i < 8; i++)
    {
        win_guid.Data4[i] = i;
    }

    // GUID constructor from byte[16]
    const uint8_t guid_b[16] = {
        0x03, 0x02, 0x01, 0x00,
        0x05, 0x04,
        0x07, 0x06,
        0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    GUID_t guid_c(
        0x00010203,
        0x0405,
        0x0607,
        { 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }
    );

    const GUID_t guid_d;

    // Prepare current time in UTC (seconds precision)
    std::time_t t = std::time(nullptr);
    std::gmtime(&t);

    /* С++11 constructor for Visual Studio 2015: this is the most JSON-lookalike syntax that makes use of C++11 initializer lists. */
    EventProperties props(name,
    {
        { "_MSC_VER", _MSC_VER },

        { "piiKind.None",               EventProperty("maxgolov",  PiiKind_None) },
        { "piiKind.DistinguishedName",  EventProperty("/CN=Max Golovanov,OU=ARIA,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
        { "piiKind.GenericData",        EventProperty("maxgolov",  PiiKind_GenericData) },
        { "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
        { "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
        { "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
        { "piiKind.PhoneNumber",        EventProperty("+1-613-866-6960", PiiKind_PhoneNumber) },
        { "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
        { "piiKind.SipAddress",         EventProperty("sip:maxgolov@microsoft.com", PiiKind_SipAddress) },
        { "piiKind.SmtpAddress",        EventProperty("Max Golovanov <maxgolov@microsoft.com>", PiiKind_SmtpAddress) },
        { "piiKind.Identity",           EventProperty("Max Golovanov", PiiKind_Identity) },
        { "piiKind.Uri",                EventProperty("http://www.microsoft.com", PiiKind_Uri) },
        { "piiKind.Fqdn",               EventProperty("www.microsoft.com", PiiKind_Fqdn) },

        { "strKey",   "hello" },
        { "strKey2",  "hello2" },
        { "int64Key", 1L },
        { "dblKey",   3.14 },
        { "boolKey",  false },
        { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
        { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
        { "guidKey2", GUID_t(guid_b) },
        { "guidKey3", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
        { "guidKey4", GUID_t(guid_c) },
        { "timeKey1",  time_ticks_t((uint64_t)0) },     // ticks   precision
        { "timeKey2",  time_ticks_t(&t) }               // seconds precision
    });

    props.SetProperty("win_guid", GUID_t(win_guid));
    props.SetPriority(prio);

    return props;
}

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
}

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
    configuration[CFG_INT_TRACE_LEVEL_MIN]  = ACTTraceLevel_Trace;
    configuration[CFG_STR_COLLECTOR_URL]    = "https://127.0.0.1/";
    ILogger *result = LogManager::Initialize(TEST_TOKEN, configuration);
    EXPECT_EQ(true, (result != NULL));
    LogManager::FlushAndTeardown();
}

TEST(APITest, LogManager_Initialize_DebugEventListener)
{
    constexpr static unsigned MAX_ITERATIONS = 100;

    TestDebugEventListener debugListener;

    auto &configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128; // API calls + Global mask for general messages - less SQL
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Info;
    configuration[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
    addAllListeners(debugListener);
    ILogger *result = LogManager::Initialize(TEST_TOKEN, configuration);

    // Log some foo
    size_t numIterations = MAX_ITERATIONS;
    while (numIterations--)
        result->LogEvent("foo1");
    // Check the counts
    EXPECT_EQ(MAX_ITERATIONS,       debugListener.numLogged);
    EXPECT_EQ(0,                    debugListener.numDropped);
    EXPECT_EQ(0,                    debugListener.numReject);

    LogManager::UploadNow();                                    // Try to upload whatever we got
    PAL::sleep(1000);                                           // Give enough time to upload at least one event
    EXPECT_NE(0,                    debugListener.numSent);     // Some posts have successed within 500ms
    LogManager::PauseTransmission();
    numIterations = MAX_ITERATIONS;
    while (numIterations--)
        result->LogEvent("bar2");                               // New events go straight to offline storage
    EXPECT_EQ(2 * MAX_ITERATIONS,   debugListener.numLogged);

    LogManager::Flush();
    EXPECT_LE(MAX_ITERATIONS,   debugListener.numCached);       // Verify we saved at least the number of 'bar2' + possibly stats
    LogManager::SetTransmitProfile(TransmitProfile_RealTime);
    LogManager::ResumeTransmission();
    PAL::sleep(3000);                                           // Wait long enough for transmit timer to kick-in
    EXPECT_GE(debugListener.numSent, debugListener.numLogged);  // Check that we sent whatever we logged + stats
    LogManager::FlushAndTeardown();
    debugListener.printStats();
    removeAllListeners(debugListener);
}

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

    EXPECT_EQ(false, failed);
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
        EventProperties props = CreateSampleEvent("event_name", EventPriority_Normal);
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

    size_t numIterations     = 10;
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
        config[CFG_INT_TRACE_LEVEL_MIN]  = ACTTraceLevel_Debug;
        config[CFG_STR_COLLECTOR_URL]    = COLLECTOR_URL_PROD;
        config[CFG_INT_SDK_MODE]         = SdkModeTypes::SdkModeTypes_Aria;
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

    EXPECT_GE(shutdownSec+1, total_time);
}

TEST(APITest, LogManager_BadStoragePath_Test)
{
    auto &config = LogManager::GetLogConfiguration();
    config[CFG_INT_TRACE_LEVEL_MASK]  = 0xFFFFFFFF; // API calls + Global mask for general messages - less SQL
    config[CFG_INT_TRACE_LEVEL_MIN]   = ACTTraceLevel_Debug;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 16;

    std::vector<std::string> paths =
    {
        "invalid-path",                          // This would pass
        "T:\\invalid\\file\\path",               // This fails - no offline storage
        u8"C:\\неправильный\\каталог\\utf-8",    // This fails - no offline storage
        u8"C:\\Проверка-проверка 1 2 3\\файл.db" // This should pass if dir exists
    };

    for (const auto &path : paths)
    {
        config[CFG_STR_CACHE_FILE_PATH] = path.c_str();
        ILogger *result = LogManager::Initialize(TEST_TOKEN, config);
        EXPECT_EQ(true, (result != NULL));
        result->LogEvent("test");
        LogManager::Flush();
        LogManager::FlushAndTeardown();
    }

}

TEST(APITest, LogManager_BadNetwork_Test)
{
    MAT_v1::LogConfiguration config;
    config.eventCollectorUri = "http://1.2.3.4"; // "http://127.0.0.1";
    config.traceLevelMask = 0;
    config.minimumTraceLevel = ACTTraceLevel_Warn;
    config.maxTeardownUploadTimeInSec = 2; // Wait for 2 seconds

    ILogConfiguration newConfig = FromLogConfiguration(config);
    size_t numIterations = 10;
    while (numIterations--)
        EXPECT_GE(StressSingleThreaded(newConfig), MAX_ITERATIONS);
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
        threads.push_back(std::thread([logger1,logger2,logger3](){
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

#if 0
// FIXME: [MG] - enable tracing API

bool TracingAPI_File(const char *filename, ACTTraceLevel verbosity = ACTTraceLevel_Trace, size_t traceFileSize = 30000000, size_t evtCount = 1)
{
    bool result = false;
    // Remove old logging file
    std::remove(filename);
    LogConfiguration config;
    // Log something
    config.traceLevelMask = 0xFFFFFFFF;
    config.minimumTraceLevel = verbosity;
    config.SetProperty(CFG_INT_DBG_TRACE_PROVIDER, "1");

    // Property const char* must remain constant for the duration of the run.
    // Most common pitfall is to pass a const char * to temporary that goes
    // out of scope ... what happens next is that the parser would attempt
    // to parse an invalid memory block!
    std::string size = std::to_string(traceFileSize);
    config.SetProperty(CFG_INT_DBG_TRACE_SIZE, size.c_str() );

    if (strlen(filename))
        config.SetProperty(CFG_STR_DBG_TRACE_PATH, filename);
    LogManager::Initialize(TEST_TOKEN, config);
    while (evtCount--)
        LogManager::GetLogger()->LogEvent("TracingAPI");
    LogManager::FlushAndTeardown();
    // Check if default log file has been created
    if (strlen(filename))
        result = common::FileExists(filename);
    else
        result = common::FileExists(PAL::GetDefaultTracePath().c_str());
    return result;
}

TEST(APITest, TracingAPI_DefaultFile)
{
    EXPECT_EQ(TracingAPI_File(""), true);
}

TEST(APITest, TracingAPI_CustomFile)
{
    EXPECT_EQ(TracingAPI_File("mydebug.log"), true);
}

/// <summary>
/// Usage example for EVT_TRACE
/// </summary>
class MyTraceListener : public DebugEventListener {
    std::atomic<unsigned> evtCount;

public:
    MyTraceListener() :
        DebugEventListener(),
        evtCount(0) {};

    // Inherited via DebugEventListener
    virtual void OnDebugEvent(DebugEvent & evt) override
    {
        evtCount++;
        printf("[%d] %s:%d\n", evt.param1, (const char *)evt.data, evt.param2);
    }

    virtual unsigned getEvtCount()
    {
        return evtCount.load();
    }
};

MyTraceListener traceListener;

TEST(APITest, TracingAPI_Callback)
{
    LogConfiguration config;
    config.traceLevelMask = 0xFFFFFFFF;
    config.minimumTraceLevel = ACTTraceLevel_Trace;
    config.SetProperty(CFG_INT_DBG_TRACE_PROVIDER, "1");
    config.SetProperty(CFG_INT_DBG_TRACE_SIZE, "30000000");

    // This event listener is goign to print an error (filename:linenumber) whenever
    // SDK internally experiences any error, warning or fatal event
    LogManager::AddEventListener(EVT_TRACE, traceListener);
    LogManager::Initialize("invalid-token", config);    // We don't check tokens for validity at init ...
    LogManager::GetLogger()->LogEvent("going-nowhere"); // ... but we do check event names.
    LogManager::FlushAndTeardown();
    LogManager::RemoveEventListener(EVT_TRACE, traceListener);

    // Expecting at least one error notification.
    // Invalid event name may trigger several errors at various stages of the pipeline.
    EXPECT_GE(traceListener.getEvtCount(), 1);
}

TEST(APITest, TracingAPI_Verbosity)
{
    // Nothing should be logged - filesize must remain zero
    TracingAPI_File("", ACTTraceLevel_Fatal);
    EXPECT_EQ(common::GetFileSize(PAL::GetDefaultTracePath().c_str()), 0);
    // Default file gets deleted on Release bits on shutdown
}

TEST(APITest, TracingAPI_FileSizeLimit)
{
    // Log file size must remain within 1MB range + 512K contingency buffer
    size_t maxFileSize = 1024000;
    size_t maxContingency = 512000;
    size_t maxEventCount = 50000; // 50K events is sufficient to attempt an overflow on file size
    TracingAPI_File("", ACTTraceLevel_Debug, maxFileSize, maxEventCount);
    EXPECT_LE(common::GetFileSize(PAL::GetDefaultTracePath().c_str()), maxFileSize + maxContingency);
}
#endif

// TEST_PULL_ME_IN(APITest)
