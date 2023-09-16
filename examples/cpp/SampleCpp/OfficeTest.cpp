#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "LogManager.hpp"

#include "ILogConfiguration.hpp"

LOGMANAGER_INSTANCE;

using namespace MAT;

const char* officeCustomTransmitProfile = R"(
[{
    "name": "Office_Telemetry_CustomProfile",
    "rules": [
    { "netCost": "restricted", "timers": [  -1,  -1,  -1 ] },
    { "netCost": "high",       "timers": [  -1,  -1, 300 ] },
    { "netCost": "low",        "timers": [ 300, 300, 300 ] },
    { "netCost": "unknown",    "timers": [ 300, 300, 300 ] },
    {                          "timers": [ 300, 300, 300 ] }
    ]
}]
)";

void SendEvents(ILogger* pLogger, uint8_t eventCount, std::chrono::milliseconds sleepTime)
{
    for (auto i = 0; i < eventCount; ++i)
    {
        pLogger->LogEvent("TestEvent");
        if (sleepTime >= std::chrono::milliseconds(0)) std::this_thread::sleep_for(sleepTime);
    }
}

void SetupLogConfiguration(ILogConfiguration& config)
{
    config[CFG_STR_CACHE_FILE_PATH]   = "Wrapper.db";
    config[CFG_INT_CACHE_FILE_SIZE]   = 10 * 1024 * 1024;
    config[CFG_INT_RAM_QUEUE_SIZE]    = 2 * 1024 * 1024;
    config[CFG_INT_RAM_QUEUE_BUFFERS] = 10;
    config[CFG_INT_TRACE_LEVEL_MASK]  = 0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN]   = ACTTraceLevel::ACTTraceLevel_Trace;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 20;
    config[CFG_INT_MAX_PENDING_REQ]   = 4;

    //  SQLite Settings -- // Though it looks like these aren't used at all
    config[CFG_BOOL_ENABLE_WAL_JOURNAL] = true;
    config[CFG_STR_PRAGMA_JOURNAL_MODE] = "WAL";
    config[CFG_STR_PRAGMA_SYNCHRONOUS]  = "NORMAL";
}

extern "C" int OfficeTest()
{
    char foo[4] = { 'f','o','o','o' };
    char bar[20] = {};
    strncpy_s(bar, 20, foo, _TRUNCATE);

    SetupLogConfiguration(LogManager::GetLogConfiguration());
    LogManager::LoadTransmitProfiles(officeCustomTransmitProfile);
    LogManager::SetTransmitProfile("Office_Telemetry_CustomProfile");
    ILogger* pLogger = LogManager::Initialize("eab8635a626e40d1b5e91352f4c09548-853df1db-5a5d-443b-af59-b73cad4497ba-7070");
    if (pLogger == nullptr)
        return 1;
    SendEvents(pLogger, 1, std::chrono::milliseconds(50));
    LogManager::UploadNow();
    LogManager::UploadMaxNow();
    LogManager::FlushAndTeardown();
    printf("Test successfully completed!\n");
    return 0;
}
