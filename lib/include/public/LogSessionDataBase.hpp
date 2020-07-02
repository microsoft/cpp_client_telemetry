#ifndef LOGSESSIONDATABASE_HPP
#define LOGSESSIONDATABASE_HPP

#include "Version.hpp"

#include <string>

namespace ARIASDK_NS_BEGIN
{

    /// <summary>
    /// The LogSessionData class represents the session cache.
    /// </summary>
    class LogSessionDataBase
    {
    public:
        /// <summary>
        /// The LogSessionData constructor, taking a cache file path.
        /// </summary>
        LogSessionDataBase() = delete;

        /// <summary>
        /// Gets the time that this session began.
        /// </summary>
        /// <returns>A 64-bit integer that contains the time.</returns>
        virtual unsigned long long getSessionFirstTime() const           = 0;
       
        /// <summary>
        /// Gets the SDK unique identifier.
        /// </summary>
        virtual std::string getSessionSDKUid() const= 0;


    protected:
        
        std::string                         m_sessionSDKUid;
        unsigned long long                  m_sessionFirstTimeLaunch;
    };


} ARIASDK_NS_END

#endif
