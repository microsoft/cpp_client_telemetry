// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGSESSIONDATA_HPP
#define LOGSESSIONDATA_HPP

#include "LogSessionDataBase.hpp"
#include "Version.hpp"

#include <string>

namespace ARIASDK_NS_BEGIN
{

    /// <summary>
    /// The LogSessionData class represents the session cache.
    /// </summary>
    class LogSessionData : public LogSessionDataBase
    {
    public:
        /// <summary>
        /// The LogSessionData constructor, taking a cache file path.
        /// </summary>
        LogSessionData(std::string const& cacheFilePath);

        virtual unsigned long long getSessionFirstTime() const override;
        virtual std::string getSessionSDKUid() const override;
        

    protected:

        void open(const std::string& path);

        bool parse(const std::string& cacheContents);
    };


} ARIASDK_NS_END
#endif
