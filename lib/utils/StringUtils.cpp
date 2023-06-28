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

    std::string to_string(const GUID_t& uuid)
    {
        static char inttoHex[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
        const unsigned buffSize = 36 + 1;  // 36 + null-termination
        char buf[buffSize] = { 0 };

        int  test = (uuid.Data1 >> 28 & 0x0000000F);
        buf[0] = inttoHex[test];
        test = (int)(uuid.Data1 >> 24 & 0x0000000F);
        buf[1] = inttoHex[test];
        test = (int)(uuid.Data1 >> 20 & 0x0000000F);
        buf[2] = inttoHex[test];
        test = (int)(uuid.Data1 >> 16 & 0x0000000F);
        buf[3] = inttoHex[test];
        test = (int)(uuid.Data1 >> 12 & 0x0000000F);
        buf[4] = inttoHex[test];
        test = (int)(uuid.Data1 >> 8 & 0x0000000F);
        buf[5] = inttoHex[test];
        test = (int)(uuid.Data1 >> 4 & 0x0000000F);
        buf[6] = inttoHex[test];
        test = (int)(uuid.Data1 & 0x0000000F);
        buf[7] = inttoHex[test];
        buf[8] = '-';
        test = (int)(uuid.Data2 >> 12 & 0x000F);
        buf[9] = inttoHex[test];
        test = (int)(uuid.Data2 >> 8 & 0x000F);
        buf[10] = inttoHex[test];
        test = (int)(uuid.Data2 >> 4 & 0x000F);
        buf[11] = inttoHex[test];
        test = (int)(uuid.Data2 & 0x000F);
        buf[12] = inttoHex[test];
        buf[13] = '-';
        test = (int)(uuid.Data3 >> 12 & 0x000F);
        buf[14] = inttoHex[test];
        test = (int)(uuid.Data3 >> 8 & 0x000F);
        buf[15] = inttoHex[test];
        test = (int)(uuid.Data3 >> 4 & 0x000F);
        buf[16] = inttoHex[test];
        test = (int)(uuid.Data3 & 0x000F);
        buf[17] = inttoHex[test];
        buf[18] = '-';
        test = (int)(uuid.Data4[0] >> 4 & 0x0F);
        buf[19] = inttoHex[test];
        test = (int)(uuid.Data4[0] & 0x0F);
        buf[20] = inttoHex[test];
        test = (int)(uuid.Data4[1] >> 4 & 0x0F);
        buf[21] = inttoHex[test];
        test = (int)(uuid.Data4[1] & 0x0F);
        buf[22] = inttoHex[test];
        buf[23] = '-';
        test = (int)(uuid.Data4[2] >> 4 & 0x0F);
        buf[24] = inttoHex[test];
        test = (int)(uuid.Data4[2] & 0x0F);
        buf[25] = inttoHex[test];
        test = (int)(uuid.Data4[3] >> 4 & 0x0F);
        buf[26] = inttoHex[test];
        test = (int)(uuid.Data4[3] & 0x0F);
        buf[27] = inttoHex[test];
        test = (int)(uuid.Data4[4] >> 4 & 0x0F);
        buf[28] = inttoHex[test];
        test = (int)(uuid.Data4[4] & 0x0F);
        buf[29] = inttoHex[test];
        test = (int)(uuid.Data4[5] >> 4 & 0x0F);
        buf[30] = inttoHex[test];
        test = (int)(uuid.Data4[5] & 0x0F);
        buf[31] = inttoHex[test];
        test = (int)(uuid.Data4[6] >> 4 & 0x0F);
        buf[32] = inttoHex[test];
        test = (int)(uuid.Data4[6] & 0x0F);
        buf[33] = inttoHex[test];
        test = (int)(uuid.Data4[7] >> 4 & 0x0F);
        buf[34] = inttoHex[test];
        test = (int)(uuid.Data4[7] & 0x0F);
        buf[35] = inttoHex[test];
        buf[36] = 0;
        return std::string(buf);
    }

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
