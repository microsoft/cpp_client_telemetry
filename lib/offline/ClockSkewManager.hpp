//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef CLOCKSKEWMANAGER_HPP
#define CLOCKSKEWMANAGER_HPP

#include "pal/PAL.hpp"

#include <vector>
#include <mutex>

namespace MAT_NS_BEGIN {

    class ClockSkewManager
    {
    public:
        ClockSkewManager() = default;

        void handleResponse(const HttpHeaders& headers)
        {
            const auto& timeString = headers.get("time-delta-millis");
            if (!timeString.empty())
            {
                SetDelta(timeString);
            }
            else
            {
                SetDelta("");
            }
            m_resumeTransmissionAfterClockSkew = true;
        }

        void SetDelta(const std::string& delta)
        {
            m_deltaReceived = true;
            m_delta = delta;
        }

        bool isClockSkewOn() const
        {
            if (!m_pingSent || (m_deltaReceived && !m_delta.empty()))
            {
               return true;
            }
            return false;
        }

        bool isWaitingForClockSkew()
        {
            if (!m_deltaReceived && m_pingSent)
            {
               int64_t timeSincePing = PAL::getUtcSystemTime() - m_pingSendTime;
               if (timeSincePing > 30) //30 secs have passed since ping was sent, something went wrong, disable clock skew
               {
                  SetDelta("");
                  return false;
               }
               return true;
            }
            return false;
        }

        std::string GetDelta()
        {
            if (m_pingSent == false)
            {
               m_pingSent = true;
               m_pingSendTime = PAL::getUtcSystemTime();
               return "use-collector-delta";
            }
            else
            {
               return m_delta;
            }
        }

        bool GetResumeTransmissionAfterClockSkew() const
        {
            return m_resumeTransmissionAfterClockSkew;
        }

        void SetResumeTransmissionAfterClockSkew(bool value)
        {
            m_resumeTransmissionAfterClockSkew = value;
        }

    private:
        std::string             m_delta {};
        bool                    m_pingSent {};
        bool                    m_deltaReceived {};
        bool                    m_resumeTransmissionAfterClockSkew {};
        int64_t                 m_pingSendTime {};
    };

} MAT_NS_END
#endif

