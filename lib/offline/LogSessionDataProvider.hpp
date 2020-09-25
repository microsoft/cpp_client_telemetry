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
            bool disableSdkUid = false)
            :
            m_offlineStorage(offlineStorage),
            m_storageType(SessionStorageType::DatabaseStore),
            m_disableSdkUid(disableSdkUid),
            m_logSessionData(nullptr)
        {
        }

        LogSessionDataProvider(
            std::string const& cacheFilePath,
            bool disableSdkUid = false)
            :
            m_cacheFilePath(cacheFilePath),
            m_storageType(SessionStorageType::FileStore),
            m_disableSdkUid(disableSdkUid),
            m_logSessionData(nullptr)
        {
        }

        void CreateLogSessionData();
        LogSessionData *GetLogSessionData();

    protected:
        void CreateLogSessionDataFromFile();
        void CreateLogSessionDataFromDB();
        bool parse(const std::string&, uint64_t&,  std::string&) ;

    private:
        IOfflineStorage* m_offlineStorage; //Pointer is not owned. Do not delete!
        std::string const m_cacheFilePath;
        SessionStorageType m_storageType;
        bool m_disableSdkUid;
        std::unique_ptr<LogSessionData> m_logSessionData;
        uint64_t convertStrToLong(const std::string&);
        void writeFileContents(const std::string&, uint64_t, const std::string&);
        void remove_eol(std::string& );
    };
}
MAT_NS_END
#endif
