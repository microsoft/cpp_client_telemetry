// Copyright (c) Microsoft. All rights reserved.
#ifndef LIB_UTILS_HPP
#define LIB_UTILS_HPP

// This file is being used during both C++ native and C++/CX managed compilation with /cli flag
// Certain features, e.g. <mutex> or <thread> - cannot be used while building with /cli
// For this reason this header cannot include any other headers that rely on <mutex> or <thread>

#include "Version.hpp"
#include "Enums.hpp"
#include "StringConversion.hpp"

#include <chrono>
#include <algorithm>
#include <string>
#include <cstdio>

#include "EventProperty.hpp"

/* Lean implementation of SLDC "Annex K" for non-Windows OS */
#include "annex_k.hpp"

#if (__cplusplus < 201402L) && !defined(_MSC_VER)
/* Workaround for lack of std::make_unique in C++11 (gcc-5). N3936 for C++14 support mentions 201402L */
#include <memory>
namespace std
{
    template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) { return std::unique_ptr<T>(new T(std::forward<Args>(args)...)); }
}
#endif

#ifdef __unix__
#ifndef ANDROID
#include <execinfo.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#endif

#if defined(WINDOWS_UWP) || defined(__cplusplus_winrt)
#include <Windows.h>
#define _WINRT
#endif

#ifndef EOK
#define EOK 0
#endif

#ifndef EFAIL
#define EFAIL   -1
#endif

namespace MAT_NS_BEGIN {

    const char* getMATSDKLogComponent();

    typedef std::chrono::milliseconds ms;

    /* Obtain a backtrace and print it to stdout. */
    void print_backtrace(void);

    void sleep(unsigned delayMs);

    long GetCurrentProcessId();

    std::string GetTempDirectory();
    std::string GetAppLocalTempDirectory();

    std::string toString(char const*        value);
    std::string toString(bool               value);
    std::string toString(char               value);
    std::string toString(int                value);
    std::string toString(long               value);
    std::string toString(long long          value);
    std::string toString(unsigned char      value);
    std::string toString(unsigned int       value);
    std::string toString(unsigned long      value);
    std::string toString(unsigned long long value);
    std::string toString(float              value);
    std::string toString(double             value);
    std::string toString(long double        value);

    std::string to_string(const GUID_t& uuid);

    inline std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(), 
                   [](unsigned char c){ return (char)::tolower(c); }
                );
        return result;
    }

    inline std::string toUpper(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(), 
                   [](unsigned char c){ return (char)::toupper(c); }
                );
        return result;
    }

    inline bool equalsIgnoreCase(const std::string& str1, const std::string& str2)
    {
        return (str1.size() == str2.size()) && (toLower(str1) == toLower(str2));
    }

    inline std::string sanitizeIdentifier(const std::string &str)
    {
#if 0
        // TODO: [MG] - we have to add some sanitizing logic, but definitely NOT replacing dots by underscores
        std::replace(str.begin(), str.end(), '.', '_');
#endif
        return str;
    }

    EventRejectedReason validateEventName(std::string const& name);

    EventRejectedReason validatePropertyName(std::string const& name);

    inline std::string tenantTokenToId(std::string const& tenantToken)
    {
        return tenantToken.substr(0, tenantToken.find('-'));
    }

    inline const char* priorityToStr(EventPriority priority)
    {
        switch (priority) {
        case EventPriority_Unspecified:
            return "Unspecified";

        case EventPriority_Off:
            return "Off";

        case EventPriority_Low:
            return "Low";

        case EventPriority_Normal:
            return "Normal";

        case EventPriority_High:
            return "High";

        case EventPriority_Immediate:
            return "Immediate";

        default:
            return "???";
        }
    }

    inline const char* latencyToStr(EventLatency latency)
    {
        switch (latency) {
        case EventLatency_Unspecified:
            return "Unspecified";

        case EventLatency_Off:
            return "Off";

        case EventLatency_Normal:
            return "Normal";

        case EventLatency_CostDeferred:
            return "CostDeferred";

        case EventLatency_RealTime:
            return "RealTime";

        case EventLatency_Max:
            return "Immediate";

        default:
            return "???";
        }
    }

    inline bool replace(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    inline uint64_t GetUptimeMs()
    {
        return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    }

#ifdef _WINRT

    Platform::String ^to_platform_string(const std::string& s);

    std::string from_platform_string(Platform::String ^ ps);

#endif

    unsigned hashCode(const char* str, int h = 0);

} MAT_NS_END

#endif
