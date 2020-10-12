//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
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

    /**
    * Return temporary directory on Win32 Desktop classic SKU
    * or AppData app-specific temporary directory
    */
    std::string GetAppLocalTempDirectory()
    {
#ifdef _WINRT_DLL // Win 10 UWP
        RoInitialize(RO_INIT_MULTITHREADED);
        ::Windows::Storage::StorageFolder^ temp = ::Windows::Storage::ApplicationData::Current->TemporaryFolder;
        // TODO: [MG]
        // - verify that the path ends with a slash
        // -- add exception handler in case if AppData temp folder is not accessible
        return from_platform_string(temp->Path->ToString());
#else
        return GetTempDirectory();
#endif
    }

    std::string GetTempDirectory()
    {
#ifdef _WIN32
        /* UTF-8 temp directory for Win32 Desktop apps */
        std::string path = "";
        wchar_t lpTempPathBuffer[MAX_PATH + 1] = { 0 };
        if (::GetTempPathW(MAX_PATH, lpTempPathBuffer))
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

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif
    /**
     * Convert various numeric types and bool to string in an uniform manner.
     */
    template<typename T>
    std::string to_string(char const* format, T value)
    {
        static const int buf_size = 40;
        char buf[buf_size] = { 0 };
#ifdef _WIN32
        ::_snprintf_s(buf, buf_size, format, value);
#else
        snprintf(buf, buf_size, format, value);
#endif
        return std::string(buf);
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    std::string toString(char const*        value) { return std::string(value); }
    std::string toString(bool               value) { return value ? "true" : "false"; }
    std::string toString(char               value) { return to_string("%d", static_cast<signed char>(value)); }
    std::string toString(int                value) { return to_string("%d", value); }
    std::string toString(long               value) { return to_string("%ld", value); }
    std::string toString(long long          value) { return to_string("%lld", value); }
    std::string toString(unsigned char      value) { return to_string("%u", value); }
    std::string toString(unsigned int       value) { return to_string("%u", value); }
    std::string toString(unsigned long      value) { return to_string("%lu", value); }
    std::string toString(unsigned long long value) { return to_string("%llu", value); }
    std::string toString(float              value) { return to_string("%f", value); }
    std::string toString(double             value) { return to_string("%f", value); }
    std::string toString(long double        value) { return to_string("%Lf", value); }

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

    std::string to_string(const GUID_t& uuid)
    {
        static char inttoHex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
        const unsigned buffSize = 36 + 1;  // 36 + null-termination
        char buf[buffSize] = { 0 };

        int  test = (uuid.Data1 >> 28 & 0x0000000F);
        buf[0] = inttoHex[test];
        test = (int)(uuid.Data1 >> 24 & 0x0000000F);
        buf[1] = inttoHex[test];
        test = (int)(uuid.Data1 >> 20 & 0x0000000F);
        buf[2] = inttoHex[test];
        test = (int)(uuid.Data1 >> 16 & 0x0000000F);
        buf[3] = inttoHex[test];
        test = (int)(uuid.Data1 >> 12 & 0x0000000F);
        buf[4] = inttoHex[test];
        test = (int)(uuid.Data1 >> 8 & 0x0000000F);
        buf[5] = inttoHex[test];
        test = (int)(uuid.Data1 >> 4 & 0x0000000F);
        buf[6] = inttoHex[test];
        test = (int)(uuid.Data1 & 0x0000000F);
        buf[7] = inttoHex[test];
        buf[8] = '-';
        test = (int)(uuid.Data2 >> 12 & 0x000F);
        buf[9] = inttoHex[test];
        test = (int)(uuid.Data2 >> 8 & 0x000F);
        buf[10] = inttoHex[test];
        test = (int)(uuid.Data2 >> 4 & 0x000F);
        buf[11] = inttoHex[test];
        test = (int)(uuid.Data2 & 0x000F);
        buf[12] = inttoHex[test];
        buf[13] = '-';
        test = (int)(uuid.Data3 >> 12 & 0x000F);
        buf[14] = inttoHex[test];
        test = (int)(uuid.Data3 >> 8 & 0x000F);
        buf[15] = inttoHex[test];
        test = (int)(uuid.Data3 >> 4 & 0x000F);
        buf[16] = inttoHex[test];
        test = (int)(uuid.Data3 & 0x000F);
        buf[17] = inttoHex[test];
        buf[18] = '-';
        test = (int)(uuid.Data4[0] >> 4 & 0x0F);
        buf[19] = inttoHex[test];
        test = (int)(uuid.Data4[0] & 0x0F);
        buf[20] = inttoHex[test];
        test = (int)(uuid.Data4[1] >> 4 & 0x0F);
        buf[21] = inttoHex[test];
        test = (int)(uuid.Data4[1] & 0x0F);
        buf[22] = inttoHex[test];
        buf[23] = '-';
        test = (int)(uuid.Data4[2] >> 4 & 0x0F);
        buf[24] = inttoHex[test];
        test = (int)(uuid.Data4[2] & 0x0F);
        buf[25] = inttoHex[test];
        test = (int)(uuid.Data4[3] >> 4 & 0x0F);
        buf[26] = inttoHex[test];
        test = (int)(uuid.Data4[3] & 0x0F);
        buf[27] = inttoHex[test];
        test = (int)(uuid.Data4[4] >> 4 & 0x0F);
        buf[28] = inttoHex[test];
        test = (int)(uuid.Data4[4] & 0x0F);
        buf[29] = inttoHex[test];
        test = (int)(uuid.Data4[5] >> 4 & 0x0F);
        buf[30] = inttoHex[test];
        test = (int)(uuid.Data4[5] & 0x0F);
        buf[31] = inttoHex[test];
        test = (int)(uuid.Data4[6] >> 4 & 0x0F);
        buf[32] = inttoHex[test];
        test = (int)(uuid.Data4[6] & 0x0F);
        buf[33] = inttoHex[test];
        test = (int)(uuid.Data4[7] >> 4 & 0x0F);
        buf[34] = inttoHex[test];
        test = (int)(uuid.Data4[7] & 0x0F);
        buf[35] = inttoHex[test];
        buf[36] = 0;
        return std::string(buf);
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

