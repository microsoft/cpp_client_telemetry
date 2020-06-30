// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGSESSIONDATADB_HPP
#define LOGSESSIONDATADB_HPP

#include "LogSessionDataBase.hpp"
#include "Version.hpp"

#include <string>

namespace ARIASDK_NS_BEGIN
{

    /// <summary>
    /// The LogSessionData class represents the session cache.
    /// </summary>
    class IOfflineStorage;

    class LogSessionDataDB : public LogSessionDataBase
    {
    public:
        /// <summary>
        /// The LogSessionData constructor, taking a cache file path.
        /// </summary>
        LogSessionDataDB(IOfflineStorage* offlineStorage);

    protected:

        void validateAndSetSdkId(const std::string& sdkId);

        void setSessionData(IOfflineStorage* offlineStorage);
    };


} ARIASDK_NS_END
#endif
