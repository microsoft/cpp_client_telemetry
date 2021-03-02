//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include "ctmacros.hpp"
#include <EventProperty.hpp>

#include <algorithm>
#include <string>
#include <vector>

namespace MAT_NS_BEGIN
{
    namespace StringUtils
    {
        void SplitString(const std::string& s, const char separator, std::vector<std::string>& parts);
        bool AreAllCharactersWhitelisted(const std::string& stringToTest, const std::string& whitelist);
    }

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

    std::string toLower(const std::string& str);

    std::string toUpper(const std::string& str);

    bool equalsIgnoreCase(const std::string& str1, const std::string& str2);

    std::string sanitizeIdentifier(const std::string& str);

    const char* priorityToStr(EventPriority priority);

    const char* latencyToStr(EventLatency latency);

    bool replace(std::string& str, const std::string& from, const std::string& to);

} MAT_NS_END
#endif

