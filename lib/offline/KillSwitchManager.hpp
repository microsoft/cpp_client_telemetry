//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef KILLSWITCHMANAGER_HPP
#define KILLSWITCHMANAGER_HPP

#include "pal/PAL.hpp"

#include <map>
#include <stdexcept>
#include <string>
#include <mutex>

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
                    // Defense in depth: only accept tenant tokens that match the
                    // expected character set. Offline-storage deletes are already
                    // parameterized, but rejecting clearly-malformed tokens avoids
                    // acting on attacker-shaped values from a (possibly MITM'd)
                    // collector response.
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
        // Parse a count of seconds from an untrusted response-header value
        // (Retry-After / kill-duration). Returns false when the value is
        // non-numeric or out of range instead of letting std::stoi throw: the
        // worker thread that drives handleResponse has no exception guard, so a
        // throw here would crash the process. Note that a standards-compliant
        // server may legitimately send Retry-After as an HTTP-date, which is
        // non-numeric and must be ignored rather than fatal.
        static bool tryParseSeconds(const std::string& value, int64_t& outSeconds)
        {
            try
            {
                outSeconds = static_cast<int64_t>(std::stoll(value));
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
        }

        // A tenant token is the API ingestion key tenant id: alphanumeric with
        // '-', '_' or '.' separators. Reject anything else (quotes, ';',
        // whitespace, control characters) that a legitimate token never contains.
        static bool isValidTenantToken(const std::string& token)
        {
            if (token.empty() || token.size() > 256)
            {
                return false;
            }
            for (char c : token)
            {
                const bool ok =
                    (c >= '0' && c <= '9') ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c == '-') || (c == '_') || (c == '.');
                if (!ok)
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

