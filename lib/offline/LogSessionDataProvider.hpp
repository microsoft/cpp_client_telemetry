//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_LOGSESSIONDATA_PROVIDER_HPP
#define MAT_LOGSESSIONDATA_PROVIDER_HPP

#include "LogSessionData.hpp"
#include "IOfflineStorage.hpp"
#include "api/IRuntimeConfig.hpp"
#include <string>
#include <memory>

namespace MAT_NS_BEGIN
{
    enum class SessionStorageType : uint8_t {
        FileStore,
        DatabaseStore  
    };

    class  LogSessionDataProvider
    {
    public:

        LogSessionDataProvider(
            IOfflineStorage* offlineStorage,
            bool disableSdkUid)
            :
            m_offlineStorage(offlineStorage),
            m_storageType(SessionStorageType::DatabaseStore),
            m_logSessionData(nullptr),
            m_disableSdkUid(disableSdkUid)

        {
        }

        LogSessionDataProvider(
            IOfflineStorage* offlineStorage) 
            : 
            LogSessionDataProvider(offlineStorage, false)
        {
        }

        LogSessionDataProvider(
            std::string const& cacheFilePath,
            bool disableSdkUid)
            :
            m_cacheFilePath(cacheFilePath),
            m_storageType(SessionStorageType::FileStore),
            m_logSessionData(nullptr),
            m_disableSdkUid(disableSdkUid)
        {
        }

        LogSessionDataProvider(
            std::string const& cacheFilePath) 
            : 
            LogSessionDataProvider(cacheFilePath, false)
        {
        }

        void CreateLogSessionData();
        void ResetLogSessionData();
        void DeleteLogSessionData();
        LogSessionData *GetLogSessionData();

    protected:
        void CreateLogSessionDataFromFile();
        void CreateLogSessionDataFromDB();
        void DeleteLogSessionDataFromFile();
        void DeleteLogSessionDataFromDB();
        bool parse(const std::string&, uint64_t&,  std::string&) ;

    private:
        IOfflineStorage* m_offlineStorage; //Pointer is not owned. Do not delete!
        std::string const m_cacheFilePath;
        bool m_disableSdkUid;
        SessionStorageType m_storageType;
        std::unique_ptr<LogSessionData> m_logSessionData;
        uint64_t convertStrToLong(const std::string&);
        void writeFileContents(const std::string&, uint64_t, const std::string&);
        void remove_eol(std::string& );
    };
}
MAT_NS_END
#endif

