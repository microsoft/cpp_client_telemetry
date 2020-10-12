//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STRINGMATCHER_HPP
#define STRINGMATCHER_HPP

namespace MAT_NS_BEGIN
{
    /// <summary>
    /// Simple class to cheaply manage string matches. Note that we currently
    /// have no way to exactly match a string of "*"
    /// </summary>
    class StringMatcher
    {
    private:
        enum MatchType
        {
            Invalid,
            StartsWith,
            ExactMatch,
        };

        MatchType   _matchType;
        std::string _matchTarget;

        void Initialize(const std::string& stringToMatch)
        {
            if (stringToMatch.empty())
            {
                _matchType = Invalid;
            }
            else if ('*' == *(stringToMatch.rbegin()) )
            {
                _matchType = StartsWith;
                _matchTarget = stringToMatch.substr(0, stringToMatch.length() - 1);
            }
            else
            {
                _matchType = ExactMatch;
                _matchTarget = stringToMatch;
            }
        }

    public:
        /// <summary>
        /// ctor
        /// <param name="stringToMatch">The string that will be used by this matcher.
        /// If it ends with an asterisk, then it will use a 'StartsWith' match</param>
        /// </summary>
        StringMatcher(const std::string& stringToMatch)
        {
            Initialize(stringToMatch);
        }

        /// <summary>
        /// ctor
        /// <param name="stringToMatch">The string that will be used by this matcher.
        /// If it ends with an asterisk, then it will use a 'StartsWith' match</param>
        /// </summary>
        StringMatcher(const char* stringToMatch)
        {
            if (stringToMatch == nullptr)
            {
                _matchType = Invalid;
            }
            else
            {
                std::string tempStringToMatch(stringToMatch);
                Initialize(tempStringToMatch);
            }
        }

        /// <summary>
        /// copy ctor
        /// </summary>
        StringMatcher(const StringMatcher& source)
        {
            _matchType = source._matchType;
            _matchTarget = source._matchTarget;
        }

        /// <summary>
        /// Check to see if the target string matches the parameters for this matcher
        /// <param name="target">The target string</param>
        /// <returns>true only if the string matches the criteria</returns>
        /// </summary>
        bool Matches(const std::string& target) const
        {
            // Empty string can only match a wildcard of '*'
            if (target.length() == 0)
            {
                return (_matchType == StartsWith && _matchTarget.empty());
            }

            switch (_matchType)
            {
            case Invalid:
                return false;

            case ExactMatch:
                return target.compare(_matchTarget) == 0;

            case StartsWith:
                return target.compare(0, _matchTarget.length(), _matchTarget) == 0;
            }

            return false;
        }

        bool IsValid() const
        {
            return _matchType != Invalid;
        }

        bool IsExactMatch() const
        {
            return _matchType == ExactMatch;
        }

        bool IsStartsWith() const
        {
            return _matchType == StartsWith;
        }
    };

} MAT_NS_END
#endif

