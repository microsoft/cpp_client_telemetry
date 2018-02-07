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

std::string toString(GUID_t uuid);

std::string toLower(std::string str);
std::string toUpper(std::string str);
std::string sanitizeIdentifier(std::string buff);
EventRejectedReason validateEventName(std::string const& name);
EventRejectedReason validatePropertyName(std::string const& name);

inline std::string tenantTokenToId(std::string const& tenantToken)
{
    return tenantToken.substr(0, tenantToken.find('-'));
}

inline const char* latencyToStr(EventLatency priority)
{
    switch (priority) {
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


} ARIASDK_NS_END
#endif