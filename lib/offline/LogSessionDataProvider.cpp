//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "LogSessionDataProvider.hpp"
#include "utils/FileUtils.hpp"
#include "utils/StringUtils.hpp"
#include "utils/Utils.hpp"

#include<cstring>
#include<cstdlib>
#include<errno.h>

#ifndef _WIN32  /* Avoid warning under Windows */
extern int errno;  
#endif

namespace MAT_NS_BEGIN
{
    static const char* sessionFirstLaunchTimeName = "sessionfirstlaunchtime";
    static const char* sessionSdkUidName = "sessionsdkuid";

    void LogSessionDataProvider::CreateLogSessionData()
    {
        if (m_storageType == SessionStorageType::FileStore)
        {
            CreateLogSessionDataFromFile();
        } else 
        {
            CreateLogSessionDataFromDB();
        }
    }

    void LogSessionDataProvider::ResetLogSessionData()
    {
        DeleteLogSessionData();
        CreateLogSessionData();
    }

    void LogSessionDataProvider::DeleteLogSessionData()
    {
        if (m_storageType == SessionStorageType::FileStore)
        {
            DeleteLogSessionDataFromFile();
        } 
        else
        {
            DeleteLogSessionDataFromDB();
        }
    }

    LogSessionData* LogSessionDataProvider::GetLogSessionData() 
    {
        return m_logSessionData.get();
    }

    void  LogSessionDataProvider::CreateLogSessionDataFromDB()
    {
        if (nullptr == m_offlineStorage) {
            LOG_WARN(" offline storage not available. Session data won't be initialized");
            return;
        }
        uint64_t sessionFirstTimeLaunch = 0;
        std::string sessionSDKUid = m_offlineStorage->GetSetting(sessionSdkUidName);
        sessionFirstTimeLaunch = convertStrToLong(m_offlineStorage->GetSetting(sessionFirstLaunchTimeName));
        if ((sessionFirstTimeLaunch == 0) || sessionSDKUid.empty())
        {
            sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
            sessionSDKUid = PAL::generateUuidString();
            if (!m_offlineStorage->StoreSetting(sessionFirstLaunchTimeName, std::to_string(sessionFirstTimeLaunch))) 
            {
                LOG_WARN("Unable to save session analytics to DB for %d", sessionFirstLaunchTimeName);
            }
            if (!m_offlineStorage->StoreSetting(sessionSdkUidName, sessionSDKUid)) {
                LOG_WARN("Unable to save session analytics to DB for %s", sessionSDKUid.c_str());
            }
        }
        m_logSessionData.reset(new LogSessionData(sessionFirstTimeLaunch, sessionSDKUid));
    }


    void LogSessionDataProvider::DeleteLogSessionDataFromDB()
    {
        if (nullptr == m_offlineStorage) {
            LOG_WARN(" offline storage not available. Session data won't be deleted");
            return ;
        }
        if (!m_offlineStorage->DeleteSetting(sessionFirstLaunchTimeName))
        {
            LOG_WARN("Unable to delete session analytics from DB for %d", sessionFirstLaunchTimeName);
        }
        if (!m_offlineStorage->DeleteSetting(sessionSdkUidName))
        {
            LOG_WARN("Unable to delete session analytics from DB for %d", sessionSdkUidName);
        }
    }

    void LogSessionDataProvider::DeleteLogSessionDataFromFile()
    {
        std::string sessionPath = m_cacheFilePath.empty() ? "" : (m_cacheFilePath + ".ses").c_str();
        if (!sessionPath.empty() && MAT::FileExists(sessionPath.c_str()))
        {
            MAT::FileDelete(sessionPath.c_str());
        }
    }

    void LogSessionDataProvider::CreateLogSessionDataFromFile()
    {
        uint64_t sessionFirstTimeLaunch = 0;
        std::string sessionSDKUid;
        std::string sessionPath = m_cacheFilePath.empty() ? "" : (m_cacheFilePath + ".ses").c_str();
        if (!sessionPath.empty()) 
        {
            if (MAT::FileExists(sessionPath.c_str())) 
            {
                auto content = MAT::FileGetContents(sessionPath.c_str());
                if (!parse (content, sessionFirstTimeLaunch, sessionSDKUid)) 
                {
                    sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
                    sessionSDKUid = PAL::generateUuidString();
                    writeFileContents(sessionPath, sessionFirstTimeLaunch, sessionSDKUid);
                }
            } else
            {
                sessionFirstTimeLaunch = PAL::getUtcSystemTimeMs();
                sessionSDKUid = PAL::generateUuidString();
                writeFileContents(sessionPath, sessionFirstTimeLaunch, sessionSDKUid);
            }
        }
        m_logSessionData.reset(new LogSessionData(sessionFirstTimeLaunch, sessionSDKUid));
    }

    bool LogSessionDataProvider::parse(
        const std::string &content,
        uint64_t& sessionFirstTimeLaunch,
        std::string& sessionSDKUid)
    {
        if (content.empty()) {
            return false;
        }
        std::vector<std::string> v;
        StringUtils::SplitString(content, '\n', v);
        if (v.size() != 3) {
           return false;
        }
        remove_eol(v[0]);
        remove_eol(v[1]);
        sessionFirstTimeLaunch = convertStrToLong(v[0]);
        if (sessionFirstTimeLaunch == 0 ) {
            return false;
        }
        sessionSDKUid =  v[1];
        return true;
    }

    uint64_t LogSessionDataProvider::convertStrToLong(const std::string& s)
    {
        uint64_t res = 0ull;
        char *endptr = nullptr;
        res = std::strtoll(s.c_str(), &endptr, 10);
        if (errno == ERANGE && (res == LONG_MAX || res == 0 ))
        {
            LOG_WARN ("Converted value falls out of uint64_t range.");
            res = 0;
        } 
        else if ( 0 != errno  && 0 == res )
        {
            LOG_WARN("Conversion cannot be performed.");
        }
        else if (std::strlen(endptr) > 0)
        {
            LOG_WARN ("Conversion cannot be performed. Alphanumeric characters present");
            res = 0;
        } 
        return res;
    }

    void LogSessionDataProvider::writeFileContents(
        const std::string &path,
        uint64_t sessionFirstTimeLaunch,
        const std::string &sessionSDKUid)
    {
        std::string contents;
        contents += toString(sessionFirstTimeLaunch);
        contents += '\n';
        contents += sessionSDKUid;
        // Valid line ends with newline as per posix specs ( https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_206)
        contents += '\n';
        //TBD (labhas) - validate if file is NOT a symlink/junction before trying to write.
        if (!MAT::FileWrite(path.c_str(), contents.c_str()))
        {
            LOG_WARN("Unable to save session analytics to %s", path.c_str());
        }
    }

    void LogSessionDataProvider::remove_eol(std::string& result)
    {
        if (!result.empty() && result[result.length() - 1] == '\n')
        {
            result.erase(result.length() - 1);
        }
    }
}
MAT_NS_END

