//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include <Windows.h>
#include <objbase.h>
#include <crtdbg.h>
#include <cstdio>
#include <cstring>

int main(int argc, char** argv)
{
    if (argc != 3 || (std::strcmp(argv[2], "native") != 0 &&
                     std::strcmp(argv[2], "disabled") != 0 &&
                     std::strcmp(argv[2], "legacy") != 0 &&
                     std::strcmp(argv[2], "legacy-no-cost") != 0 &&
                     std::strcmp(argv[2], "legacy-failures") != 0))
    {
        std::fprintf(stderr, "Usage: network-detector-reload-test <DLL> "
                             "<native|disabled|legacy|legacy-no-cost|legacy-failures>\n");
        return 1;
    }
    APTTYPE apartment;
    APTTYPEQUALIFIER qualifier;
    if (CoGetApartmentType(&apartment, &qualifier) != CO_E_NOTINITIALIZED)
    {
        std::fprintf(stderr, "The host must not initialize a COM apartment.\n");
        return 1;
    }
    for (unsigned iteration = 1; iteration <= 5; ++iteration)
    {
#ifdef _DEBUG
        _CrtMemState before {};
        _CrtMemCheckpoint(&before);
#endif
        std::printf("Loading SDK DLL, iteration %u\n", iteration);
        std::fflush(stdout);
        HMODULE module = LoadLibraryA(argv[1]);
        if (module == nullptr)
        {
            std::fprintf(stderr, "LoadLibrary failed: %lu\n", GetLastError());
            return 1;
        }
        auto hasDetector = reinterpret_cast<bool (*)()>(GetProcAddress(module, "HasNetworkDetector"));
        if (hasDetector != nullptr && !hasDetector())
        {
            std::printf("Network detection is disabled in this SDK SKU.\n");
            return FreeLibrary(module) ? 77 : 1;
        }
        const char* entry = "ExerciseNetworkDetector";
        if (std::strcmp(argv[2], "disabled") == 0)
        {
            entry = "ExerciseDisabledNetworkDetection";
        }
        else if (std::strcmp(argv[2], "legacy") == 0)
        {
            entry = "ExerciseLegacyNetworkDetector";
        }
        else if (std::strcmp(argv[2], "legacy-no-cost") == 0)
        {
            entry = "ExerciseLegacyNetworkDetectorWithoutCost";
        }
        else if (std::strcmp(argv[2], "legacy-failures") == 0)
        {
            entry = "ExerciseLegacyNetworkDetectorFailures";
        }
        auto exercise = reinterpret_cast<bool (*)()>(GetProcAddress(module, entry));
        if (exercise == nullptr || !exercise())
        {
            std::fprintf(stderr, "Network detector exercise failed on iteration %u.\n", iteration);
            FreeLibrary(module);
            return 1;
        }
        if (!FreeLibrary(module))
        {
            std::fprintf(stderr, "FreeLibrary failed: %lu\n", GetLastError());
            return 1;
        }
        if (GetModuleHandleA(argv[1]) != nullptr)
        {
            std::fprintf(stderr, "The SDK DLL is still loaded.\n");
            return 1;
        }
        if (CoGetApartmentType(&apartment, &qualifier) != CO_E_NOTINITIALIZED)
        {
            std::fprintf(stderr, "The SDK changed the host COM apartment.\n");
            return 1;
        }
#ifdef _DEBUG
        _CrtMemState after {};
        _CrtMemState difference {};
        _CrtMemCheckpoint(&after);
        _CrtMemDifference(&difference, &before, &after);
        const size_t blocks = difference.lCounts[_NORMAL_BLOCK] + difference.lCounts[_CLIENT_BLOCK];
        const size_t bytes = difference.lSizes[_NORMAL_BLOCK] + difference.lSizes[_CLIENT_BLOCK];
        std::printf("After unload: %zu blocks, %zu bytes\n", blocks, bytes);
        if (blocks != 0 || bytes != 0)
        {
            std::fprintf(stderr, "Reload iteration %u leaked %zu blocks / %zu bytes.\n", iteration, blocks, bytes);
            _CrtMemDumpAllObjectsSince(&before);
            return 1;
        }
#endif
        std::printf("SDK DLL unloaded, iteration %u\n", iteration);
        std::fflush(stdout);
    }
    return 0;
}
