//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef WINDOWSENVIRONMENTINFO_HPP
#define WINDOWSENVIRONMENTINFO_HPP

#include "Enums.hpp"
#include <string>
#include <stdint.h>

namespace MAT_NS_BEGIN {

    class WindowsEnvironmentInfo
    {
    public:
        static OsArchitectureType GetProcessorArchitecture()
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
        };

        static std::string GetTimeZone()
        {
            TIME_ZONE_INFORMATION timeZone = {};
            if (GetTimeZoneInformation(&timeZone) == TIME_ZONE_ID_DAYLIGHT)
            {
                return TimeZoneBiasToISO8601(timeZone.Bias + timeZone.DaylightBias);
            }
            else
            {
                // TODO: [MG] - ref. https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-gettimezoneinformation
                // Need to handle the case when API return TIME_ZONE_ID_UNKNOWN. Otherwise we may be reporting invalid timeZone.Bias
                return TimeZoneBiasToISO8601(timeZone.Bias + timeZone.StandardBias);
            }
        };
    protected:
        // Convert a bias in minutes to the ISO 8601 time zone representations.
        // ISO 8601 examples: +01:30, -08
        static std::string TimeZoneBiasToISO8601(long bias)
        {
            auto hours = (long long)abs(bias) / 60;
            auto minutes = (long long)abs(bias) % 60;

            // UTC = local time + bias; bias sign should be interved.
            return std::string(bias <= 0 ? "+" : "-") + (hours >= 10 ? "" : "0") + std::to_string(hours) + ":"
                + (minutes >= 10 ? "" : "0") + std::to_string(minutes);
        }
    };

} MAT_NS_END
#endif

