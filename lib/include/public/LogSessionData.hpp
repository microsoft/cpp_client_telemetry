// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGSESSIONDATA_HPP
#define LOGSESSIONDATA_HPP

#include "Version.hpp"

#include <string>
#include <cstdint>

namespace ARIASDK_NS_BEGIN
{
    class LogSessionDataProvider;
    class LogSessionData
    {
    public:
        LogSessionData(LogSessionDataProvider *);

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
        uint64_t                m_sessionFirstTimeLaunch{0ull};
        std::string             m_sessionSDKUid;
        LogSessionDataProvider* m_logSessionDataProvider;
    };

} ARIASDK_NS_END
#endif
