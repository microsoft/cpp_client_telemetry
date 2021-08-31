//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef KILLSWITCHMANAGER_HPP
#define KILLSWITCHMANAGER_HPP

#include "pal/PAL.hpp"

#include <map>
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
                int64_t timeinSecs = std::stoi(timeStr);
                if (timeinSecs > 0)
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
                    killtokensVector.push_back(token);
                }

                int64_t timeinSecs = 0;
                std::string timeString = headers.get("kill-duration");
                if (!timeString.empty())
                {
                    timeinSecs = std::stoi(timeString);
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
        std::map<std::string, int64_t> m_tokenTime;
        std::mutex      m_lock;
        bool            m_isRetryAfterActive;
        int64_t         m_retryAfterExpiryTime;
    };

} MAT_NS_END
#endif

