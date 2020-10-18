//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "EventLogger.h"

#include "LogManager.hpp"

#include <cstdio>
#include <exception>
#include <thread>

using namespace std;
using namespace MAT;

LOGMANAGER_INSTANCE

static ILogger *logger;

void EventLogger::Init(std::string token)
{
    auto& configuration = LogManager::GetLogConfiguration();
    configuration[CFG_STR_CACHE_FILE_PATH] = "offlinestorage.db"; // ":memory:";
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Fatal;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_CS;
    configuration[CFG_INT_MAX_TEARDOWN_TIME] = 5;
    configuration[CFG_INT_RAM_QUEUE_SIZE]  = 32 * 1024 * 1024; // 32 MB heap limit for sqlite3
    configuration[CFG_INT_CACHE_FILE_SIZE] = 16 * 1024 * 1024; // 16 MB storage file limit

    // TODO: move logger from static to private class member
    logger = LogManager::Initialize(token);

    // LogManager::SetTransmitProfile(TRANSMITPROFILE_REALTIME);
}

void EventLogger::Pause()
{
    LogManager::PauseTransmission();
}

void EventLogger::Resume()
{
    LogManager::ResumeTransmission();
}

void EventLogger::LogEvent(std::map<std::string, std::string>& event)
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

void EventLogger::Upload()
{
    printf("LogManager::UploadNow\n");
    LogManager::UploadNow();
}

void EventLogger::Done()
{
    printf("LogManager::FlushAndTeardown\n");
    LogManager::FlushAndTeardown();
}
