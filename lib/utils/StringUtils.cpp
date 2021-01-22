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

} MAT_NS_END
