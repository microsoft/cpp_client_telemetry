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

#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"
 
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

void samplingTest()
{
    const char *sampledList[] = {
        "MyEvent1",
        "MyEvent2",
        "MyEvent3",
        "MyEvent4"
    };
    uint32_t samplingRates[] = { 100, 75, 50, 0 };
    LogManager::SetExclusionFilter(TOKEN, sampledList, samplingRates, 4);
    
    ILogger *logger = LogManager::GetLogger();
    for (size_t i = 0; i < 100; i++)
    {
        logger->LogEvent("MyEvent1");
        logger->LogEvent("MyEvent2");
        logger->LogEvent("MyEvent3");
        logger->LogEvent("MyEvent4");
        logger->LogEvent("NS1.MyEvent1.Foo");
        logger->LogEvent("NS2.MyEvent2.Bar");
        logger->LogEvent("NS3.MyEvent3.Toor");
        logger->LogEvent("NS4.MyEvent4.Root");
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

#define MAX_EVENTS_TO_LOG       50000L

extern "C" int OfficeTest();
extern "C" void test_c_api();

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
    config["name"] = "HelloAria";
    config["version"] = "1.2.5";
    config["config"]["host"] = "HelloAria"; // host

    config[CFG_STR_CACHE_FILE_PATH]   = "offlinestorage.db";
    config[CFG_INT_TRACE_LEVEL_MASK]  = 0;  0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN]   = ACTTraceLevel_Warn; // ACTTraceLevel_Info; // ACTTraceLevel_Debug;
    config[CFG_INT_SDK_MODE]          = SdkModeTypes::SdkModeTypes_Aria;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;
#ifdef USE_INVALID_URL	/* Stress-test for the case when collector is unreachable */
    config[CFG_STR_COLLECTOR_URL]     = "https://127.0.0.1/invalid/url";
#endif
    config[CFG_INT_RAM_QUEUE_SIZE]    = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config[CFG_INT_CACHE_FILE_SIZE]   = 16 * 1024 * 1024; // 16 MB storage file limit

    // printf("LogConfiguration: %s\n", configuration.data());

    printf("Adding debug event listeners...\n");
    auto eventsList = {
        DebugEventType::EVT_LOG_EVENT,
        DebugEventType::EVT_LOG_SESSION,
        DebugEventType::EVT_REJECTED,
        DebugEventType::EVT_SEND_FAILED,
        DebugEventType::EVT_SENT,
        DebugEventType::EVT_DROPPED,
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

    printf("LogManager::Initialize\n");
    ILogger *logger = LogManager::Initialize(TOKEN);
    
#if 1
    // LogManager::PauseTransmission();
    logger->LogEvent("TestEvent");
#endif

    // Api_v1_CompatChecks();
     
    printf("LogManager::GetSemanticContext \n");
    ISemanticContext* semanticContext = LogManager::GetSemanticContext();

    // Ingest events of various latencies
    printf("Starting stress-test...\n");
    for(size_t i = 1; i <= MAX_EVENTS_TO_LOG; i++)
    {
        EventLatency latency = (EventLatency)(1 + i % (unsigned)EventLatency_RealTime);
        std::string eventName("sample_event_lat");
        eventName += std::to_string((unsigned)latency);

        EventProperties event(eventName);
        event.SetProperty("result", "Success");
        event.SetProperty("random", rand());
        event.SetProperty("secret", 5.6872);
        event.SetProperty("seq", (uint64_t)i);
        event.SetLatency(latency);
        logger->LogEvent(event);
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
