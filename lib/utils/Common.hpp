// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "pal/PAL.hpp"
#include <aria/Enums.hpp>
#include <Aria/Config.hpp>


namespace ARIASDK_NS_BEGIN {


ARIASDK_LOG_DECL_COMPONENT_NS();


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
