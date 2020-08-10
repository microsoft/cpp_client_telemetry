// Copyright (c) Microsoft. All rights reserved.
#include <string.h>

#include "LogSessionData.hpp"
#include "offline/LogSessionDataProvider.hpp"
using namespace std;

namespace ARIASDK_NS_BEGIN {

    uint64_t LogSessionData::getSessionFirstTime() const
    {
        return m_sessionFirstTimeLaunch;
    }

    std::string LogSessionData::getSessionSDKUid() const
    {
        return m_sessionSDKUid;
    }

} ARIASDK_NS_END
