//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "api/AuthTokensController.hpp"
#include "pal/PAL.hpp"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>

namespace
{
    std::atomic<bool> versionDestroyed{false};
    std::atomic<bool> mutexDestroyed{false};
    bool checkVersion = false;
    bool checkMutex = false;
    const std::string* versionAddress = nullptr;
    std::mutex* mutexAddress = nullptr;

    void Fail(const char* message)
    {
        std::fprintf(stderr, "%s\n", message);
#ifdef _WIN32
        // The exit owner holds the CRT exit lock while waiting for this worker.
        ::TerminateProcess(::GetCurrentProcess(), 1);
#endif
        std::_Exit(1);
    }

    void ReadState()
    {
        if (checkVersion)
        {
            if (versionDestroyed.load())
            {
                Fail("SDK version was destroyed before its exit owner");
            }
            const auto& version = PAL::GetPAL().getSdkVersion();
            if (version.empty() || (versionAddress && versionAddress != &version))
            {
                Fail("SDK version did not retain its value and identity");
            }
            versionAddress = &version;
        }
        if (checkMutex)
        {
            if (mutexDestroyed.load())
            {
                Fail("Auth mutex was destroyed before its exit owner");
            }
            auto& mutex = MAT::AuthTokensController::GetMutex();
            if (mutexAddress && mutexAddress != &mutex)
            {
                Fail("Auth mutex did not retain its identity");
            }
            std::lock_guard<std::mutex> lock(mutex);
            mutexAddress = &mutex;
        }
    }

    void ExitOwner()
    {
        // Model an owner registered before its first upload initializes SDK state.
        std::thread lateWorker(ReadState);
        lateWorker.join();
    }
}

extern "C" void LifetimeVersionDestroyed()
{
    versionDestroyed.store(true);
}

extern "C" void LifetimeMutexDestroyed()
{
    mutexDestroyed.store(true);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 2;
    }
    checkVersion = std::strcmp(argv[1], "version") == 0 || std::strcmp(argv[1], "both") == 0;
    checkMutex = std::strcmp(argv[1], "mutex") == 0 || std::strcmp(argv[1], "both") == 0;
    if ((!checkVersion && !checkMutex) || std::atexit(ExitOwner) != 0)
    {
        return 2;
    }
    ReadState();
    ReadState();
    return 0;
}
