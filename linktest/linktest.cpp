#include <aria/ILogManager.hpp>
#include <memory>


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


#elif ARIASDK_PAL_WIN32

#include <Windows.h>


static void platformInit()
{
}

static void platformCleanup()
{
}

static void platformSleepMs(unsigned ms)
{
    ::Sleep(ms);
}

static char const* platformGetUsername()
{
    size_t result = 0;
    static char username[100];
    ::getenv_s(&result, username, "USERNAME");
    return (result > 0) ? username : "Dummy";
}


#else

#error Unsupported PAL.

#endif


#pragma warning(suppress:4447) // 'main' signature found without threading model. Consider using 'int main(Platform::Array<Platform::String^>^ args)'.
int main()
{
    platformInit();

    {
        using namespace Microsoft::Applications::Telemetry;

        LogConfiguration config;
        std::unique_ptr<ILogManager> logManager(ILogManager::Create(config));

        ISemanticContext& semanticContext = logManager->GetSemanticContext();
        semanticContext.SetUserId(platformGetUsername());
        semanticContext.SetAppVersion("999/1.0.0.2/"); // Stolen from SkyLibTest2


        ILogger* logger = logManager->GetLogger("3db013fd10854e539b31037c9fc300b1-ed0342ba-3967-4213-b29a-9ff9cb9378f5-6541"); // Aria SDK Sandbox

        EventProperties event("ariasdk_test_linktest");
        event.SetProperty("result", "Success");
        event.SetProperty("random", rand());
        event.SetProperty("secret", 5.6872);
        logger->LogEvent(event);


        platformSleepMs(5000);


        logManager->FlushAndTeardown();
    }

    platformCleanup();

    return 0;
}
