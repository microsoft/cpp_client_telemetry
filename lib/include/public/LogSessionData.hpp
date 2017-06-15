// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "string"
#include "Version.hpp"

namespace ARIASDK_NS_BEGIN {

    class IStorage;
    class LogSessionData
    {
    public:
        LogSessionData(std::string const& cacheFilePath);

        unsigned long long getSesionFirstTime() const;

        std::string& getSessionSDKUid();

    private:

        std::string                         m_sessionSDKUid;
        unsigned long long                  m_sessionFirstTimeLaunch;
        IStorage*                           m_sessionStorage;

        bool StartSessionStorage(std::string const& sessionpath);
        void StopSessionStorage();
        void PopulateSession();
        unsigned long long to_long(const char *string, size_t size);
    };


} ARIASDK_NS_END
