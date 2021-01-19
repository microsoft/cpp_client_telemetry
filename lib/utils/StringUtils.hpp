//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STRINGUTILS_HPP
#define STRINGUTILS_HPP

#include "Version.hpp"
#include "ctmacros.hpp"

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

} MAT_NS_END
#endif

