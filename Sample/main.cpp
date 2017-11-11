#include "LogManager.hpp"

#include "Enums.hpp"
#include <iostream>

#include <time.h>

#include <vector>
#include <ctime>

using namespace Microsoft::Applications::Telemetry;

#define TOKEN   "b60cc5413ca74e4ba47146997e534810-694a01ac-0f96-4568-bceb-fe1b6dc510e1-6969"
// https://aria.int.trafficmanager.net/eventinspector?tenant=b60cc5413ca74e4ba47146997e534810

ILogConfiguration& configuration = LogManager::GetLogConfiguration();

ILogger* init(bool mode = false)
{
    configuration.SetProperty(CFG_STR_CACHE_FILE_PATH, "offlinestorage.db");
    configuration.SetProperty(CFG_STR_COLLECTOR_URL, "https://pipe.int.trafficmanager.net/OneCollector/1.0");
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
            EventProperties props("one_sdk_demo");
            props.SetPolicyBitFlags(MICROSOFT_KEYWORD_CRITICAL_DATA | MICROSOFT_EVENTTAG_CORE_DATA | MICROSOFT_EVENTTAG_REALTIME_LATENCY);
            props.SetProperty("strKey", "someValue");
            props.SetProperty("intKey", 12345L);
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