//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef LOGSESSIONDATA_HPP
#define LOGSESSIONDATA_HPP

#include "ctmacros.hpp"

#include <string>
#include <cstdint>

namespace MAT_NS_BEGIN
{
    class LogSessionData
    {
    public:

        LogSessionData(
            uint64_t sessionFirstTimeLaunch,
            std::string sessionSDKUid)
            :
            m_sessionFirstTimeLaunch(sessionFirstTimeLaunch),
            m_sessionSDKUid(sessionSDKUid)
        {
        }

        /// <summary>
        /// Gets the time that this session began.
        /// </summary>
        /// <returns>A 64-bit integer that contains the time.</returns>
        uint64_t getSessionFirstTime() const;

        /// <summary>
        /// Gets the SDK unique identifier.
        /// </summary>
        std::string getSessionSDKUid() const;

    protected:
        const uint64_t                m_sessionFirstTimeLaunch{0ull};
        const std::string             m_sessionSDKUid;
    };

} MAT_NS_END
#endif

