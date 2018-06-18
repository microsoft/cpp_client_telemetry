#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "DebugCallback.hpp"

#define	 ENTER	printf("%s\n", __FUNCTION__)

typedef std::chrono::milliseconds ms;

// Delay execution for specified number of milliseconds. Generally for testing code only.
inline void sleep(unsigned delayMs)
{
    std::this_thread::sleep_for(ms(delayMs));
}

#pragma warning(suppress:4447) // 'main' signature found without threading model. Consider using 'int main(Platform::Array<Platform::String^>^ args)'.

#include "LogManager.hpp"

using namespace Microsoft::Applications::Events;

#define TOKEN       "4bd39c465b534cad9c1da2ae998b549a-6e15bcfd-4743-4ee8-a3f2-d9708afb783e-7102"

MyDebugEventListener listener;

int main()
{
    printf("Setting up configuration...\n");
    auto& configuration = LogManager::GetLogConfiguration();
    configuration[CFG_STR_CACHE_FILE_PATH]   = "offlinestorage.db";
    configuration[CFG_INT_TRACE_LEVEL_MASK]  = 0xFFFFFFFF ^ 128;
    configuration[CFG_INT_TRACE_LEVEL_MIN]   = ACTTraceLevel_Debug;
    configuration[CFG_INT_SDK_MODE]          = SdkModeTypes::SdkModeTypes_Aria;
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 5;
#ifdef USE_INVALID_URL	/* Stress-test for the case when collector is unreachable */
    configuration[CFG_STR_COLLECTOR_URL]     = "https://127.0.0.1/invalid/url";
#endif
    configuration[CFG_INT_RAM_QUEUE_SIZE]    = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    configuration[CFG_INT_CACHE_FILE_SIZE]   = 16 * 1024 * 1024; // 16 MB storage file limit

    printf("LogConfiguration:\n");

    printf("%s=%s\n", CFG_STR_CACHE_FILE_PATH,   configuration.GetProperty(CFG_STR_CACHE_FILE_PATH));
    printf("%s=%x\n", CFG_INT_TRACE_LEVEL_MASK,  configuration.GetIntProperty(CFG_INT_TRACE_LEVEL_MASK));
    printf("%s=%d\n", CFG_INT_TRACE_LEVEL_MIN,   configuration.GetIntProperty(CFG_INT_TRACE_LEVEL_MIN));
    printf("%s=%d\n", CFG_INT_SDK_MODE,          configuration.GetIntProperty(CFG_INT_SDK_MODE));
    printf("%s=%d\n", CFG_INT_MAX_TEARDOWN_TIME, configuration.GetIntProperty(CFG_INT_MAX_TEARDOWN_TIME));

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
    for(size_t i = 1; i <= 30000; i++)
    {
        std::string eventName("Microsoft.Applications.Events.SampleEventOne");
        EventProperties event(eventName);
        event.SetProperty("result", "Success");
        event.SetProperty("random", rand());
        event.SetProperty("secret", 5.6872);
        event.SetProperty("seq", i);
        logger->LogEvent(event);
        if((i % 1000) == 0)
            printf("processed %d events...\n", i);
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
