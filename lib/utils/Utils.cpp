//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "pal/PAL.hpp"

#include "Utils.hpp"
#ifdef ANDROID
#include "http/HttpClient_Android.hpp"
#endif

#include <algorithm>
#include <string>

#ifdef _WIN32
#include <Windows.h>

#ifdef _WINRT_DLL
// Win 10-specific APIs
#include <Roapi.h>
#endif

#else
#include <paths.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <string>

#include <thread>

#include <iostream>
#include <locale>
#include <codecvt>

namespace MAT_NS_BEGIN {
    MATSDK_LOG_INST_COMPONENT_NS("MATSDK", "MS App Telemetry client");
} MAT_NS_END

namespace MAT_NS_BEGIN {

    void sleep(unsigned delayMs)
    {
        PAL::sleep(delayMs);
    }

    void print_backtrace(void)
    {
#ifdef USE_BACKTRACE
        // Debug builds only
#ifdef __unix__
        void *array[10] = { 0 };
        size_t size = 0;
        char **strings;
        size_t i;
        size = backtrace(array, sizeof(array) / sizeof(array[0]));
        strings = backtrace_symbols(array, size);
        printf("XXXXXXXXXXXXXXXXXXXX Obtained %zd stack frames:\n", size);
        for (i = 0; i < size; i++)
            printf("[%2lu] %s\n", i, demangle(strings[i]).c_str());
        printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
        free(strings);
#endif
#endif
    }

    long GetCurrentProcessId()
    {
#ifndef _WIN32
        return getpid();
#else
        return ::GetCurrentProcessId();
#endif
    }

    bool IsRunningInApp()
    {
#ifdef _WINRT_DLL  // Win 10 UWP
        typedef LONG (*LPFN_GPFN)(UINT32*, PWSTR);
        bool isRunningInApp = true;

        LPFN_GPFN lpGetPackageFamilyName = (LPFN_GPFN)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetCurrentPackageFamilyName");
        if (lpGetPackageFamilyName)
        {
            UINT32 size = 0;
            if (lpGetPackageFamilyName(&size, NULL) == APPMODEL_ERROR_NO_PACKAGE)
                isRunningInApp = false;
        }
        return isRunningInApp;
#else
        return false;
#endif
    }

    /**
    * Return temporary directory on Win32 Desktop classic SKU
    * or AppData app-specific temporary directory
    */
    std::string GetAppLocalTempDirectory()
    {
#ifdef _WINRT_DLL // Win 10 UWP
        if (IsRunningInApp())
        {
            auto hr = RoInitialize(RO_INIT_MULTITHREADED);
            /* Ignoring result from call to `RoInitialize` as either initialzation is successful, or else already
             * initialized and it should be ok to proceed in both the scenarios */
            UNREFERENCED_PARAMETER(hr);

            ::Windows::Storage::StorageFolder ^ temp = ::Windows::Storage::ApplicationData::Current->TemporaryFolder;
            // TODO: [MG]
            // - verify that the path ends with a slash
            // -- add exception handler in case if AppData temp folder is not accessible
            return from_platform_string(temp->Path->ToString());
        }
        else
        {
            return GetTempDirectory();
        }
#else
        return GetTempDirectory();
#endif
    }

    std::string GetTempDirectory()
    {
#ifdef _WIN32
        auto lpGetTempPathW = reinterpret_cast<decltype(&::GetTempPathW)>(GetProcAddress(GetModuleHandle(TEXT("kernel32")), "GetTempPath2W"));
        if (lpGetTempPathW == NULL)
        {
            lpGetTempPathW = ::GetTempPathW;
        }
        /* UTF-8 temp directory for Win32 Desktop apps */
        std::string path = "";
        wchar_t lpTempPathBuffer[MAX_PATH + 1] = { 0 };
        if (lpGetTempPathW(MAX_PATH, lpTempPathBuffer))
        {
            path = to_utf8_string(lpTempPathBuffer);
        }
        return path;
#else
        std::string result;
#if defined(ANDROID) && defined(HAVE_MAT_CURL_HTTP_CLIENT)
        result = "/data/local/tmp";
#elif ANDROID
        result = HttpClient_Android::GetCacheFilePath();
        if (result.empty())
        {
           result = "/data/local/tmp";
        }
#else
        char *tmp = getenv("TMPDIR");
        if (tmp != NULL) {
            result = tmp;
        }
        else {
#ifdef P_tmpdir
            if (P_tmpdir != NULL)
                result = P_tmpdir;
#endif
        }
#ifdef _PATH_TMP
        if (result.empty())
        {
            result = _PATH_TMP;
        }
#endif
        if (result.empty())
        {
            result = "/tmp";
        }
#endif
        result += "/";
        return result;
#endif
    }

    EventRejectedReason validateEventName(std::string const& name)
    {
        // Data collector uses this regex (avoided here for code size reasons):
        // ^[a-zA-Z0-9]([a-zA-Z0-9]|_){2,98}[a-zA-Z0-9]$

        if (name.length() < 1 + 2 + 1 || name.length() > 1 + 98 + 1) {
            LOG_ERROR("Invalid event name - \"%s\": must be between 4 and 100 characters long", name.c_str());
            return REJECTED_REASON_VALIDATION_FAILED;
        }

        auto filter = [](char ch) -> bool { return !isalnum(static_cast<uint8_t>(ch)) && (ch != '_') && (ch != '.'); };
        if (std::find_if(name.begin(), name.end(), filter) != name.end()) {
            LOG_ERROR("Invalid event name - \"%s\": must contain [0-9A-Za-z_] characters only", name.c_str());
            return REJECTED_REASON_VALIDATION_FAILED;
        }

#if 0
        if (name.front() == '_' || name.back() == '_') {
            LOG_ERROR("Invalid event name - \"%s\": must not start or end with an underscore", name.c_str());
            return REJECTED_REASON_VALIDATION_FAILED;
        }
#endif

        return REJECTED_REASON_OK;
    }

    EventRejectedReason validatePropertyName(std::string const& name)
    {
        // Data collector does not seem to validate property names at all.
        // The ObjC SDK uses this regex (avoided here for code size reasons):
        // ^[a-zA-Z0-9](([a-zA-Z0-9|_|.]){0,98}[a-zA-Z0-9])?$

        if (name.length() < 1 + 0 || name.length() > 1 + 98 + 1) {
            LOG_ERROR("Invalid property name - \"%s\": must be between 1 and 100 characters long", name.c_str());
            return REJECTED_REASON_VALIDATION_FAILED;
        }

        auto filter = [](char ch) -> bool { return !isalnum(static_cast<uint8_t>(ch)) && (ch != '_') && (ch != '.'); };

        if (std::find_if(name.begin(), name.end(), filter) != name.end()) {
            LOG_ERROR("Invalid property name - \"%s\": must contain [0-9A-Za-z_.] characters only", name.c_str());
            return REJECTED_REASON_VALIDATION_FAILED;
        }

        if ((name.front() == '.' || name.back() == '.') /* || (name.front() == '_' || name.back() == '_') */)
        {
            LOG_ERROR("Invalid property name - \"%s\": must not start or end with _ or . characters", name.c_str());
            return REJECTED_REASON_VALIDATION_FAILED;
        }
        return REJECTED_REASON_OK;
    }

#ifdef _WINRT
    Platform::String ^to_platform_string(const std::string& s)
    {
        std::wstring wcontent(s.begin(), s.end());
        return ref new Platform::String(wcontent.data());
    }

    std::string from_platform_string(Platform::String ^ ps)
    {
        std::wstring wcontent(ps->Data());
        return to_utf8_string(wcontent);
    }
#endif

    unsigned hashCode(const char* str, int h)
    {
        return (unsigned)(!str[h] ? 5381 : ((unsigned long long)hashCode(str, h + 1) * (unsigned)33) ^ str[h]);
    }

} MAT_NS_END

