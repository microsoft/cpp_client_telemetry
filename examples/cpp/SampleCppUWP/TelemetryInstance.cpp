#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"

#include "LogManager.hpp"
#include "Utils.hpp"

#include <string>
#include <thread>

// Windows SDK Test - Prod: Default Ingestion Token.
#define TOKEN   "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999"

// Windows SDK Test 2 - Int: Default Ingestion Token.
#define TOKEN2  "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9990"

using namespace std;
using namespace MAT;

LOGMANAGER_INSTANCE;

const std::string tenantToken = TOKEN;

static ILogConfiguration& configuration = LogManager::GetLogConfiguration();

void TelemetryInitialize()
{
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFFF;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Debug;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes_UTCCommonSchema;
    ILogger *logger = LogManager::Initialize(TOKEN);
    LogManager::GetSemanticContext()->SetAppId("SampleCppUWP");
    logger->LogSession(Session_Started, EventProperties("Microsoft.SampleCppUWP.AppSession"));
    logger->LogEvent("Microsoft.SampleCppUWP.EventSimple");
}

void TelemetryTeardown()
{
    ILogger *logger = LogManager::GetLogger("shutdown");
    ISemanticContext *context = LogManager::GetSemanticContext();
    logger->LogSession(Session_Ended, EventProperties("Microsoft.SampleCppUWP.AppSession"));
    LogManager::FlushAndTeardown();
}

/**
 * Start on process start
 * Stop on process shutdown
 */
struct TelemetryInstance {


    TelemetryInstance()
    {
        TelemetryInitialize();
    }

    ~TelemetryInstance()
    {
        TelemetryTeardown();
    };

};

static TelemetryInstance sdkSingleton;
