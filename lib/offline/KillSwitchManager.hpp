//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef KILLSWITCHMANAGER_HPP
#define KILLSWITCHMANAGER_HPP

#include "pal/PAL.hpp"

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
                    // it->second is a const reference (const_iterator), so this is a
                    // copy; no std::move (moving from const silently copies anyway).
                    std::string token = it->second;
                    size_t pos = token.find(':');
                    if (pos != std::string::npos)
                    {
                        // Strip suffix and assume ':all' events of that tenant are killed
                        token.erase(pos);
                    }
                    // Reject kill-tokens with control characters (defensive
                    // hygiene; see isValidTenantToken). Any other opaque value is
                    // safe to act on and must remain killable -- over-restricting
                    // would let that tenant escape the kill.
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
        // kill-duration). Returns false when the value is malformed or out of
        // range instead of letting std::stoll throw: the worker thread that drives
        // handleResponse has no exception guard, so a throw here would crash the
        // process.
        //
        // RFC 7231 allows Retry-After to be either delay-seconds or an HTTP-date.
        // We deliberately accept only delay-seconds and ignore the HTTP-date form:
        //   - The collector this SDK targets sends Retry-After as delay-seconds, so
        //     the date form does not occur in practice.
        //   - An HTTP-date is absolute and would have to be converted to a delay
        //     against the device clock, which a telemetry SDK cannot trust (it has
        //     a separate clock-skew channel for exactly this reason); a skewed clock
        //     would yield a wrong, negative, or huge back-off.
        //   - Parsing it would add an error-prone three-format date parser on this
        //     no-exception-guard worker thread for no practical benefit.
        // Ignoring an unparseable value degrades safely: the SDK's own retry
        // back-off still applies; only the server's exact instant is not honored.
        //
        // RFC 7231 delay-seconds is 1*DIGIT, optionally surrounded by HTTP optional
        // whitespace (OWS = SP / HTAB). We trim only OWS and require the remaining
        // value to be all digits. This rejects malformed inputs that std::stoll
        // would otherwise accept -- a leading '+'/'-' sign, or leading CR/LF and
        // other control whitespace that stoll silently skips -- and keeps leading
        // and trailing handling symmetric.
        static bool tryParseSeconds(const std::string& value, int64_t& outSeconds)
        {
            outSeconds = 0;
            size_t begin = 0;
            size_t end = value.size();
            while (begin < end && (value[begin] == ' ' || value[begin] == '\t'))
            {
                ++begin;
            }
            while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t'))
            {
                --end;
            }
            if (begin == end)
            {
                return false;
            }
            for (size_t i = begin; i < end; ++i)
            {
                if (value[i] < '0' || value[i] > '9')
                {
                    return false;
                }
            }
            try
            {
                // The substring is all digits, so std::stoll itself can only throw
                // std::out_of_range; substr() may also throw (e.g. std::bad_alloc).
                // Either way the std::exception catch below ignores the value rather
                // than crashing.
                const long long parsed = std::stoll(value.substr(begin, end - begin));
                // Clamp to a value that cannot overflow when later added to a current
                // UTC timestamp (seconds) to compute an expiry time. No legitimate
                // Retry-After / kill-duration approaches this; an absurd value is
                // capped instead of wrapping the expiry into the past.
                const int64_t kMaxSeconds = 100LL * 365 * 24 * 60 * 60; // ~100 years
                outSeconds = (parsed > kMaxSeconds) ? kMaxSeconds : static_cast<int64_t>(parsed);
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
        }

        // Tenant tokens are opaque (they may legitimately contain spaces, quotes,
        // etc.). Every sink that consumes the token handles raw bytes safely -- the
        // offline-storage DELETE is parameterized (SQLite bind / Room DAO) and the
        // in-memory match is a plain string compare -- so any printable value must
        // remain killable; over-restricting would let that tenant escape the kill.
        // We reject empty tokens and control characters as defensive hygiene:
        // a legitimate token never contains control characters, an embedded NUL would
        // be truncated by the Room JNI NewStringUTF(c_str()) call (acting on the wrong
        // token), and it keeps the value safe if it ever reaches a log/display sink.
        // We also bound the length: real tenant tokens are a fixed, small size
        // (~74 chars; see HttpRequestEncoder.cpp), so kMaxTenantTokenBytes (256) is a
        // generous ceiling -- it caps memory growth from a malicious/oversized
        // kill-token in an untrusted HTTP response, while staying far above any real
        // token so a legitimate tenant can never be made unkillable by this check.
        static constexpr size_t kMaxTenantTokenBytes = 256;
        static bool isValidTenantToken(const std::string& token)
        {
            if (token.empty() || token.size() > kMaxTenantTokenBytes)
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

