#define _CRT_SECURE_NO_WARNINGS

// #define USE_ECG
// #define DETECT_MEMLEAKS

#include <stdlib.h>
#include <stdio.h>

#ifdef DETECT_MEMLEAKS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "LogManager.hpp"

#include <iostream>

#include <time.h>

#include <thread>
#include <mutex>
#include <vector>
#include <ctime>

#ifdef USE_ECG
#include <ecg.hpp>
#pragma comment(lib, "ecg.lib")
#include <chrono>
#include <thread>
#include <json.hpp> // This MIT-licensed JSON support library is optional, but is quite convenient. CounterSink expects the counters in a JSON record.
using json = nlohmann::json;
#endif

extern "C" void __cdecl triggerAriaAbort();

/* Identify the build variant */
#ifdef _DEBUG
#define VER1	"Win32-Debug"
#else
#define VER1	"Win32-Release"
#endif
#ifdef _STATIC
#define VER2	"Static"
#else
#define VER2	"Dynamic"
#endif

using namespace Microsoft::Applications::Telemetry;
using namespace std;

#define USE_INT
//#define USE_BOGUS_URL

#ifdef USE_INT
// Windows SDK Test - Int: Default Ingestion Token.
#define TOKEN   "0c21c15bdccc48c99678a748488bb87f-cca6848e-b4aa-48a6-b24a-0170caf27523-7582"
// Windows SDK Test 2 - Int: Default Ingestion Token.
#define TOKEN2  "d80b0dc91fee49889ac97707644df9e5-8af395b4-7c71-4470-8374-bb4e7e5cd2ab-7066"
#else
// Windows SDK Test - Prod: Default Ingestion Token.
#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"
// Windows SDK Test 2 - Int: Default Ingestion Token.
#define TOKEN2  "xyz"
#endif

// Windows SDK Test - Prod: Default Ingestion Token.
const string cTenantToken = TOKEN;

std::mutex dbg_callback_mtx;

#ifdef USE_ECG
CounterSink counterSink("127.0.0.1", 8888, CounterSinkProto::UDP);
#endif

const char* networkCostNames[] = {
    "Unknown",
    "Unmetered",
    "Metered",
    "Roaming",
};

class MyDebugEventListener : public DebugEventListener {
public:
    virtual void OnDebugEvent(DebugEvent &evt)
    {
#ifdef USE_ECG
        int64_t cyclesPerTime = 0;
        {
            std::lock_guard<std::mutex> lock(dbg_callback_mtx);
            static int64_t prevCycles = 0;
            static int64_t currCycles = 0;
            static auto prevTime = std::chrono::steady_clock::now();
            static auto currTime = std::chrono::steady_clock::now();
            // Obtain current time
            currTime = std::chrono::steady_clock::now();
            currCycles = GetCPUCycles();
            // Calculate diff in milliseconds
            auto diffTime = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - prevTime).count();
            prevTime = currTime;
            if ((currCycles > prevCycles) && (((int64_t)diffTime) > 0)) {
                cyclesPerTime = (currCycles - prevCycles) / ((int64_t)diffTime);
                // Calculate CPU cycles per time
                auto currCycles = GetCPUCycles();
            }
            else {
                prevCycles = currCycles;
                return;
            }
            prevCycles = currCycles;
        }

        json j = {
            { "evtSeq",      evt.seq },
            { "memUse",      GetMemoryUsage() },
            { "cpuCycles",   cyclesPerTime },
            { "numThreads",  GetCurrentThreadCount() },
            { "tcpCount",    GetCurrentTCPCount() },
            { "udpCount",    GetCurrentUDPCount() },
        };
        switch (evt.type) {
        case EVT_SENT:
            j.push_back({ "evtSent", evt.param1});
            break;
        case EVT_HTTP_OK:
            j.push_back({ "httpOK", evt.size });
            break;
        case EVT_HTTP_ERROR:
            j.push_back({ "httpERR", evt.size });
            break;
        }
        counterSink.log(j.dump());

#else
        // lock for the duration of the print, so that we don't mess up the prints
        std::lock_guard<std::mutex> lock(dbg_callback_mtx);
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
            printf("OnEventLogged:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_REJECTED:
            printf("OnEventRejected:    seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_ADDED:
            printf("OnEventAdded:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_CACHED:
            printf("OnEventCached:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_DROPPED:
            printf("OnEventDropped:     seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_SENT:
            printf("OnEventsSent:       seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            break;
        case EVT_STORAGE_FULL:
            printf("OnStorageFull:      seq=%llu, ts=%llu, type=0x%08x, p1=%u, p2=%u\n", evt.seq, evt.ts, evt.type, evt.param1, evt.param2);
            if (evt.param1 >= 75) {
                // UploadNow must NEVER EVER be called from SDK callback thread, so either use this structure below
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
#endif
    };
};

MyDebugEventListener listener;

#define MAX_STRESS_COUNT            1000
#define MAX_STRESS_THREADS          10

/// <summary>
/// New fluent syntax
/// </summary>
/// <param name="name"></param>
/// <param name="prio"></param>
/// <returns></returns>

// stress-test for a large string
#define MAX_WEIRDOS     2000
char weirdoBuffer[MAX_WEIRDOS] = { 'A' };

EventProperties CreateSampleEvent(const char *name, EventPriority prio) {

    GUID win_guid;
    win_guid.Data1 = 0;
    win_guid.Data2 = 1;
    win_guid.Data3 = 2;
    for (size_t i = 0; i < 8; i++)
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

#if defined(_MSC_VER) && (_MSC_VER == 1800)
    /* map assignment operator for Visual Studio 2013, which may not fully support C++11 features. */
    std::map<std::string, EventProperty> values;
    values["_MSC_VER"]  = _MSC_VER;

    values["piiKind.None"]              = EventProperty("jackfrost",  PiiKind_None);
    values["piiKind.DistinguishedName"] = EventProperty("/CN=Jack Frost,OU=PIE,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName);
    values["piiKind.GenericData"]       = EventProperty("jackfrost",  PiiKind_GenericData);
    values["piiKind.IPv4Address"]       = EventProperty("127.0.0.1", PiiKind_IPv4Address);
    values["piiKind.IPv6Address"]       = EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address);
    values["piiKind.MailSubject"]       = EventProperty("RE: test",  PiiKind_MailSubject);
    values["piiKind.PhoneNumber"]       = EventProperty("+1-613-866-6960", PiiKind_PhoneNumber);
    values["piiKind.QueryString"]       = EventProperty("a=1&b=2&c=3", PiiKind_QueryString);
    values["piiKind.SipAddress"]        = EventProperty("sip:jackfrost@microsoft.com", PiiKind_SipAddress);
    values["piiKind.SmtpAddress"]       = EventProperty("Jack Frost <jackfrost@microsoft.com>", PiiKind_SmtpAddress);
    values["piiKind.Identity"]          = EventProperty("Jack Frost", PiiKind_Identity);
    values["piiKind.Uri"]               = EventProperty("http://www.microsoft.com", PiiKind_Uri);
    values["piiKind.Fqdn"]              = EventProperty("www.microsoft.com", PiiKind_Fqdn);

    values["strKey"]    = "hello";
    values["strKey2"]   = "hello2";
    values["int64Key"]  = 1L;
    values["dblKey"]    = 3.14;
    values["boolKey"]   = false;
    values["guidKey0"]  = GUID_t("00000000-0000-0000-0000-000000000000");
    values["guidKey1"]  = GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F");
    values["guidKey2"]  = GUID_t(guid_b);
    values["guidKey3"]  = GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F");
    values["guidKey4"]  = GUID_t(guid_c);
    values["timeKey1"]  = time_ticks_t((uint64_t)0);
    values["timeKey2"]  = time_ticks_t(&t);
    EventProperties props(name, values);
#else
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

        { "strKey",   "hello"  },
        { "strKey2",  "hello2" },
        { "int64Key", 1L       },
        { "dblKey",   3.14     },
        { "boolKey",  false    },
        { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
        { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
        { "guidKey2", GUID_t(guid_b) },
        { "guidKey3", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
        { "guidKey4", GUID_t(guid_c) },
        { "timeKey1",  time_ticks_t((uint64_t)0) },     // ticks   precision
        { "timeKey2",  time_ticks_t(&t) }               // seconds precision
    });
#endif
    props.SetProperty("win_guid", GUID_t(win_guid));

    // GUID_t guidKey5("00000000-0000-0000-0000-000000000001");
    // GUID_t &g = guidKey5;
    // props.SetProperty("refGuidKey5", guidKey5);

    props.SetPriority(prio);

#if 1 /* This may cause out of memory in a stress... */
    // This buffer is intentionally concurrently modified from different threads,
    // so it's random string of ASCII characeters pretty much, depending on how
    // many threads running and at what speed
    for (size_t i = 0; i < MAX_WEIRDOS; i++)
    {
        weirdoBuffer[i]  = ' ' + (i % (127 - ' '));
    }
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

void sendEmptyEvent(ILogger *logger)
{
    EventProperties props("test_empty_event");
    logger->LogEvent(props);
}

LogConfiguration configuration;

ILogger* init() {
    configuration.cacheFilePath = "offlinestorage.db";
    configuration.traceLevelMask = 0xFFFFFFFF ^ 128; // API calls + Global mask for general messages - less SQL
//  configuration.minimumTraceLevel = ACTTraceLevel_Debug;
    configuration.minimumTraceLevel = ACTTraceLevel_Trace;
    configuration.multiTenantEnabled = true;
    configuration.cacheFileSizeLimitInBytes = 100000000;
    configuration.maxTeardownUploadTimeInSec = 5;

	// Force UTC uploader on Windows 10 even if it's not RS2
    // configuration.sdkmode = (SdkModeTypes)(-SdkModeTypes::SdkModeTypes_UTCBackCompat);

#ifdef USE_INT
    configuration.eventCollectorUri = "https://pipe.int.trafficmanager.net/Collector/3.0/";
#endif

#ifdef USE_BOGUS_URL
    configuration.eventCollectorUri = "https://127.0.0.1/";
#endif

#if 0
    std::string leastCostProfile = R"(
[{
    "name": "LEAST_COST",
    "rules": [
    { "netCost": "restricted",                              "timers": [ -1, -1, -1 ] },
    { "netCost": "high",        "powerState": "battery",    "timers": [ -1, -1, -1 ] },
    { "netCost": "high",        "powerState": "charging",   "timers": [ 35, 17,  5 ] },
    { "netCost": "low",         "powerState": "battery",    "timers": [ 32, 16,  8 ] },
    { "netCost": "low",         "powerState": "charging",   "timers": [ 16,  8,  4 ] },
    { "netCost": "unknown",     "powerState": "battery",    "timers": [128, 64, 32 ] },
    { "netCost": "unknown",     "powerState": "charging",   "timers": [ 64, 32, 16 ] },
    {                                                       "timers": [100, 50, 17 ] }
    ]
}]
)";
#endif

    // OTEL profile example
    const char* transmitProfileDefinitions = R"(
[{
    "name": "Office_Telemetry_OneSecond",
    "rules": [
    { "netCost": "restricted", "timers": [ -1, -1, -1 ] },
    { "netCost": "high",       "timers": [ -1, -1, 1 ] },
    { "netCost": "low",        "timers": [ 1, 1, 1 ] },
    { "netCost": "unknown",    "timers": [ 1, 1, 1 ] },
    {                          "timers": [ 1, 1, 1 ] }
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
    "name": "Office_Telemetry_OneMinute",
    "rules": [
    { "netCost": "restricted", "timers": [ -1, -1, -1 ] },
    { "netCost": "high",       "timers": [ -1, -1, 60 ] },
    { "netCost": "low",        "timers": [ 60, 60, 60 ] },
    { "netCost": "unknown",    "timers": [ 60, 60, 60 ] },
    {                          "timers": [ 60, 60, 60 ] }
    ]
}]
)";

    LogManager::LoadTransmitProfiles(transmitProfileDefinitions);

    LogManager::AddEventListener(DebugEventType::EVT_LOG_SESSION,         listener);
    LogManager::AddEventListener(DebugEventType::EVT_REJECTED,            listener);
    LogManager::AddEventListener(DebugEventType::EVT_SEND_FAILED,         listener);
    LogManager::AddEventListener(DebugEventType::EVT_SENT,                listener);
    LogManager::AddEventListener(DebugEventType::EVT_DROPPED,             listener);
    LogManager::AddEventListener(DebugEventType::EVT_HTTP_OK,             listener);
    LogManager::AddEventListener(DebugEventType::EVT_HTTP_ERROR,          listener);
    LogManager::AddEventListener(DebugEventType::EVT_SEND_RETRY,          listener);
    LogManager::AddEventListener(DebugEventType::EVT_SEND_RETRY_DROPPED,  listener);
    LogManager::AddEventListener(DebugEventType::EVT_CACHED,              listener);
    LogManager::AddEventListener(DebugEventType::EVT_NET_CHANGED,         listener);
    LogManager::AddEventListener(DebugEventType::EVT_STORAGE_FULL,        listener);

    std::cout << "LogManager::Initialize..." << endl;

    // Apply the profile before initialize
    LogManager::SetTransmitProfile("Office_Telemetry_TenSeconds");
    ILogger *result = LogManager::Initialize(cTenantToken, configuration);

    // TC for SetContext(<const char*,const char*, PiiKind>)
    const char* gc_value = "1234 :-)";
    LogManager::SetContext("GLOBAL_context", gc_value, PiiKind_MailSubject);

    return result;
}

std::mutex mtx_log_session;

void run(ILogger* logger, int maxStressRuns) {
    {

        for (int stressRuns = 0; stressRuns < maxStressRuns; stressRuns++)
        {
            bool doPause  = false;
            bool doResume = false;

            if (doPause) {
                LogManager::PauseTransmission();
            }

            {
                // ignore the logger passed from above
                ILogger *logger = LogManager::GetLogger();

                // Set the custom context to be sent with every telemetry event.
                logger->SetContext("TeamName", "PIE");
                logger->SetContext("AppID", VER1 VER2 "-" __DATE__ " " __TIME__);
                // Set the semantic context. For example, an app will set this property after the user logs in.
                logger->GetSemanticContext()->SetUserMsaId("BCCA864D-1386-4D5A-9570-B129F6DD42B7");
                logger->SetContext("context.string.key", "boo");

                long long_value = 12345L;
                logger->SetContext("context.long.key", long_value);

                double double_value = (double)((uint64_t)9223372036854775807L);
                logger->SetContext("context.double.key", double_value);

                {
                    EventProperties props = CreateSampleEvent("Sample.Event.Low", EventPriority_Low);
                    logger->LogEvent(props);
                }

                {
                    EventProperties props = CreateSampleEvent("Sample.Event.Normal", EventPriority_Normal);
                    logger->LogEvent(props);
                }

                {
                    EventProperties props = CreateSampleEvent("Sample.Event.High", EventPriority_High);
                    props.SetType("My.Super.Duper.Fancy.Event.Type.For.MDM.Export");
                    logger->LogEvent(props);
                }

                {
                    EventProperties props = CreateSampleEvent("Sample.Event.Immediate", EventPriority_Immediate);
                    logger->LogEvent(props);
                }
            }

            {
                // Use logger passed from above
                EventProperties props = CreateSampleEvent("Sample2.Event.High", EventPriority_High);
                logger->LogEvent(props);
            }

            // Send empty EventProperties
            sendEmptyEvent(logger);

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

#ifdef _RANDOM_DELAY_AFTER_LOG
            // Let event at least try to reach the server
            srand(time(NULL));
            unsigned sleepTime = 1000 * (rand() % 7);
            std::cout << "sleep " << sleepTime << std::endl;
            _sleep(sleepTime);
#endif

        }
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
    std::cout << "LogManager::FlushAndTeardown()..." << std::endl;
    LogManager::FlushAndTeardown();
}

int main(int argc, char* argv[])
{
    std::vector<std::thread> workers;
    std::thread t[MAX_STRESS_THREADS];

    ILogger* logger  = init();
    ILogger* logger2 = LogManager::GetLogger(TOKEN2, "tenant2");

#if 0
    printf("test_ProfileSwitch\n");
    test_ProfileSwitch(logger);
#endif

    // Run multi-threaded stress for multi-tenant
    for (int i = 0; i < MAX_STRESS_THREADS; i++) {
        workers.push_back(std::thread([logger, logger2]()
        {
            run(logger, MAX_STRESS_COUNT);
            run(logger2, MAX_STRESS_COUNT);
        }));
    }

    bool doRandomAbort = false;
    if (doRandomAbort)
    {
        // Trigger SDK abort at random within the first 40 seconds
        workers.push_back(std::thread([]() {
            srand(time(NULL));
            int sleepTime = rand() % 20;
            printf("Emulating random abort in %d seconds...\n", sleepTime);
            std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
            ::triggerAriaAbort();
        }));
    };

    // Wait for completion of all worker threads
    std::for_each(workers.begin(), workers.end(), [](std::thread &t)
    {
        if (t.joinable())
        {
            t.join();
        }
    });

    // save to disk
    LogManager::Flush();

all_done:

    bool waitForUser = true;
    if (waitForUser) {
        std::cout << "Press <ENTER> to FlushAndTeardown" << std::endl;
        fflush(stdout);
        fgetc(stdin);
    }

    // Flush and Teardown
    done();

    // 2nd run after initialize
    {
        // Reinitialize test
        logger = LogManager::Initialize(TOKEN);
        logger->LogEvent("BooEvent1");
        logger2->LogEvent("BooEvent2");
        int sleepTime = 1000 * (rand() % 3);
        printf("sleep %d\n", sleepTime);
        fflush(stdout);
        // _sleep(sleepTime);
        // Flush and Teardown
        done();
    }

#ifdef DETECT_MEMLEAKS
    _CrtDumpMemoryLeaks();
#endif

    return 0;
}
