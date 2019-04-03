#include "AriaLogger.h"
#include <cstdio>
#include <LogManager.hpp>
#include <exception>
#include <thread>

using namespace std;

using namespace Microsoft::Applications::Events;

static ILogger *logger;

void AriaLogger::Init(std::string token)
{
    auto& configuration = LogManager::GetLogConfiguration();
    configuration[CFG_STR_CACHE_FILE_PATH] = "offlinestorage.db"; // ":memory:";
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Fatal;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 5;
    configuration[CFG_INT_RAM_QUEUE_SIZE]  = 32 * 1024 * 1024; // 32 MB heap limit for sqlite3
    configuration[CFG_INT_CACHE_FILE_SIZE] = 16 * 1024 * 1024; // 16 MB storage file limit

    printf("LogConfiguration:\n");
    printf("%s=%s\n", CFG_STR_CACHE_FILE_PATH, configuration.GetProperty(CFG_STR_CACHE_FILE_PATH));
    printf("%s=%x\n", CFG_INT_TRACE_LEVEL_MASK, configuration.GetIntProperty(CFG_INT_TRACE_LEVEL_MASK));
    printf("%s=%d\n", CFG_INT_TRACE_LEVEL_MIN, configuration.GetIntProperty(CFG_INT_TRACE_LEVEL_MIN));
    printf("%s=%d\n", CFG_INT_SDK_MODE, configuration.GetIntProperty(CFG_INT_SDK_MODE));
    printf("%s=%d\n", CFG_INT_MAX_TEARDOWN_TIME, configuration.GetIntProperty(CFG_INT_MAX_TEARDOWN_TIME));

    // TODO: move logger from static to private class member
    logger = LogManager::Initialize(token);
    // LogManager::SetTransmitProfile(TRANSMITPROFILE_REALTIME);
}

void AriaLogger::Pause()
{
    LogManager::PauseTransmission();
}

void AriaLogger::Resume()
{
    LogManager::ResumeTransmission();
}

void AriaLogger::LogEvent(std::map<std::string, std::string>& event)
{
    static long eventCount = 0;
    EventProperties props(event["name"]);
    for(auto &kv : event)
    {
        props.SetProperty(kv.first, kv.second);
    }
    logger->LogEvent(props);

    eventCount++;

#if 0 /* emulate async SIGABRT after 20000 events */
    if (eventCount > 20000) {
       std::thread t([]() {
           std::terminate();
       });
       t.detach();
    }
#endif

    // This is required to fix swig projection memleak :
    event.clear();
}

void AriaLogger::Upload()
{
    printf("LogManager::UploadNow\n");
    LogManager::UploadNow();
}

void AriaLogger::Done()
{
    printf("LogManager::FlushAndTeardown\n");
    LogManager::FlushAndTeardown();
}
