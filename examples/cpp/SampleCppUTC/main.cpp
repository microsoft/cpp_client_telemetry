#define _CRT_SECURE_NO_WARNINGS
#pragma warning(suppress:4447) // 'main' signature found without threading mode. Consider using 'int main(Platform::Array<Platform::String^>^ args)'.

#include "LogManager.hpp"

#ifdef _WIN32
    #include <Windows.h>
#endif

#include "CommonFields.h"

LOGMANAGER_INSTANCE

using namespace MAT;

#define TENANT_TOKEN   "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999"

void forwardEventToUTC()
{
    printf("LogManager init\n");
    ILogger *logger = LogManager::Initialize(TENANT_TOKEN);
 
    printf("Updating LogManager context \n");
    ISemanticContext* semanticContext = LogManager::GetSemanticContext();
    semanticContext->SetAppId("MyAppName");     // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetAppVersion("1.0.1");    // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetAppLanguage("en-US");   // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetUserLanguage("en-US");  // caller must obtain the user language from preferences

#ifndef _WIN32
    // Platforms other than Windows currently do not have automatic network detection implemented,
    // so the caller must populate these fields using semantic context API
    semanticContext->SetNetworkCost(MAT::NetworkCost::NetworkCost_Unmetered);
    semanticContext->SetNetworkType(MAT::NetworkType::NetworkType_Wired);
#endif

    // Ingest 5 sample events.
    for(size_t i = 1; i <= 5; i++)
    {
        EventLatency latency = EventLatency_Normal;
        std::string eventName("Microsoft.Applications.Telemetry.ControlOptIn.sample_event");
        eventName += std::to_string((unsigned)i);

        EventProperties event(eventName);

        std::string evtType = "My.Record.BaseType"; // default v1 legacy behaviour: custom.my_record_basetype
        event.SetType(evtType);
        event.SetLatency(latency);

        // To make event observable in DDV (Diagnostic Data Viewer)
        event.SetProperty(COMMONFIELDS_METADATA_VIEWINGPRODUCERID, "MyAppName");
        event.SetProperty(COMMONFIELDS_METADATA_VIEWINGCATEGORY, "Category1|Category2");

        // Any extra DDV related information
        event.SetProperty(COMMONFIELDS_METADATA_VIEWINGPAYLOADDECODERPATH,"EnterPathOrRegistryKey");
        event.SetProperty(COMMONFIELDS_METADATA_VIEWINGPAYLOADENCODEDFIELDNAME,"EnterEncodedFieldName");
        event.SetProperty(COMMONFIELDS_METADATA_VIEWINGEXTRA1, "ExtraDataField1");
        event.SetProperty(COMMONFIELDS_METADATA_VIEWINGEXTRA2, "ExtraDataField2");
        event.SetProperty(COMMONFIELDS_METADATA_VIEWINGEXTRA3, "ExtraDataField3");

        logger->LogEvent(event);
    }

    printf("LogManager::FlushAndTeardown\n");
    LogManager::FlushAndTeardown();
}

void setupConfigs()
{
    auto& config = LogManager::GetLogConfiguration();

    // Specify Provider-Group for event
    // NB: Defaults to ARIA
    // config[CFG_STR_UTC][CFG_STR_PROVIDER_GROUP_ID] = "5ECB0BAC-B930-47F5-A8A4-E8253529EDB7";

    // Set Telemetry System to UTC.
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;

    // Other Configs.
    config[CFG_INT_TRACE_LEVEL_MASK] = 0;  // 0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Warn; // ACTTraceLevel_Info; // ACTTraceLevel_Debug;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;
    config[CFG_INT_RAM_QUEUE_SIZE] = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config[CFG_INT_CACHE_FILE_SIZE] = 16 * 1024 * 1024; // 16 MB storage file limit

#ifdef __APPLE__
    config[CFG_STR_CACHE_FILE_PATH] = "/tmp/offlinestorage.db";
#else
    config[CFG_STR_CACHE_FILE_PATH] = "offlinestorage.db";
#endif
#ifdef USE_INVALID_URL	/* Stress-test for the case when collector is unreachable */
    config[CFG_STR_COLLECTOR_URL] = "https://127.0.0.1/invalid/url";
#endif
}

int main()
{
    printf("ControlOptIn Aria 1DS Sample App\n\n");

    printf("Setting up configuration...\n");
    setupConfigs();

    forwardEventToUTC();

    return 0;
}
