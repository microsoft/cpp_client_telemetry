#import "WLogManager.h"

LOGMANAGER_INSTANCE

@implementation WLogManager

+(nullable id) initForTenant: (nonnull NSString*) tenantToken{
    /*printf("Setting up configuration...\n");
    auto& config = LogManager::GetLogConfiguration();

    config["name"] = "SampleObjectiveC";
    config["version"] = "1.2.5";
    config["config"]["host"] = "SampleObjectiveC"; // host
    config["compat"]["dotType"] = false;    // Legacy v1 behaviour with respect to SetType using underscore instead of a dot
    config[CFG_STR_CACHE_FILE_PATH]   = "/tmp/offlinestorage.db";
    config[CFG_INT_TRACE_LEVEL_MASK]  = 0;  // 0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN]   = ACTTraceLevel_Warn; // ACTTraceLevel_Info; // ACTTraceLevel_Debug;
    config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;
    config[CFG_INT_RAM_QUEUE_SIZE]    = 32 * 1024 * 1024;  // 32 MB heap limit for sqlite3
    config[CFG_INT_CACHE_FILE_SIZE]   = 16 * 1024 * 1024; // 16 MB storage file limit*/


    std::string strToken = std::string([tenantToken UTF8String]);
    ILogger* logger = LogManager::Initialize(strToken);

    // This global context variable will not be seen by C API client
    LogManager::SetContext("GlobalContext.Var", 12345);
     
    printf("LogManager::GetSemanticContext \n"); 
    ISemanticContext* semanticContext = LogManager::GetSemanticContext();

    semanticContext->SetAppId("ObjectiveC_App");     // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetAppVersion("1.0.1");    // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetAppLanguage("en-US");   // caller must obtain this from app manifest, e.g. .plist on Mac OS X
    semanticContext->SetUserLanguage("en-US");  // caller must obtain the user language from preferences
    if(!logger) return nil;
    return [[WLogger alloc] initWithILogger: logger];
}

+(nullable id) getLogger{
    ILogger* logger = LogManager::GetLogger();
    if(!logger) return nil;
    return [[WLogger alloc] initWithILogger: logger];
}

+(nullable id) getLoggerForSource: (nonnull NSString*) source{
    std::string strSource = std::string([source UTF8String]);
    ILogger* logger = LogManager::GetLogger(strSource);
    if(!logger) return nil;
    return [[WLogger alloc] initWithILogger: logger];
}

+(void) uploadNow{
    LogManager::UploadNow();
}

+(void) flush{
    LogManager::Flush();
}

+(void) flushAndTeardown{
    LogManager::FlushAndTeardown();
}

+(void) pauseTransmission{
    LogManager::PauseTransmission();
}

+(void) resumeTransmission{
    LogManager::ResumeTransmission();
}

+(void) resetTransmitProfiles{
    LogManager::ResetTransmitProfiles();
}


@end