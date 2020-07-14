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
       
        /// <summary>
        /// Gets the time that this session began.
        /// </summary>
        /// <returns>A 64-bit integer that contains the time.</returns>
        virtual unsigned long long getSessionFirstTime() const = 0;
       
        /// <summary>
        /// Gets the SDK unique identifier.
        /// </summary>
        virtual std::string getSessionSDKUid() const = 0;

    protected:

        std::string                         m_sessionSDKUid {""};
        unsigned long long                  m_sessionFirstTimeLaunch{0ull} ;
    };

    /// <summary>
    /// The LogSessionData class represents the session cache.
    /// </summary>
    class LogSessionDataFile : public LogSessionData
    {
    public:
        /// <summary>
        /// The LogSessionData constructor, taking a cache file path.
        /// </summary>
        LogSessionDataFile(std::string const& cacheFilePath);

        virtual unsigned long long getSessionFirstTime() const override;
        virtual std::string getSessionSDKUid() const override;

    protected:

        void open(const std::string& path);

        bool parse(const std::string& cacheContents);
    };


} ARIASDK_NS_END
#endif
