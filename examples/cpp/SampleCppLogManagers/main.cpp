#include "LogManagerA.hpp"
#include "LogManagerB.hpp"
#include "CommonFields.h"
#include <cstdio>

#define TOKEN   "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999"

using namespace MAT;

// Define two instances for different LogManagers: A and B
// This is how it's done with LOGMANAGER_INSTANCE macro to define the
// default LogManager instance
namespace MAT_NS_BEGIN {
    DEFINE_LOGMANAGER(LogManagerB, ModuleB);
    DEFINE_LOGMANAGER(LogManagerA, ModuleA);
} MAT_NS_END

void twoModules_LogManagerTest() 
{
    printf("Configuring two LogManager instances...\n");
    {
        printf("Creating configuration for LogManagerA \n");
        printf("Using direct upload mode... \n");
        auto& config = LogManagerA::GetLogConfiguration();
        config["name"] = "ModuleA";
        config["version"] = "1.2.5";
        config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    }

    {
        printf("Creating configuration for LogManagerB \n");
        printf("Using UTC mode... \n");
        auto& config = LogManagerB::GetLogConfiguration();
        config["name"] = "ModuleB";
        config["version"] = "1.2.5";
        config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;
    }

    printf("Initializing LogManager instances... \n");
    auto loggerA = LogManagerA::Initialize(TOKEN);
    auto loggerB = LogManagerB::Initialize(TOKEN);

    EventProperties event;
    event.SetName("SampleLogManagers.ModuleA.Direct_Event");
    event.SetProperty("result", "Success");
    event.SetProperty("random", rand());
    event.SetProperty("secret", (double)5.6872);
    event.SetProperty("seq", (uint64_t)4);
    loggerA->LogEvent(event);

    EventProperties event2;
    event2.SetName("SampleLogManagers.ModuleB.UTC_Event");
    event2.SetProperty("result", "Success");
    event2.SetProperty("random", rand());
    event2.SetProperty("secret", (double)5.6872);
    event2.SetProperty("seq", (uint64_t)4);
    event2.SetProperty(COMMONFIELDS_EVENT_PRIVTAGS, PDT_ProductAndServicePerformance);
    event2.SetProperty(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_REQUIRED);

    loggerB->LogEvent(event2);
}

void provider_LogManagerTest()
{
    printf("Creating configuration for Direct Upload LogManager...\n");
    ILogConfiguration config1, config2;
    config1["name"] = "Instance1";
    config1["version"] = "1.2.5";
    config1["config"]["host"] = "Instance1";
    config1[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
    config1[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    config1[CFG_INT_TRACE_LEVEL_MASK] = 0;  // 0xFFFFFFFF ^ 128;
    config1[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn; // ACTTraceLevel_Info; // ACTTraceLevel_Debug;
    config1[CFG_INT_MAX_TEARDOWN_TIME] = 10;
    config1[CFG_INT_RAM_QUEUE_SIZE] = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config1[CFG_INT_CACHE_FILE_SIZE] = 16 * 1024 * 1024; // 16 MB storage file limit

    printf("Creating configuration for UTC LogManager...\n");
    config2["name"] = "Instance2";
    config2["version"] = "1.2.5";
    config2["config"]["host"] = "Instance2";
    config1[CFG_STR_COLLECTOR_URL] = COLLECTOR_URL_PROD;
    config2[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;
    config2[CFG_INT_TRACE_LEVEL_MASK] = 0;  // 0xFFFFFFFF ^ 128;
    config2[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn; // ACTTraceLevel_Info; // ACTTraceLevel_Debug;
    config2[CFG_INT_MAX_TEARDOWN_TIME] = 10;
    config2[CFG_INT_RAM_QUEUE_SIZE] = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config2[CFG_INT_CACHE_FILE_SIZE] = 16 * 1024 * 1024; // 16 MB storage file limit

    printf("Creating both LogManagers...\n");
    status_t status = STATUS_SUCCESS;
    std::unique_ptr<ILogManager> lm1(LogManagerProvider::CreateLogManager(config1, status));
    std::unique_ptr<ILogManager> lm2(LogManagerProvider::CreateLogManager(config2, status));

    lm1->SetContext("test1", "abc");

    lm2->GetSemanticContext().SetAppId("123");

    ILogger* l1a = lm1->GetLogger(TOKEN, "logger_direct");
    ILogger* l2a = lm2->GetLogger(TOKEN, "logger_utc");

    printf("Logging event through UTC logger ...\n");
    EventProperties l2a1p("l2a1");
    l2a1p.SetProperty("x", "y");
    l2a1p.SetProperty("result", "Success");
    l2a1p.SetProperty("random", rand());
    l2a1p.SetProperty("secret", (double)1.21872);
    l2a1p.SetProperty("seq", (uint64_t)4);
    l2a1p.SetProperty(COMMONFIELDS_EVENT_PRIVTAGS, PDT_ProductAndServicePerformance);
    l2a1p.SetProperty(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_OPTIONAL);
    l2a->LogEvent(l2a1p);

    printf("Logging event through direct logger, source 1 ...\n");
    EventProperties l1a1p("l1a1");
    l1a1p.SetProperty("X", "Y");
    l1a1p.SetProperty("result", "Success");
    l1a1p.SetProperty("random", rand());
    l1a->LogEvent(l1a1p);

    printf("Logging event through direct logger, source 2 ...\n");
    ILogger* l1b = lm1->GetLogger("bbb");
    EventProperties l1b1p("l1b1");
    l1b1p.SetProperty("asdf", 1234);
    l1b->LogEvent(l1b1p);

    printf("Uploading...\n");
    lm1->GetLogController()->UploadNow();
    lm2->GetLogController()->UploadNow();

    lm1.reset();
    lm2.reset();
}

int main()
{
    twoModules_LogManagerTest();
    provider_LogManagerTest();
    return 0;
}
