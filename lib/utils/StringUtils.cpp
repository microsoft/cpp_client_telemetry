//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
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

    bool StringUtils::AreAllCharactersWhitelisted(const string& stringToTest, const string& whitelist)
    {
        return (stringToTest.find_first_not_of(whitelist) == string::npos);
    }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#endif
    /**
     * Convert various numeric types and bool to string in an uniform manner.
     */
    template<typename T>
    std::string to_string(char const* format, T value)
    {
        static const int buf_size = 40;
        char buf[buf_size] = { 0 };
#ifdef _WIN32
        ::_snprintf_s(buf, buf_size, format, value);
#else
        snprintf(buf, buf_size, format, value);
#endif
        return std::string(buf);
    }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    std::string toString(char const*        value) { return std::string(value); }
    std::string toString(bool               value) { return value ? "true" : "false"; }
    std::string toString(char               value) { return to_string("%d", static_cast<signed char>(value)); }
    std::string toString(int                value) { return to_string("%d", value); }
    std::string toString(long               value) { return to_string("%ld", value); }
    std::string toString(long long          value) { return to_string("%lld", value); }
    std::string toString(unsigned char      value) { return to_string("%u", value); }
    std::string toString(unsigned int       value) { return to_string("%u", value); }
    std::string toString(unsigned long      value) { return to_string("%lu", value); }
    std::string toString(unsigned long long value) { return to_string("%llu", value); }
    std::string toString(float              value) { return to_string("%f", value); }
    std::string toString(double             value) { return to_string("%f", value); }
    std::string toString(long double        value) { return to_string("%Lf", value); }

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
