//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <string.h>

#include "LogSessionData.hpp"
#include "offline/LogSessionDataProvider.hpp"
using namespace std;

namespace MAT_NS_BEGIN {

    uint64_t LogSessionData::getSessionFirstTime() const
    {
        return m_sessionFirstTimeLaunch;
    }

    std::string LogSessionData::getSessionSDKUid() const
    {
        return m_sessionSDKUid;
    }

} MAT_NS_END

