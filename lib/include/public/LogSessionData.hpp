// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGSESSIONDATA_HPP
#define LOGSESSIONDATA_HPP

#include "Version.hpp"

#include <string>

namespace ARIASDK_NS_BEGIN
{

    class LogSessionData
    {
    public:
        LogSessionData(unsigned long long, std::string);

        /// <summary>
        /// Gets the time that this session began.
        /// </summary>
        /// <returns>A 64-bit integer that contains the time.</returns>
        unsigned long long getSessionFirstTime() const ;

        /// <summary>
        /// Gets the SDK unique identifier.
        /// </summary>
        std::string getSessionSDKUid() const ;

    protected:
        unsigned long long                  m_sessionFirstTimeLaunch{0ull} ;
        std::string                         m_sessionSDKUid;
    };

} ARIASDK_NS_END
#endif
