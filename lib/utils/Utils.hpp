// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "pal/PAL.hpp"
#include "include\public\Enums.hpp"
#include <Config.hpp>


namespace ARIASDK_NS_BEGIN {

ARIASDK_LOG_DECL_COMPONENT_NS();

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

std::string toLower(std::string str);
std::string sanitizeIdentifier(std::string buff);
bool validateEventName(std::string const& name);
bool validatePropertyName(std::string const& name);

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


} ARIASDK_NS_END
