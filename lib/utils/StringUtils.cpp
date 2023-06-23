//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "StringUtils.hpp"

using std::string;
using std::vector;

namespace MAT_NS_BEGIN
{
    void StringUtils::SplitString(const string& s, const char separator, vector<string>& parts)
    {
        if (s.size())
        {
            size_t cur = 0;
            for (size_t i = 0; i != s.size(); ++i)
            {
                if (s[i] == separator)
                {
                    parts.push_back(s.substr(cur, i - cur));
                    cur = i;
                    cur++;
                }
            }

            if (s.size() > 0)
            {
                parts.push_back(s.substr(cur));
            }
        }
    }

    bool StringUtils::AreAllCharactersAllowlisted(const string& stringToTest, const string& allowlist)
    {
        return (stringToTest.find_first_not_of(allowlist) == string::npos);
    }

    std::string toString(char const*        value) { return std::string { value }; }
    std::string toString(bool               value) { return value ? std::string { "true" } : std::string { "false" }; }
    std::string toString(char               value) { return std::to_string(static_cast<signed char>(value)); }
    std::string toString(int                value) { return std::to_string(value); }
    std::string toString(long               value) { return std::to_string(value); }
    std::string toString(long long          value) { return std::to_string(value); }
    std::string toString(unsigned char      value) { return std::to_string(value); }
    std::string toString(unsigned int       value) { return std::to_string(value); }
    std::string toString(unsigned long      value) { return std::to_string(value); }
    std::string toString(unsigned long long value) { return std::to_string(value); }
    std::string toString(float              value) { return std::to_string(value); }
    std::string toString(double             value) { return std::to_string(value); }
    std::string toString(long double        value) { return std::to_string(value); }

    std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(),
                       [](unsigned char c) { return (char)::tolower(c); });
        return result;
    }

    std::string toUpper(const std::string& str)
    {
        std::string result = str;
        std::transform(str.begin(), str.end(), result.begin(),
                       [](unsigned char c) { return (char)::toupper(c); });
        return result;
    }

    bool equalsIgnoreCase(const std::string& str1, const std::string& str2)
    {
        return (str1.size() == str2.size()) && (toLower(str1) == toLower(str2));
    }

    std::string sanitizeIdentifier(const std::string& str)
    {
#if 0
        // TODO: [MG] - we have to add some sanitizing logic, but definitely NOT replacing dots by underscores
        std::replace(str.begin(), str.end(), '.', '_');
#endif
        return str;
    }

    const char* priorityToStr(EventPriority priority)
    {
        switch (priority)
        {
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

    const char* latencyToStr(EventLatency latency)
    {
        switch (latency)
        {
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

    inline bool replace(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

} MAT_NS_END
