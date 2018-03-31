#if 1
#define _CRT_SECURE_NO_WARNINGS

#include "LogManager.hpp"

#include <memory>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>

#define	 ENTER	printf("%s\n", __FUNCTION__)

typedef std::chrono::milliseconds ms;

// Delay execution for specified number of milliseconds. Generally for testing code only.
inline void sleep(unsigned delayMs)
{
    std::this_thread::sleep_for(ms(delayMs));
}

#ifdef ARIASDK_PAL_SKYPE

#include <auf/auf.hpp>
#include <spl/spl_sysinfo.hpp>

static void platformInit()
{
    auf::g_configLogHookStdoutPreinstalled = 1;
    auf::init();
}

static void platformCleanup()
{
    auf::stop();
}

static void platformSleepMs(unsigned ms)
{
    spl::sleep(ms * 1000);
}

static char const* platformGetUsername()
{
    return spl::sysInfoUserName() ? spl::sysInfoUserName() : "Dummy";
}
#else

#ifdef _WIN32
#include <Windows.h>
#endif

static void platformInit()
{
    ENTER;
}

static void platformCleanup()
{
    ENTER;
}

static const char* platformGetUsername()
{
    ENTER;
    static const char * none = "none";
    static const char * username = getenv("USERNAME");
    if(username == NULL)
    {
        username = none;
    }
    return username;
}

#endif

#pragma warning(suppress:4447) // 'main' signature found without threading model. Consider using 'int main(Platform::Array<Platform::String^>^ args)'.

using namespace Microsoft::Applications::Telemetry;

#define TOKEN       "4bd39c465b534cad9c1da2ae998b549a-6e15bcfd-4743-4ee8-a3f2-d9708afb783e-7102"

int main()
{
    platformInit();

    {
        printf("Setting up configuration...\n");
        auto& configuration = LogManager::GetLogConfiguration();
        configuration[CFG_STR_CACHE_FILE_PATH]      = "offlinestorage.db";
        configuration[CFG_INT_TRACE_LEVEL_MASK]     = 0xFFFFFFFF ^ 128;
        configuration[CFG_INT_TRACE_LEVEL_MIN]      = ACTTraceLevel_Trace;
        configuration[CFG_INT_SDK_MODE]             = SdkModeTypes::SdkModeTypes_Aria;
        configuration[CFG_INT_MAX_TEARDOWN_TIME]    = 5;

        printf("LogConfiguration:\n");

        printf("%s=%s\n", CFG_STR_CACHE_FILE_PATH,   configuration.GetProperty(CFG_STR_CACHE_FILE_PATH));
        printf("%s=%x\n", CFG_INT_TRACE_LEVEL_MASK,  configuration.GetIntProperty(CFG_INT_TRACE_LEVEL_MASK));
        printf("%s=%d\n", CFG_INT_TRACE_LEVEL_MIN,   configuration.GetIntProperty(CFG_INT_TRACE_LEVEL_MIN));
        printf("%s=%d\n", CFG_INT_SDK_MODE,          configuration.GetIntProperty(CFG_INT_SDK_MODE));
        printf("%s=%d\n", CFG_INT_MAX_TEARDOWN_TIME, configuration.GetIntProperty(CFG_INT_MAX_TEARDOWN_TIME));

        printf("LogManager::Initialize\n");
        ILogger *logger = LogManager::Initialize(TOKEN);

        printf("LogManager::GetSemanticContext\n");
        ISemanticContext* semanticContext = LogManager::GetSemanticContext();

        printf("semanticContext->Set...\n");
        semanticContext->SetUserId(platformGetUsername());
        semanticContext->SetAppVersion("1.0");

        // LogManager::PauseTransmission();

        for(size_t i = 1; i <= 100000; i++)
        {
            std::string eventName("ariasdk_test_linktest");
            EventProperties event(eventName);
            event.SetProperty("result", "Success");
            event.SetProperty("random", rand());
            event.SetProperty("secret", 5.6872);
            event.SetProperty("seq", i);
            logger->LogEvent(event);
        }

        printf("LogManager::UploadNow\n");
        LogManager::UploadNow();

//        Sleep for 5 seconds
        sleep(5000);

        printf("LogManager::FlushAndTeardown\n");
        LogManager::FlushAndTeardown();
    }
    platformCleanup();

    return 0;
}
#endif
