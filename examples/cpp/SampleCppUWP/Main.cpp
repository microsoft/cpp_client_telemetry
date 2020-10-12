#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>

#include "LogManager.hpp"

#include <time.h>

#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <cassert>

/* Identify the build variant */
#ifdef _DEBUG
#define VER1	"UWP-Debug"
#else
#define VER1	"UWP-Release"
#endif
#ifdef _STATIC
#define VER2	"Static"
#else
#define VER2	"Dynamic"
#endif

#include <MainPage.xaml.h>

#include <Main.hpp>

using namespace MAT;
using namespace std;

// Windows SDK Test - Prod: Default Ingestion Token.
#define TOKEN   "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999"

// Windows SDK Test 2 - Int: Default Ingestion Token.
#define TOKEN2  "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9990"

void DebugPrintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int nBuf;
    char szBuffer[512];
    nBuf = _vsnprintf(szBuffer, 511, fmt, args);
    // ::OutputDebugStringA(szBuffer);
    PrintLine(szBuffer);
    va_end(args);
}

#define printf DebugPrintf

// Windows SDK Test - Prod: Default Ingestion Token.
const string cTenantToken = TOKEN;

std::mutex dbg_callback_mtx;

const char* networkCostNames[] = {
    "Unknown",
    "Unmetered",
    "Metered",
    "Roaming",
};

static const constexpr size_t MAX_LATENCY_SAMPLES = 10;

std::atomic<unsigned>   latency[MAX_LATENCY_SAMPLES] = { 0 };

std::atomic<unsigned>   eps = 0;
std::atomic<unsigned>   numLogged0 = 0;
std::atomic<unsigned>   numLogged = 0;
std::atomic<unsigned>   numSent = 0;
std::atomic<unsigned>   numDropped = 0;
std::atomic<unsigned>   numReject = 0;
std::atomic<unsigned>   numCached = 0;
std::atomic<unsigned>   logLatMin = 100;
std::atomic<unsigned>   logLatMax = 0;
unsigned long           testStartMs;

class MyDebugEventListener : public DebugEventListener {
public:
    virtual void OnDebugEvent(DebugEvent &evt)
    {
        // lock for the duration of the print, so that we don't mess up the prints
        std::lock_guard<std::mutex> lock(dbg_callback_mtx);
        unsigned long ms;

        switch (evt.type) {
        case EVT_LOG_EVENT:
            // Track LogEvent latency here
            if (evt.param1 < logLatMin)
                logLatMin = evt.param1;
            if (evt.param1 > logLatMax)
                logLatMax = evt.param1;
        case EVT_LOG_LIFECYCLE:
        case EVT_LOG_FAILURE:
        case EVT_LOG_PAGEVIEW:
        case EVT_LOG_PAGEACTION:
        case EVT_LOG_SAMPLEMETR:
        case EVT_LOG_AGGRMETR:
        case EVT_LOG_TRACE:
        case EVT_LOG_USERSTATE:
        case EVT_LOG_SESSION:
            // printf("OnEventLogged:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            numLogged++;
            ms = static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
            {
                eps = (1000 * numLogged) / (ms - testStartMs);
                if ((numLogged % 500) == 0)
                {
                    printf("EPS=%u\n", eps.load() );
                }
            }
            break;
        case EVT_REJECTED:
            numReject++;
            printf("OnEventRejected:    seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_ADDED:
            printf("OnEventAdded:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_CACHED:
            numCached += evt.param1;
            // printf("OnEventCached:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_DROPPED:
            numDropped += evt.param1;
            printf("OnEventDropped:     seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_SENT:
            numSent += evt.param1;
            // printf("OnEventsSent:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_STORAGE_FULL:
            printf("OnStorageFull:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            if (evt.param1 >= 75) {
                // UploadNow must NEVER EVER be called from Telemetry callback thread, so either use this structure below
                // or notify the main app that it has to do the profile timers housekeeping / force the upload...
                std::thread([]() { LogManager::UploadNow(); }).detach();
            }
            break;

        case EVT_CONN_FAILURE:
        case EVT_HTTP_FAILURE:
        case EVT_COMPRESS_FAILED:
        case EVT_UNKNOWN_HOST:
        case EVT_SEND_FAILED:
            printf("OnEventsSendFailed: seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_HTTP_ERROR:
            printf("OnHttpError:        seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
                evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
            break;
        case EVT_HTTP_OK:
            printf("OnHttpOK:           seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
                evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
            break;
        case EVT_SEND_RETRY:
            printf("OnSendRetry:        seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
                evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
            break;
        case EVT_SEND_RETRY_DROPPED:
            printf("OnSendRetryDropped: seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u, data=%p, size=%d\n",
                evt.seq, evt.ts, evt.type, evt.param1, evt.param2, evt.data, evt.size);
            break;
        case EVT_NET_CHANGED:
            printf("OnNetChanged:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u [%s]\n",
                evt.seq, evt.ts, evt.type, evt.param1, evt.param2, networkCostNames[evt.param1]);
            if (evt.param2)
            {
                printf("Malwarebytes Antiexploit has been detected! Network cost is unknown.\n");
            }
            break;
        case EVT_UNKNOWN:
        default:
            printf("OnEventUnknown:     seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        };

    };
};

MyDebugEventListener listener;

#define MAX_REPEAT_RUNS             5
#define MAX_STRESS_COUNT            2
#define MAX_STRESS_THREADS          5
#define MAX_REINIT_COUNT            3

/// <summary>
/// New fluent syntax
/// </summary>
/// <param name="name"></param>
/// <param name="prio"></param>
/// <returns></returns>

// stress-test for a large string
#define MAX_WEIRDOS     4096
char weirdoBuffer[MAX_WEIRDOS + 1] = { 0 };
;
EventProperties CreateSampleEvent(const char *name, EventPriority prio) {
    GUID_t guid_0 = { 0x4d36e96e, 0xe325, 0x11c3,{ 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };

    GUID win_guid;
    win_guid.Data1 = 0;
    win_guid.Data2 = 1;
    win_guid.Data3 = 2;
    for (size_t i = 0; i < 8; i++)
    {
        win_guid.Data4[i] = static_cast<unsigned char>(i);
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

        { "piiKind.None",               EventProperty("jackfrost",  PiiKind_None) },
        { "piiKind.DistinguishedName",  EventProperty("/CN=Jack Frost,OU=PIE,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
        { "piiKind.GenericData",        EventProperty("jackfrost",  PiiKind_GenericData) },
        { "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
        { "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
        { "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
        { "piiKind.PhoneNumber",        EventProperty("+1-613-866-6960", PiiKind_PhoneNumber) },
        { "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
        { "piiKind.SipAddress",         EventProperty("sip:jackfrost@microsoft.com", PiiKind_SipAddress) },
        { "piiKind.SmtpAddress",        EventProperty("Jack Frost <jackfrost@microsoft.com>", PiiKind_SmtpAddress) },
        { "piiKind.Identity",           EventProperty("Jack Frost", PiiKind_Identity) },
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
    props.SetProperty("Flow_IntKey", 1);

    uint64_t bigValue = 0xFFFFFFFFFFFFFFFF;
    props.SetProperty("bigValue", std::to_string(bigValue));

    // GUID_t guidKey5("00000000-0000-0000-0000-000000000001");
    // GUID_t &g = guidKey5;
    // props.SetProperty("refGuidKey5", guidKey5);

    props.SetPriority(prio);

#if 0
    /* This may cause out of memory in a stress... */
    // This buffer is intentionally concurrently modified from different threads,
    // so it's random string of ASCII characeters pretty much, depending on how
    // many threads running and at what speed
    size_t i = 0;
    for (i = 0; i < 1 + (rand() % MAX_WEIRDOS); i++)
    {
        weirdoBuffer[i] = ' ' + (i % (127 - ' '));
    }
    weirdoBuffer[i - 1] = 0;
    props.SetProperty("weirdoString", (const char *)(&weirdoBuffer[0]));
#endif

    return props;
}

void test_ProfileSwitch(ILogger *logger)
{
    printf("switching profile to Office_Telemetry_OneMinute\n");
    LogManager::SetTransmitProfile("Office_Telemetry_OneMinute");
    for (int i = 0; i < 10; i++)
    {
        std::string eventName = "eventName_5min_";
        eventName += std::to_string(i);
        logger->LogEvent(eventName);
    }
#if 0
    std::cout << "Press <ENTER> to switch to another profile..." << std::endl;
    fflush(stdout);
    fgetc(stdin);
#endif

    printf("switching profile to Office_Telemetry_TenSeconds\n");
    LogManager::SetTransmitProfile("Office_Telemetry_TenSeconds");
    for (int i = 0; i < 10; i++)
    {
        std::string eventName = "eventName_10sec_";
        eventName += std::to_string(i);
        logger->LogEvent(eventName);
    }
#if 0
    std::cout << "Press <ENTER> to continue..." << std::endl;
    fflush(stdout);
    fgetc(stdin);
#endif

}

static auto& configuration = LogManager::GetLogConfiguration();

ILogger* init() {
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 0;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes_UTCCommonSchema;

    // OTEL profile example
    const char* transmitProfileDefinitions = R"(
[{
    "name": "Office_Telemetry_OneSecond",
    "rules": [
    { "netCost": "restricted", "timers": [ -1, -1, -1 ] },
    { "netCost": "high",       "timers": [ -1, -1,  1 ] },
    { "netCost": "low",        "timers": [  1,  1,  1 ] },
    { "netCost": "unknown",    "timers": [  1,  1,  1 ] },
    {                          "timers": [  1,  1,  1 ] }
    ]
},
{
    "name": "Office_Telemetry_TenSeconds",
    "rules": [
    { "netCost": "restricted", "timers": [ -1, -1, -1 ] },
    { "netCost": "high",       "timers": [ -1, -1, 10 ] },
    { "netCost": "low",        "timers": [ 10, 10, 10 ] },
    { "netCost": "unknown",    "timers": [ 10, 10, 10 ] },
    {                          "timers": [ 10, 10, 10 ] }
    ]
},
{
    "name": "Office_Telemetry_300Seconds",
    "rules": [ 
    { "netCost": "restricted", "timers": [  -1,  -1,  -1 ] },
    { "netCost": "high",       "timers": [  -1,  -1,  10 ] },
    { "netCost": "low",        "timers": [ 300, 300, 300 ] },
    { "netCost": "unknown",    "timers": [ 300, 300, 300 ] },
    {                          "timers": [ 300, 300, 300 ] }
    ]
}]
)";

    // LogManager::LoadTransmitProfiles(transmitProfileDefinitions);

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

    printf("LogManager::Initialize...\n");
    ILogger *result = LogManager::Initialize(cTenantToken);

    return result;
}

std::mutex mtx_log_session;

void run(ILogger* logger, int maxStressRuns) {
    {

        for (int stressRuns = 0; stressRuns < maxStressRuns; stressRuns++)
        {
            bool doPause = false;
            bool doResume = false;

            if (doPause) {
                LogManager::PauseTransmission();
            }

            {
                // Set the custom context to be sent with every telemetry event.
                logger->SetContext("TeamName", "PIE");
                logger->SetContext("AppID", VER1 VER2 "-" __DATE__ " " __TIME__);
                logger->SetContext("context.string.key", "boo");

                long long_value = 12345L;
                logger->SetContext("context.long.key", long_value);

                double double_value = (double)((uint64_t)9223372036854775807L);
                logger->SetContext("context.double.key", double_value);

                {
                    EventProperties props = CreateSampleEvent("Sample.Event.Low", EventPriority_Low);
                    logger->LogEvent(props);
                }

                if ((stressRuns % 2) == 0)
                {
                    EventProperties props = CreateSampleEvent("Sample.Event.Normal", EventPriority_Normal);
                    logger->LogEvent(props);
                }

                if ((stressRuns % 4) == 0)
                {
                    EventProperties props = CreateSampleEvent("Sample.Event.High", EventPriority_High);
                    props.SetType("My.Super.Duper.Fancy.Event.Type.For.MDM.Export");
                    logger->LogEvent(props);
                }

                if ((stressRuns % 8) == 0)
                {
                    EventProperties props = CreateSampleEvent("Sample.Event.Immediate", EventPriority_Immediate);
                    logger->LogEvent(props);
                }
            }

            {
                // LogSession API is not thread-safe by design -- use logger passed from above
                std::lock_guard<std::mutex> lock(mtx_log_session);
                EventProperties props("LogSessionTest");
                props.SetPriority(EventPriority_Immediate);
                logger->LogSession(SessionState::Session_Started, props);
                logger->LogSession(SessionState::Session_Ended, props);
            }

            if (doResume) {
                LogManager::ResumeTransmission();
            }

#ifdef DETECT_MEMLEAKS
            // _ASSERTE( _CrtCheckMemory() );   // <-- this one is super-slow
#endif

            // _sleep(5);

        };

    }
}

void test_failure(ILogger *logger) {
    try {
        // This event should be rejected
        logger->LogEvent("!!!EPIC FAIL!!!");
    }
    catch (...) {

    }
}

void done() {
    printf("calling LogManager::FlushAndTeardown()...");
    LogManager::FlushAndTeardown();
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
    printf("[DONE]");
}

class TelemetryEventListener : public DebugEventListener {
public:

};

void resetCounters()
{
    testStartMs = 0;
    eps = 0;
    numLogged0 = 0;
    numLogged = 0;
    numSent = 0;
    numDropped = 0;
    numReject = 0;
    numCached = 0;
    logLatMin = 100;
    logLatMax = 0;
}

int testRun()
{
    resetCounters();

    std::vector<std::thread> workers;
    std::thread t[MAX_STRESS_THREADS];
    resetCounters();
    ILogger* logger = init();
    ILogger* logger2 = LogManager::GetLogger(TOKEN2, "tenant2");

#if 0
    printf("test_ProfileSwitch\n");
    test_ProfileSwitch(logger);
#endif

    // Run multi-threaded stress for multi-tenant
    testStartMs = static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
    for (int i = 0; i < MAX_STRESS_THREADS; i++) {
        workers.push_back(std::thread([i, logger, logger2]()
        {
            std::string threadName = "test_thread_";
            threadName += std::to_string(i);
            run(logger, MAX_STRESS_COUNT);
            run(logger2, MAX_STRESS_COUNT);
        }));
    }

    // Wait for completion of all worker threads
    std::for_each(workers.begin(), workers.end(), [](std::thread &t)
    {
        if (t.joinable())
        {
            t.join();
        }
    });

// all_done:

    unsigned long ms = static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
    unsigned long shutdownTimeMs = 0;

    eps = (1000 * numLogged) / (ms - testStartMs);
    printf("***************** BEFORE TEARDOWN ************\n");
    printf("EPS:        %u\n", eps.load());
    printf("Latency:    [%u..%u]\n", logLatMin.load(), logLatMax.load());
    printf("Logged:     %u\n", numLogged.load());
    printf("Cached:     %u\n", numCached.load());
    printf("Sent:       %u\n", numSent.load());
    printf("Dropped:    %u\n", numDropped.load());
    printf("Rejected:   %u\n", numReject.load());

    shutdownTimeMs = static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
    done();
    auto shutdownTimeMsDelta = (std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1)) - shutdownTimeMs;

    shutdownTimeMs = static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
    auto totalTimeDelta = shutdownTimeMs - testStartMs;
    printf("***************** AFTER TEARDOWN ************\n");
    printf("Stop dur:   %u ms\n", shutdownTimeMsDelta);
    printf("Logged:     %u\n", numLogged.load());
    printf("Cached:     %u\n", numCached.load());
    printf("Sent:       %u\n", numSent.load());
    printf("Dropped:    %u\n", numDropped.load());
    printf("Rejected:   %u\n", numReject.load());
    printf("Total time: %u ms\n", totalTimeDelta);

    return 0;
}

void PerformanceTest()
{
    // Make sure we're not initialized because we'd reinitialize
    LogManager::FlushAndTeardown();

    for (size_t i = 0; i < MAX_REPEAT_RUNS; i++)
    {
        testRun();
    }
}
