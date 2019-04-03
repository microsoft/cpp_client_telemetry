#include <cstdlib>
#include <cstdio>

#include "LogManager.hpp"

LOGMANAGER_INSTANCE

#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"

using namespace MAT;

int main(int argc, char *argv[])
{
    printf("Setting up configuration...\n");

    auto& config = LogManager::GetLogConfiguration();
    config["name"] = "HelloAria";
    config["version"] = "1.2.5";
    config["config"]["host"] = "HelloAria"; // host
    config["compat"]["dotType"] = false;    // Legacy v1 behaviour with respect to SetType using underscore instead of a dot
    config[CFG_STR_CACHE_FILE_PATH] = "offlinestorage.db";
    config[CFG_INT_TRACE_LEVEL_MASK] = 0;  // 0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn; // ACTTraceLevel_Info; // ACTTraceLevel_Debug;
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS; // SdkModeTypes::SdkModeTypes_UTCCommonSchema
    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;
    config[CFG_INT_RAM_QUEUE_SIZE] = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config[CFG_INT_CACHE_FILE_SIZE] = 16 * 1024 * 1024; // 16 MB storage file limit

    printf("LogManager::Initialize\n");
    ILogger *logger = LogManager::Initialize(TOKEN);

    logger->LogEvent("fooBar");
    LogManager::FlushAndTeardown();
    printf("[ DONE ]\n");

    return 0;
}
