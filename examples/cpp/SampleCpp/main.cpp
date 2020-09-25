// #define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
//#include <stdlib.h>  
//#include <crtdbg.h>

#include <memory>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <future>
#include <cassert>

#include "LogManager.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

#include "DebugCallback.hpp"
 
LOGMANAGER_INSTANCE

#include "DefaultApiKey.h"

extern "C" void guestTest();    // see guest.cpp 

/**
* All v1.x legacy API compile-time checks in alphabetic order
*/
void Api_v1_CompatChecks()
{
    ILogger *logger = LogManager::GetLogger();
    auto context = logger->GetSemanticContext();
    context->SetAppVersion("1.2.3");
    logger->SetContext("CommonContextVar", 12345); 

    {
        EventProperties props("AggregatedMetric1");
        AggregatedMetricData amd("AggregatedMetric", 0, 0);
        logger->LogAggregatedMetric(amd, props);
    }

    {
        EventProperties props("AppLifecycle1");
        AppLifecycleState state = AppLifecycleState_Unknown;
        logger->LogAppLifecycle(state, props);
    }

    {
        EventProperties props("LogEvent1");
        logger->LogEvent(props);
    }

    {
        EventProperties props("LogFailure1");
        logger->LogFailure("signature", "detail", props);
    }

    {
        EventProperties props("PageAction1");
        PageActionData pad("PageAction", ActionType_Unknown);
        logger->LogPageAction(pad, props);
    }

    {
        EventProperties props("PageView1");
        logger->LogPageView("id", "pageName", props);
    }

    {
        EventProperties props("SampledMetric1");
        logger->LogSampledMetric("name", 0.1, "units", props);
    }

    {
        EventProperties props("LogSession1");
        SessionState sess(Session_Started);
        logger->LogSession(sess, props);
    }

    {
        EventProperties props("LogTrace1");
        logger->LogTrace(TraceLevel_Error, "message", props);
    }

    {
        EventProperties props("LogUserState1");
        logger->LogUserState(UserState_Unknown, 1L, props);
    }

}

#define	 ENTER	printf("%s\n", __FUNCTION__)

typedef std::chrono::milliseconds ms;

/// <summary>
/// Delay execution for specified number of milliseconds. Generally for testing code only.
/// </summary>
/// <param name="delayMs">The delay ms.</param>
inline void sleep(unsigned delayMs)
{
    std::this_thread::sleep_for(ms(delayMs));
}

#pragma warning(suppress:4447) // 'main' signature found without threading model. Consider using 'int main(Platform::Array<Platform::String^>^ args)'.

#include "LogManager.hpp"

using namespace MAT;

MyDebugEventListener listener;

ILogConfiguration testConfiguration()
{
    std::string someString = "12345";

    ILogConfiguration result = {
        { "a", 0 },
        { "b", 0.1 },
        { "c",
            {
                { "demo",  "string"  },
                { "demo2", someString },
            }
        }
    };
    return result;
}

#define MAX_EVENTS_TO_LOG       1000L 

extern "C" int OfficeTest();
extern "C" void test_c_api();

void logPiiEvent()
{
    auto logger = LogManager::GetLogger();

    // Log detailed event with various properties
    EventProperties detailed_event("detailed_event",
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
    logger->LogEvent(detailed_event);
}

void logPiiMark()
{
    auto logger = LogManager::GetLogger();

    // Log event with Pii properties with no Pii mark
    EventProperties event1("MyEvent.Pii",
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
    logger->LogEvent(event1);

    // Log event with Pii properties AND Pii mark flag at event level
    event1.SetName("MyEvent.Pii.PiiMarked");
    event1.SetPolicyBitFlags(MICROSOFT_EVENTTAG_MARK_PII);
    logger->LogEvent(event1);
}

void logDoNotStore()
{
    auto logger = LogManager::GetLogger();
    EventProperties eventInRam("MyEvent.NeverStore");
    eventInRam.SetPersistence(EventPersistence::EventPersistence_DoNotStoreOnDisk);
    logger->LogEvent(eventInRam); // this event should not go to disk
}

int main()
{
#ifdef OFFICE_TEST  /* Custom test for a stats crash scenario experienced by OTEL */
    OfficeTest();
    if (1)
        return 0;
#endif

#if 0
    // Guest SDKs start first
    guestTest();
#endif

    // Host SDK starts
    printf("Setting up configuration...\n");
    auto& config = LogManager::GetLogConfiguration();

    config["name"] = "SampleCpp";
    config["version"] = "1.2.5";
    config["config"]["host"] = "SampleCpp"; // host
    config["compat"]["dotType"] = false;    // Legacy v1 behaviour with respect to SetType using underscore instead of a dot

#ifdef __APPLE__
    config[CFG_STR_CACHE_FILE_PATH]   = "/tmp/offlinestorage.db";
#else
    config[CFG_STR_CACHE_FILE_PATH]   = "offlinestorage.db";
#endif

    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;
    config[CFG_INT_TRACE_LEVEL_MASK]  = 0;  // 0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN]   = ACTTraceLevel_Warn; // ACTTraceLevel_Info; // ACTTraceLevel_Debug;
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS; // SdkModeTypes::SdkModeTypes_UTCCommonSchema
    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;

// #define USE_LOCAL_URL /* Send to local test server */
#ifdef USE_LOCAL_URL
    config[CFG_STR_COLLECTOR_URL]     = "https://127.0.0.1:5001/OneCollector/";
#endif

//#define USE_INVALID_URL
#ifdef USE_INVALID_URL /* Stress-test for the case when collector is unreachable */
    config[CFG_STR_COLLECTOR_URL]     = "https://127.0.0.1/invalid/url";
#endif
    config[CFG_INT_RAM_QUEUE_SIZE]    = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config[CFG_INT_CACHE_FILE_SIZE]   = 16 * 1024 * 1024; // 16 MB storage file limit

    printf("Adding debug event listeners...\n");
    auto eventsList = {
        DebugEventType::EVT_LOG_EVENT,
        DebugEventType::EVT_LOG_SESSION,
        DebugEventType::EVT_REJECTED,
        DebugEventType::EVT_SEND_FAILED,
        DebugEventType::EVT_SENT,
        DebugEventType::EVT_DROPPED,
        DebugEventType::EVT_HTTP_STATE,
        DebugEventType::EVT_HTTP_OK,
        DebugEventType::EVT_HTTP_ERROR,
        DebugEventType::EVT_SEND_RETRY,
        DebugEventType::EVT_SEND_RETRY_DROPPED,
        DebugEventType::EVT_CACHED,
        DebugEventType::EVT_NET_CHANGED,
        DebugEventType::EVT_STORAGE_FULL
    };
    // Add event listeners
    for (auto evt : eventsList)
        LogManager::AddEventListener(evt, listener);

    ILogger *logger = nullptr;

#ifdef _WIN32
    printf("LogManager::Initialize in UTC\n");
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;
    logger = LogManager::Initialize(API_KEY);
    logPiiMark();   // UTC upload
    LogManager::FlushAndTeardown();
#endif

    printf("LogManager::Initialize in direct\n");
    printf("Teardown time: %d\n", int(config[CFG_INT_MAX_TEARDOWN_TIME]) );
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;

#ifdef _WIN32
    // Code snippet showing how to perform MS Root certificate check for v10 end-point.
    // Most other end-points are Baltimore CA-rooted. But v10 is MS CA-rooted.
    config["http"]["msRootCheck"] = true;
    config[CFG_STR_COLLECTOR_URL] = "https://v10.events.data.microsoft.com/OneCollector/1.0/";
#endif

    logger = LogManager::Initialize(API_KEY);

    logPiiMark();   // Direct upload

    // This global context variable will not be seen by C API client
    LogManager::SetContext("GlobalContext.Var", 12345);

    printf("LogManager::GetSemanticContext \n"); 
    ISemanticContext* semanticContext = LogManager::GetSemanticContext();

    semanticContext->SetAppId("MyAppName");     // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetAppVersion("1.0.1");    // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetAppLanguage("en-US");   // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetUserLanguage("en-US");  // caller must obtain the user language from preferences

    logPiiEvent();

    Api_v1_CompatChecks();

    // Run C API client test
    test_c_api();
 
#ifndef _WIN32
    // Platforms other than Windows currently do not have automatic network detection implemented,
    // so the caller must populate these fields using semantic context API
    semanticContext->SetNetworkCost(MAT::NetworkCost::NetworkCost_Unmetered);
    semanticContext->SetNetworkType(MAT::NetworkType::NetworkType_Wired);
#endif

    LogManager::SetTransmitProfile(TransmitProfile::TransmitProfile_NearRealTime);

    logDoNotStore();
    LogManager::UploadNow();

    // Ingest events of various latencies
    printf("Starting stress-test...\n");
    for(size_t i = 1; i <= MAX_EVENTS_TO_LOG; i++)
    {
        EventLatency latency = (i % 2) ? EventLatency_Normal : EventLatency_CostDeferred;
        std::string eventName("Microsoft.Applications.Telemetry.SampleCpp.sample_event_lat");
        eventName += std::to_string((unsigned)latency);

        EventProperties event(eventName);
        std::string evtType = "My.Record.BaseType"; // default v1 legacy behaviour: custom.my_record_basetype
        event.SetName("MyProduct.TaggedEvent");
        event.SetType(evtType);
        event.SetProperty("result", "Success");
        event.SetProperty("random", rand());
        event.SetProperty("secret", 5.6872);
        event.SetProperty("seq", (uint64_t)i); 
        event.SetProperty(COMMONFIELDS_EVENT_PRIVTAGS, static_cast<int64_t>(i + 1));
        event.SetProperty(COMMONFIELDS_EVENT_LEVEL, static_cast<uint8_t>(i + 1));
        event.SetLatency(latency); 
        logger->LogEvent(event);

        EventProperties event2("MyProduct.TaggedEvent2",
            {
                { "result", "Success" },
                { "random", rand() },
                { "secret", 5.6872 },
                { "seq", (uint64_t)i },
                {COMMONFIELDS_EVENT_PRIVTAGS, static_cast<int64_t>(i + 1)},
                {COMMONFIELDS_EVENT_LEVEL, static_cast<uint8_t>(i + 1)}
            });
        logger->LogEvent(event2);
    }

    // Default transmission timers:
    // high		- 2 sec
    // normal	- 4 sec
    // low		- 8 sec

    printf("LogManager::FlushAndTeardown\n");
    LogManager::FlushAndTeardown();

    // Remove event listeners
    for (auto evt : eventsList)
        LogManager::RemoveEventListener(evt, listener);

    return 0;
}
