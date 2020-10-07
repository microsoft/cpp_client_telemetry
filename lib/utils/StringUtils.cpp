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

} MAT_NS_END
