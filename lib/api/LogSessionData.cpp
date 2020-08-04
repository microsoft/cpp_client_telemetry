// Copyright (c) Microsoft. All rights reserved.
#include <string.h>

#include "LogSessionData.hpp"
using namespace std;

namespace ARIASDK_NS_BEGIN {

    LogSessionData::LogSessionData(
        unsigned long long sessionFirstTimeLaunch,
        const std::string sessionSDKUid) :
        m_sessionFirstTimeLaunch(sessionFirstTimeLaunch),
        m_sessionSDKUid(sessionSDKUid)
    {
    }

    unsigned long long LogSessionData::getSessionFirstTime() const
    {
        return m_sessionFirstTimeLaunch;
    }

    std::string LogSessionData::getSessionSDKUid() const
    {
        return m_sessionSDKUid;
    }

} ARIASDK_NS_END

