//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "Version.hpp"
#include "mat/config.h"

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace
{
    bool CheckVersion(const char* version)
    {
        const char prefix[] = EVTSDK_VERSION_PREFIX "-";
#ifdef HAVE_MAT_EXP
        const char suffix[] = "-C++-ECS-" BUILD_VERSION_STR;
#else
        const char suffix[] = "-C++-No-" BUILD_VERSION_STR;
#endif
        if (!version)
        {
            return false;
        }

        const auto size = std::strlen(version);
        return size >= sizeof(suffix) - 1 &&
               std::strncmp(version, prefix, sizeof(prefix) - 1) == 0 &&
               std::strcmp(version + size - (sizeof(suffix) - 1), suffix) == 0;
    }

    bool Reload(const char* path)
    {
#ifdef _WIN32
        const auto library = ::LoadLibraryA(path);
        const auto version = library
                                 ? reinterpret_cast<const char* (*)()>(::GetProcAddress(library, "GetSdkVersionForReload"))
                                 : nullptr;
#else
        auto* library = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        const auto version = library
                                 ? reinterpret_cast<const char* (*)()>(dlsym(library, "GetSdkVersionForReload"))
                                 : nullptr;
#endif
        if (!library)
        {
            std::fprintf(stderr, "Unable to load SDK fixture: %s\n", path);
            return false;
        }

        // Do not keep a pointer into an unloaded DSO or heap buffer on the stack.
        const bool valid = version && CheckVersion(version()) && CheckVersion(version());
#ifdef _WIN32
        const bool unloaded = ::FreeLibrary(library) && !::GetModuleHandleA(path);
#else
        const bool closed = dlclose(library) == 0;
        auto* remaining = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        const bool unloaded = closed && !remaining;
        if (remaining)
        {
            dlclose(remaining);
        }
#endif
        if (!valid || !unloaded)
        {
            std::fprintf(stderr, "SDK version validation or actual library unload failed\n");
        }

        return valid && unloaded;
    }
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return 2;
    }

    for (int cycle = 0; cycle != 2; ++cycle)
    {
        if (!Reload(argv[1]))
        {
            return 1;
        }
    }

    std::puts("PASS two SDK version unload/reload cycles");
    return 0;
}
