// Copyright (c) Microsoft. All rights reserved.
#ifndef LOGSESSIONDATADB_HPP
#define LOGSESSIONDATADB_HPP

#include "LogSessionData.hpp"
#include "IOfflineStorage.hpp"
#include "pal/PAL.hpp"
#include "DebugTrace.hpp"
#include "utils/Utils.hpp"
#include "utils/StringUtils.hpp"
#include "Version.hpp"

#include <string>
#include <memory>

namespace ARIASDK_NS_BEGIN
{
    static const char* sessionFirstLaunchTimeName = "sessionfirstlaunchtime";
    static const char* sessionSdkUid = "sessionsdkuid";


    /// <summary>
    /// The LogSessionDataDB class represents the session cache in DB
    /// </summary>
    class LogSessionDataDB : public LogSessionData
    {
    public:
        /// <summary>
        /// The LogSessionData constructor, taking the offline storage instance
        /// </summary>
        LogSessionDataDB(IOfflineStorage* offlineStorage)
        :
        m_offlineStorage(offlineStorage)
        {
        }

        virtual std::string getSessionSDKUid() const override
        {
            if (!m_isDBInitialized)
            {
                //TODO (labhas) - Need better way instead of remove constness
                (const_cast<LogSessionDataDB*>(this))->Initialize(); 
            }
            return m_sessionSDKUid;
        }

        virtual unsigned long long getSessionFirstTime() const override
        {
            if (!m_isDBInitialized)
            {
                (const_cast<LogSessionDataDB*>(this))->Initialize();
            }
            return m_sessionFirstTimeLaunch;
        }

    protected:

        void Initialize()
        {
            m_sessionSDKUid = m_offlineStorage->GetSetting(sessionSdkUid);
            m_sessionFirstTimeLaunch = convertStrToLong(m_offlineStorage->GetSetting(sessionFirstLaunchTimeName));
            if (!m_sessionFirstTimeLaunch || m_sessionSDKUid.empty())
            {
                m_sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
                m_sessionSDKUid = PAL::generateUuidString();
                setSessionData();
            }
            m_isDBInitialized = true;
        }

        unsigned long long convertStrToLong(const std::string& s)
        {
            unsigned long long res = 0ull;
            try
            {
                res = std::stoull(s);
            }
            catch (const std::invalid_argument&)
            {
                LOG_WARN("Non-integer data passed to std::stoull");
            }
            catch (const std::out_of_range&)
            {
                LOG_WARN("Value passed to std::stoull was larger than unsigned long long could represent");
            }
            return res;          
        }

        void setSessionData()
        {
            if (!m_offlineStorage->StoreSetting(sessionFirstLaunchTimeName, std::to_string(m_sessionFirstTimeLaunch)))
            {    
               LOG_WARN("Unable to save session analytics to DB for %s", sessionFirstLaunchTimeName);
               return;
            }

            if (!m_offlineStorage->StoreSetting(sessionSdkUid, m_sessionSDKUid))
            {
                LOG_WARN("Unable to save session analytics to DB for %s", sessionSdkUid);
                return;
            }
           
        }


    private:
        bool    m_isDBInitialized = false;
        IOfflineStorage *m_offlineStorage;
    };
} ARIASDK_NS_END
#endif
