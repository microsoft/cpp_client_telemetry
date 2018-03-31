// Copyright (c) Microsoft. All rights reserved.
#ifndef LIB_UTILS_HPP
#define LIB_UTILS_HPP

#include "Enums.hpp"
#include <Config.hpp>
#include "pal/PAL.hpp"

#include <chrono>
#include <algorithm>
#include <string>

#include "EventProperty.hpp"

/* Lean implementation of SLDC "Annex K" for non-Windows OS */
#include "annex_k.hpp"

#ifdef __unix__
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#endif

namespace ARIASDK_NS_BEGIN {

    ARIASDK_LOG_DECL_COMPONENT_NS();

    typedef std::chrono::milliseconds ms;

    /* Obtain a backtrace and print it to stdout. */
    void print_backtrace(void);

    inline void sleep(unsigned delayMs)
    {
        std::this_thread::sleep_for(ms(delayMs));
    }

    long		GetCurrentProcessId();

    std::string	GetTempDirectory();

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

    std::string to_string(GUID_t uuid);

    inline std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(), ::tolower);
        return result;
    }

    inline std::string toUpper(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(), ::toupper);
        return result;
    }

    inline std::string sanitizeIdentifier(std::string &str)
    {
        std::replace(str.begin(), str.end(), '.', '_');
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

        case EventPriority_Immediate:
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

} ARIASDK_NS_END

#endif
