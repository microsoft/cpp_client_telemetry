// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGSESSIONDATADB_HPP
#define LOGSESSIONDATADB_HPP

#include "IOfflineStorage.hpp"
#include "Version.hpp"

#include <string>

namespace ARIASDK_NS_BEGIN
{

    /// <summary>
    /// The LogSessionData class represents the session cache.
    /// </summary>
    class LogSessionDataDB
    {
    public:
        /// <summary>
        /// The LogSessionData constructor, taking a cache file path.
        /// </summary>
        LogSessionDataDB(IOfflineStorage* offlineStorage);

        /// <summary>
        /// Gets the time that this session began.
        /// </summary>
        /// <returns>A 64-bit integer that contains the time.</returns>
        unsigned long long getSessionFirstTime() const
        {
            return m_sessionFirstTimeLaunch;
        }

        /// <summary>
        /// Gets the SDK unique identifier.
        /// </summary>
        std::string getSessionSDKUid() const
        {
            return m_sessionSDKUid;
        }

    protected:

        void validateAndSetSdkId(const std::string& sdkId);

        void setSessionData(IOfflineStorage* offlineStorage);

        std::string                         m_sessionSDKUid;
        unsigned long long                  m_sessionFirstTimeLaunch;
    };


} ARIASDK_NS_END
#endif
