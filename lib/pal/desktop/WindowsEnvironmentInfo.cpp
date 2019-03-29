// Copyright (c) Microsoft. All rights reserved.
#include "Version.hpp"
#include "WindowsEnvironmentInfo.h"
#include <Windows.h>

namespace ARIASDK_NS_BEGIN
{
    // Convert a bias in minutes to the ISO 8601 time zone representaiton.
    // ISO 8601 examples: +01:30, -08
    std::string TimeZoneBiasToISO8601(long bias)
    {
        auto hours = (long long)abs(bias) / 60;
        auto minutes = (long long)abs(bias) % 60;

        // UTC = local time + bias; bias sign should be interved.
        return std::string(bias <= 0 ? "+" : "-") + (hours >= 10 ? "" : "0") + std::to_string(hours) + ":"
            + (minutes >= 10 ? "" : "0") + std::to_string(minutes);
    }

    // Retrieves the processor architecture.
    OsArchitectureType WindowsEnvironmentInfo::GetProcessorArchitecture()
    {
        _SYSTEM_INFO sysinfo = {};
        GetNativeSystemInfo(&sysinfo);

        switch (sysinfo.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_AMD64:
        case PROCESSOR_ARCHITECTURE_IA64:
            return OsArchitectureType_X64;

        case PROCESSOR_ARCHITECTURE_ARM:
            return OsArchitectureType_Arm;

        case PROCESSOR_ARCHITECTURE_INTEL:
            return OsArchitectureType_X86;

        default:
            return OsArchitectureType_Unknown;
        }
    }

    std::string WindowsEnvironmentInfo::GetTimeZone()
    {
        TIME_ZONE_INFORMATION timeZone = {};
        if (GetTimeZoneInformation(&timeZone) == TIME_ZONE_ID_DAYLIGHT)
        {
            return TimeZoneBiasToISO8601(timeZone.Bias + timeZone.DaylightBias);
        }
        else
        {
            // TODO: [MG] - fix this benign compiler warning
            // Warning	C6102	Using 'timeZone' from failed function call at line '46'
            return TimeZoneBiasToISO8601(timeZone.Bias + timeZone.StandardBias);
        }
    }

} ARIASDK_NS_END
