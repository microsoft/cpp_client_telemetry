#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>

#include "LogManagerA.hpp"
#include "LogManagerB.hpp"

#ifdef _WIN32
#include <Windows.h>
#endif

#include "DebugCallback.hpp"

#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"

/**
* All v1.x legacy API compile-time checks in alphabetic order
*/
void Api_v1_CompatChecks()
{
    ILogger *logger = LogManager::GetLogger();
    EventProperties props("name");
    logger->GetSemanticContext();

    AggregatedMetricData amd("name", 0, 0);
    logger->LogAggregatedMetric(amd, props);

    AppLifecycleState state = AppLifecycleState_Unknown;
    logger->LogAppLifecycle(state, props);

    logger->LogEvent(props);
    logger->LogEvent("name");

    logger->LogFailure("signature", "detail", props);

    PageActionData pad("name", ActionType_Unknown);
    logger->LogPageAction(pad, props);

    logger->LogPageView("id", "pageName", props);

    logger->LogSampledMetric("name", 0.1, "units", props);

    SessionState sess(Session_Started);
    logger->LogSession(sess, props);

    logger->LogTrace(TraceLevel_Error, "message", props);

    logger->LogUserState(UserState_Unknown, 1L, props);

    // TODO: add more checks for various context types
    logger->SetContext("var", 12345);
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

using namespace Microsoft::Applications::Telemetry;

MyDebugEventListener listener;

void guestTest()
{

    {
        auto& config = LogManagerA::GetLogConfiguration();
        config["name"] = "ModuleA";
        config["version"] = "1.2.5";
        config["config"]["host"] = "*"; // Any host
    }

    {
        auto& config = LogManagerB::GetLogConfiguration();
        config["name"] = "ModuleB";
        config["version"] = "1.2.5";
        config["config"]["host"] = "*"; // Any host
    }

    auto loggerA = LogManagerA::Initialize(TOKEN);
    auto loggerB = LogManagerB::Initialize(TOKEN);

    loggerA->LogEvent("HelloFromModuleA");
    loggerB->LogEvent("HelloFromModuleB");

}

int main()
{
#if 0
    const char *s1 = "{ \"test\": 1, \"str\": \"string\" }";
    auto jj = FromJSON(s1);
    std::string s2;
    Variant::serialize(jj, s2);
    printf("%s\n", s2.c_str());
#endif

    // Guest SDKs start first
    guestTest();

    // Host SDK starts
    printf("Setting up configuration...\n");
    auto& config = LogManager::GetLogConfiguration();
    config["name"] = "HelloAria";
    config["version"] = "1.2.5";
    config["config"]["host"] = "HelloAria"; // host

    config[CFG_STR_CACHE_FILE_PATH]   = "offlinestorage.db";
    config[CFG_INT_TRACE_LEVEL_MASK]  = 0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN]   = ACTTraceLevel_Debug;
    config[CFG_INT_SDK_MODE]          = SdkModeTypes::SdkModeTypes_Aria;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 5;
#ifdef USE_INVALID_URL	/* Stress-test for the case when collector is unreachable */
    config[CFG_STR_COLLECTOR_URL]     = "https://127.0.0.1/invalid/url";
#endif
    config[CFG_INT_RAM_QUEUE_SIZE]    = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config[CFG_INT_CACHE_FILE_SIZE]   = 16 * 1024 * 1024; // 16 MB storage file limit

    // printf("LogConfiguration: %s\n", configuration.data());

    printf("Adding debug event listener...\n");
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

    printf("LogManager::Initialize\n");
    ILogger *logger = LogManager::Initialize(TOKEN);

    printf("LogManager::GetSemanticContext\n");
    ISemanticContext* semanticContext = LogManager::GetSemanticContext();

    printf("Starting stress-test...\n");
    for(size_t i = 1; i <= 10; i++)
    {
        std::string eventName("ariasdk_test_linktest");
        EventProperties event(eventName);
        event.SetProperty("result", "Success");
        event.SetProperty("random", rand());
        event.SetProperty("secret", 5.6872);
        event.SetProperty("seq", i);

        samplingTest();

        logger->LogEvent(event);
        if((i % 1000) == 0)
            printf("processed %llu events...\n", i);
    }

    // Default transmission timers:
    // high		- 2 sec
    // normal	- 4 sec
    // low		- 8 sec

    printf("LogManager::UploadNow\n");
    LogManager::UploadNow();

    // Sleep for 5 seconds
    sleep(5000);

    printf("LogManager::FlushAndTeardown\n");
    LogManager::FlushAndTeardown();

    return 0;
}
