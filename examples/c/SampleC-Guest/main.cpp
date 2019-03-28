#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <chrono>
#include <thread>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <future>
#include <cassert>

#include "LogManager.hpp"

using namespace MAT;

#ifdef _WIN32
#include <Windows.h>
#endif

LOGMANAGER_INSTANCE

#define TOKEN   "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322"

extern "C" void test_c_api_guest();

int main()
{
    printf("Setting up configuration...\n");
    auto& config = LogManager::GetLogConfiguration();
    config["name"] = "HostModule";
    config["version"] = "1.2.5";
    config["config"]["host"] = "HostModule"; // host
    config["compat"]["dotType"] = false;    // Legacy v1 behaviour with respect to SetType using underscore instead of a dot
    config[CFG_INT_MAX_TEARDOWN_TIME] = 10;

    printf("LogManager::Initialize\n");
    ILogger *logger = LogManager::Initialize(TOKEN);

    printf("Sending host event...\n");
    logger->LogEvent("TestApp.Event.Host");

    printf("Initializing guest and sending some events...\n");
    test_c_api_guest();

    printf("Sending host event after guest shutdown...\n");
    logger->LogEvent("TestApp.Event.Host.AfterGuestShutdown");
    LogManager::UploadNow();
    LogManager::FlushAndTeardown();

    return 0;
}
