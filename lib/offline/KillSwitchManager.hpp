//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef KILLSWITCHMANAGER_HPP
#define KILLSWITCHMANAGER_HPP

#include "pal/PAL.hpp"

#include <cctype>
#include <list>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include <atomic>

namespace MAT_NS_BEGIN {

    class KillSwitchManager
    {
    public:

        bool isActive()
        {
            return !m_tokenTime.empty();
        }

        KillSwitchManager() : m_isRetryAfterActive(false), m_retryAfterExpiryTime(0)
        {
        }

        virtual ~KillSwitchManager()
        {
        }

        bool handleResponse(HttpHeaders& headers)
        {
            bool isNewTokenKilled = false;

            std::string timeStr = headers.get("Retry-After");
            if (!timeStr.empty())
            {
                int64_t timeinSecs = 0;
                if (tryParseSeconds(timeStr, timeinSecs) && timeinSecs > 0)
                {
                    std::lock_guard<std::mutex> guard(m_lock);
                    m_retryAfterExpiryTime = PAL::getUtcSystemTime() + timeinSecs;
                    m_isRetryAfterActive = true;
                }
            }

            std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator> ret;
            ret = headers.equal_range("kill-tokens");

            if (ret.first != ret.second)
            {
                std::vector<std::string> killtokensVector;

                for (std::multimap<std::string, std::string>::const_iterator it = ret.first; it != ret.second; ++it)
                {
                    std::string token = std::move(it->second);
                    size_t pos = token.find(':');
                    if (pos != std::string::npos)
                    {
                        // Strip suffix and assume ':all' events of that tenant are killed
                        token.erase(pos, token.length() - pos);
                    }
                    // Ignore kill-tokens containing control characters; tenant
                    // tokens are otherwise opaque and safe (the DELETE is
                    // parameterized), so they must remain killable.
                    if (!isValidTenantToken(token))
                    {
                        continue;
                    }
                    killtokensVector.push_back(token);
                }

                int64_t timeinSecs = 0;
                std::string timeString = headers.get("kill-duration");
                if (!timeString.empty())
                {
                    tryParseSeconds(timeString, timeinSecs);
                }

                if (killtokensVector.size() > 0 && timeinSecs > 0)
                {
                    isNewTokenKilled = true;
                    for (std::vector<std::string>::iterator iter = killtokensVector.begin(); iter < killtokensVector.end(); ++iter)
                    {
                        addToken(*iter, timeinSecs);
                    }
                }
            }
            return isNewTokenKilled;
        }

        void addToken(const std::string& tokenId, int64_t timeInSeconds)
        {
            std::lock_guard<std::mutex> guard(m_lock);
            if (timeInSeconds > 0)
            {
                m_tokenTime[tokenId] = PAL::getUtcSystemTime() + timeInSeconds; //convert milisec to sec
            }
        }

        bool isTokenBlocked(const std::string& tokenId)
        {
            std::lock_guard<std::mutex> guard(m_lock);

            if (m_isRetryAfterActive)
            {
                if (m_retryAfterExpiryTime > PAL::getUtcSystemTime())
                {
                    return true;//always return true for all tokens
                }
                else
                {
                    m_retryAfterExpiryTime = 0;
                    m_isRetryAfterActive = false;
                }
            }
            std::map<std::string, int64_t>::iterator iter = m_tokenTime.find(tokenId);
            if (iter != m_tokenTime.end())
            {//found, check the time stamp
                int64_t timeStamp = m_tokenTime[tokenId];

                if (timeStamp > PAL::getUtcSystemTime())  //convert milisec to sec
                {
                    return true;
                }
                else
                { //remove the entry for this token as this has expired
                    m_tokenTime.erase(tokenId);
                }
            }

            return false;
        }

        void removeToken(const std::string& tokenId)
        {
            std::lock_guard<std::mutex> guard(m_lock);
            std::map<std::string, int64_t>::iterator iter = m_tokenTime.find(tokenId);
            if (iter != m_tokenTime.end())
            {//found, check the time stamp
                m_tokenTime.erase(tokenId);
            }
        }

        std::list<std::string> getTokensList()
        {
            std::lock_guard<std::mutex> guard(m_lock);
            std::list<std::string> result;
            for (const auto &kv : m_tokenTime)
            {
                result.push_back(kv.first);
            }
            return result;
        }

        bool isRetryAfterActive()
        {
            return m_isRetryAfterActive;
        }

    private:
        // Parse a count of seconds from a response-header value (Retry-After /
        // kill-duration). Returns false when the value is non-numeric or out of
        // range instead of letting std::stoll throw: the worker thread that
        // drives handleResponse has no exception guard, so a throw here would
        // crash the process. A standards-compliant server may legitimately send
        // Retry-After as an HTTP-date, which is non-numeric and must be ignored.
        static bool tryParseSeconds(const std::string& value, int64_t& outSeconds)
        {
            outSeconds = 0;
            try
            {
                size_t consumed = 0;
                const long long parsed = std::stoll(value, &consumed);
                // Reject trailing garbage (e.g. "120; foo"); only trailing
                // whitespace is allowed, so a malformed header is ignored rather
                // than silently parsed from its numeric prefix.
                for (size_t i = consumed; i < value.size(); ++i)
                {
                    if (std::isspace(static_cast<unsigned char>(value[i])) == 0)
                    {
                        return false;
                    }
                }
                outSeconds = static_cast<int64_t>(parsed);
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
        }

        // Tenant tokens are opaque strings elsewhere in the SDK (they may contain
        // spaces, quotes, etc.), and the offline-storage DELETE is parameterized,
        // so any value is safe to act on. Only reject genuinely unsafe content:
        // control characters (including CR/LF, which could enable log injection)
        // and over-long values. Over-restricting here would prevent a legitimately
        // stored tenant token from ever being killed.
        static bool isValidTenantToken(const std::string& token)
        {
            if (token.empty() || token.size() > 256)
            {
                return false;
            }
            for (unsigned char c : token)
            {
                // Reject C0 control characters and DEL; allow any other byte
                // (printable ASCII and UTF-8 sequences).
                if (c < 0x20 || c == 0x7f)
                {
                    return false;
                }
            }
            return true;
        }

        std::map<std::string, int64_t> m_tokenTime;
        std::mutex      m_lock;
        bool            m_isRetryAfterActive;
        int64_t         m_retryAfterExpiryTime;
    };

} MAT_NS_END
#endif

