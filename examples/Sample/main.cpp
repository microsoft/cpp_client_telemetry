#include "LogManager.hpp"

#include "Enums.hpp"
#include <iostream>

#include <time.h>

#include <vector>
#include <ctime>

using namespace Microsoft::Applications::Telemetry;

#define TOKEN "3cebeafc3f674518b3754f3de9d539a9-e6a1edec-cc4e-4af5-9c52-b20c279edb94-6808"

ILogConfiguration& configuration = LogManager::GetLogConfiguration();


ILogger* init(bool mode = false)
{
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, "offlinestorage.db");
    configuration.SetProperty(CFG_STR_COLLECTOR_URL, "https://v10.events.data.microsoft.com/OneCollector/1.0/");
    configuration.SetSdkModeType(
        (mode) ?
        SdkModeTypes_UTCAriaBackCompat :
        SdkModeTypes_Aria
    );
    return LogManager::Initialize(TOKEN);
}


int main(int argc, char* argv[])
{
    if (argc);
    const char *program = argv[0];

    bool flipFlop = false;

    while (true)
    {
        ILogger* logger = init(flipFlop);
        for (size_t i = 0; i < 10; i++)
        {
            EventProperties props("OneSdkDemo.TestEvent");
            props.SetPolicyBitFlags(MICROSOFT_KEYWORD_CRITICAL_DATA | MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_EVENTTAG_REALTIME_LATENCY);
            props.SetProperty("strKey",   "someValue");
            props.SetProperty("intKey",   12345L);
            props.SetProperty("boolKey",  (bool)true);
            props.SetProperty("dblKey",   3.14);
            props.SetProperty("guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F"));
            props.SetProperty("timeKey",  time_ticks_t((uint64_t)0));
            props.SetProperty("compilerVersion", _MSC_FULL_VER);
            logger->LogEvent(props);
        }

        std::cout << "Press <ENTER> to send more events" << std::endl;
        fgetc(stdin);
        LogManager::FlushAndTeardown();
        flipFlop = !flipFlop;
    }

    std::cout << "Press <ENTER> to exit" << std::endl;
    fgetc(stdin);

    return 0;
}
