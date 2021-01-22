//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include "Version.hpp"
#include "ctmacros.hpp"
#include <EventProperty.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace MAT_NS_BEGIN
{
    class StringUtils
    {
    public:

        static void SplitString(const std::string& s, const char separator, std::vector<std::string>& parts);
        static bool AreAllCharactersWhitelisted(const std::string& stringToTest, const std::string& whitelist);

    private:

        StringUtils(const StringUtils&) = delete;
        StringUtils& operator=(const StringUtils&) = delete;
    };

    std::string toString(char const* value);
    std::string toString(bool value);
    std::string toString(char value);
    std::string toString(int value);
    std::string toString(long value);
    std::string toString(long long value);
    std::string toString(unsigned char value);
    std::string toString(unsigned int value);
    std::string toString(unsigned long value);
    std::string toString(unsigned long long value);
    std::string toString(float value);
    std::string toString(double value);
    std::string toString(long double value);

    std::string to_string(const GUID_t& uuid);

    inline std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(),
                       [](unsigned char c) { return (char)::tolower(c); });
        return result;
    }

    inline std::string toUpper(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(),
                       [](unsigned char c) { return (char)::toupper(c); });
        return result;
    }

    inline bool equalsIgnoreCase(const std::string& str1, const std::string& str2)
    {
        return (str1.size() == str2.size()) && (toLower(str1) == toLower(str2));
    }

    inline std::string sanitizeIdentifier(const std::string& str)
    {
#if 0
        // TODO: [MG] - we have to add some sanitizing logic, but definitely NOT replacing dots by underscores
        std::replace(str.begin(), str.end(), '.', '_');
#endif
        return str;
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

} MAT_NS_END
#endif

