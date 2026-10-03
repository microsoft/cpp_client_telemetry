//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include <Windows.h>
#include <crtdbg.h>
#include <cstdio>
#include <cstring>

#if defined(_DEBUG) && !defined(_DLL)
#error The unload regression requires the shared Debug CRT.
#endif

namespace
{
    using Exercise = bool (*)();
    constexpr unsigned threadCount = 7;

    struct Worker
    {
        HANDLE ready = nullptr;
        HANDLE exit = nullptr;
        Exercise exercise = nullptr;
        bool succeeded = false;
    };

    DWORD WINAPI RunWorker(void* context)
    {
        auto& worker = *static_cast<Worker*>(context);
        worker.succeeded = worker.exercise == nullptr || worker.exercise();
        SetEvent(worker.ready);
        return WaitForSingleObject(worker.exit, INFINITE) == WAIT_OBJECT_0 ? 0 : 1;
    }
}

int main(int argc, char** argv)
{
    if (argc != 3 || (std::strcmp(argv[2], "idle") != 0 &&
                     std::strcmp(argv[2], "dispatch") != 0 &&
                     std::strcmp(argv[2], "network-native") != 0 &&
                     std::strcmp(argv[2], "network-legacy") != 0 &&
                     std::strcmp(argv[2], "network-legacy-no-cost") != 0 &&
                     std::strcmp(argv[2], "network-legacy-failures") != 0))
    {
        std::fprintf(stderr, "Usage: debug-listener-unload-test <DLL> "
                             "<idle|dispatch|network-native|network-legacy|network-legacy-no-cost|network-legacy-failures>\n");
        return 1;
    }
#ifndef _DEBUG
    std::fprintf(stderr, "This regression requires the Debug CRT and Debug STL.\n");
    return 1;
#else
    std::printf("Debug SDK unload: %s, %u live threads\n", argv[2], threadCount);
    _CrtMemState before {};
    _CrtMemState after {};
    _CrtMemState difference {};
    _CrtMemCheckpoint(&before);

    HMODULE module = LoadLibraryA(argv[1]);
    if (module == nullptr)
    {
        std::fprintf(stderr, "LoadLibrary failed: %lu\n", GetLastError());
        return 1;
    }
    const char* entry = "ExerciseDebugListeners";
    if (std::strcmp(argv[2], "network-native") == 0)
    {
        entry = "ExerciseNetworkDetector";
    }
    else if (std::strcmp(argv[2], "network-legacy") == 0)
    {
        entry = "ExerciseLegacyNetworkDetector";
    }
    else if (std::strcmp(argv[2], "network-legacy-no-cost") == 0)
    {
        entry = "ExerciseLegacyNetworkDetectorWithoutCost";
    }
    else if (std::strcmp(argv[2], "network-legacy-failures") == 0)
    {
        entry = "ExerciseLegacyNetworkDetectorFailures";
    }
    if (std::strncmp(argv[2], "network-", 8) == 0)
    {
        auto hasDetector = reinterpret_cast<Exercise>(GetProcAddress(module, "HasNetworkDetector"));
        if (hasDetector != nullptr && !hasDetector())
        {
            std::printf("Network detection is disabled in this SDK SKU.\n");
            return FreeLibrary(module) ? 77 : 1;
        }
    }
    auto exercise = reinterpret_cast<Exercise>(GetProcAddress(module, entry));
    if (exercise == nullptr)
    {
        std::fprintf(stderr, "GetProcAddress failed: %lu\n", GetLastError());
        FreeLibrary(module);
        return 1;
    }

    Worker workers[threadCount];
    HANDLE threads[threadCount] {};
    HANDLE exit = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    bool succeeded = exit != nullptr;
    for (unsigned i = 0; succeeded && i < threadCount; ++i)
    {
        workers[i].exit = exit;
        workers[i].ready = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        workers[i].exercise = std::strcmp(argv[2], "idle") != 0 ? exercise : nullptr;
        if (workers[i].ready == nullptr)
        {
            succeeded = false;
            break;
        }
        threads[i] = CreateThread(nullptr, 0, RunWorker, &workers[i], 0, nullptr);
        succeeded = threads[i] != nullptr &&
                    WaitForSingleObject(workers[i].ready, 10000) == WAIT_OBJECT_0 &&
                    workers[i].succeeded;
    }

    if (succeeded)
    {
        succeeded = FreeLibrary(module) != FALSE;
        if (succeeded)
        {
            module = nullptr;
        }
        else
        {
            std::fprintf(stderr, "FreeLibrary failed: %lu\n", GetLastError());
        }
        if (GetModuleHandleA(argv[1]) != nullptr)
        {
            std::fprintf(stderr, "The SDK DLL is still loaded.\n");
            succeeded = false;
        }
        for (auto thread : threads)
        {
            DWORD code = 0;
            succeeded = GetExitCodeThread(thread, &code) != FALSE &&
                        code == STILL_ACTIVE && succeeded;
        }
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        const size_t blocks = difference.lCounts[_NORMAL_BLOCK] + difference.lCounts[_CLIENT_BLOCK];
        const size_t bytes = difference.lSizes[_NORMAL_BLOCK] + difference.lSizes[_CLIENT_BLOCK];
        std::printf("After unload, seven threads still alive: %zu blocks, %zu bytes\n", blocks, bytes);
        succeeded = succeeded && blocks == 0 && bytes == 0;
        if (blocks != 0 || bytes != 0)
        {
            _CrtMemDumpAllObjectsSince(&before);
        }
    }
    else
    {
        std::fprintf(stderr, "Worker setup/exercise failed: %lu\n", GetLastError());
    }

    if (exit != nullptr)
    {
        SetEvent(exit);
    }
    for (unsigned i = 0; i < threadCount; ++i)
    {
        if (threads[i] != nullptr)
        {
            succeeded = WaitForSingleObject(threads[i], INFINITE) == WAIT_OBJECT_0 && succeeded;
            CloseHandle(threads[i]);
        }
        if (workers[i].ready != nullptr)
        {
            CloseHandle(workers[i].ready);
        }
    }
    if (exit != nullptr)
    {
        CloseHandle(exit);
    }
    if (module != nullptr)
    {
        FreeLibrary(module);
    }
    return succeeded ? 0 : 1;
#endif
}
