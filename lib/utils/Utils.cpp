// Copyright (c) Microsoft. All rights reserved.
#include <pal/PAL.hpp>

#include "Utils.hpp"

#include <Config.hpp>

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
#include <fstream>
#include <streambuf>

#include <thread>

#include <iostream>
#include <locale>
#include <codecvt>

namespace ARIASDK_NS_BEGIN {
    ARIASDK_LOG_INST_COMPONENT_NS("AriaSDK", "Aria telemetry client");
} ARIASDK_NS_END

namespace ARIASDK_NS_BEGIN {

    void sleep(unsigned delayMs)
    {
        std::this_thread::sleep_for(ms(delayMs));
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
        if (result.empty())
        {
            result = _PATH_TMP;
        }
        if (result.empty())
        {
            result = "/tmp";
        }
        result += "/";
        return result;
#endif
    }

    std::string toString(char const*        value) { return std::string(value); }
    std::string toString(bool               value) { return value ? "true" : "false"; }
    std::string toString(char               value) { return PAL::to_string("%d", static_cast<signed char>(value)); }
    std::string toString(int                value) { return PAL::to_string("%d", value); }
    std::string toString(long               value) { return PAL::to_string("%ld", value); }
    std::string toString(long long          value) { return PAL::to_string("%lld", value); }
    std::string toString(unsigned char      value) { return PAL::to_string("%u", value); }
    std::string toString(unsigned int       value) { return PAL::to_string("%u", value); }
    std::string toString(unsigned long      value) { return PAL::to_string("%lu", value); }
    std::string toString(unsigned long long value) { return PAL::to_string("%llu", value); }
    std::string toString(float              value) { return PAL::to_string("%f", value); }
    std::string toString(double             value) { return PAL::to_string("%f", value); }
    std::string toString(long double        value) { return PAL::to_string("%Lf", value); }

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

#if ARIASDK_PAL_SKYPE
        // Allow also ':' and '-' for Skype. Those are unfortunately used by
        // someone (it was not an error before) and changing that needs time.
        auto filter = [](char ch) -> bool { return !isalnum(static_cast<uint8_t>(ch)) && (ch != '_') && (ch != '.') && (ch != '-') && (ch != ':'); };
#else
        auto filter = [](char ch) -> bool { return !isalnum(static_cast<uint8_t>(ch)) && (ch != '_') && (ch != '.'); };
#endif

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

    std::string to_string(GUID_t uuid)
    {
        static char inttoHex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
        const unsigned buffSize = 36 + 1;  // 36 + null-termination
        char buf[buffSize];

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

    /** \brief Convert UTF-8 to UTF-16
    */
    std::wstring to_utf16_string(const std::string& in)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
        return converter.from_bytes(in);
    }

    /** \brief Convert UTF-16 to UTF-8
    */
    std::string to_utf8_string(const std::wstring& in)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
        return converter.to_bytes(in);
    }

    /**
     * Read file contents into std::string.
     *
     * TODO: This function is in use by Linux SDK only.
     *
     * It needs an improvement to use wide strings for UTF-8
     * support on Windows 7. Latest Windows 10 RS4 should
     * natively support UTF-8.
     *
     * @param       flename
     * @return      File contents
     */
    std::string ReadFile(const char *filename)
    {
        std::ifstream t(filename);
        std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
        return str;
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
        return std::string(wcontent.begin(), wcontent.end());
    }
#endif

} ARIASDK_NS_END
