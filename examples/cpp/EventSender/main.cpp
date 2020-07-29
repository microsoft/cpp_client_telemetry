#include <cstdio>

#include "LogManager.hpp"                              // include LogManager header that implements 1DS API

using namespace MAT;                                   // all code lives in Microsoft Applications Telemetry package
LOGMANAGER_INSTANCE                                    // default instance (SDK allows to run multiple instances in one process)
#define AI_KEY "4861647d-5f17-40c2-8b52-f5ad2f535464"  // Specify instrumentation key to be used for ingestion

int main(int argc, char* argv[])
{
    auto& config = LogManager::GetLogConfiguration();  // Obtain default configuration
    config[CFG_INT_SDK_MODE] = SdkModeTypes_AI;        // Configure SDK in Application Insights mode...
    config[CFG_STR_PRIMARY_TOKEN] = AI_KEY;            // Specify instrumentation key
    ILogger* logger = LogManager::Initialize();        // Initialize SDK

    printf("Log My.Simple.Event...\n");
    logger->LogEvent("My.Simple.Event");

    EventProperties evt("My.Detailed.Event",           // Event with custom properties
    {
        {"strKey1", "Hello"},
        {"strKey2", "Application Insights!"},
        {"int64Key", int64_t(1L)},
        {"dblKey", 3.14},
        {"guid", GUID_t("01234567-890A-BCDE-F012-34567890ABCD")},
    });
    printf("Log My.Detailed.Event...\n");
    logger->LogEvent(evt);                             // Emit structured event with custom properties
    LogManager::UploadNow();                           // Events get batched. But we can force the upload.

    printf("Press any key to exit.\n");
    fgetc(stdin);
    return 0;
}