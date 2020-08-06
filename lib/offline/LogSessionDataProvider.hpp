#ifndef MAT_LOGSESSIONDATA_PROVIDER_HPP
#define MAT_LOGSESSIONDATA_PROVIDER_HPP

#include "LogSessionData.hpp"
#include "IOfflineStorage.hpp"
#include <string>
#include <memory>

namespace ARIASDK_NS_BEGIN
{
    enum class SessionStorageType : uint8_t {
        FileStore,
        DatabaseStore  
    };

    class  LogSessionDataProvider
    {
    public:
        LogSessionDataProvider(std::shared_ptr<IOfflineStorage> offlineStorage):
            m_storageType(SessionStorageType::DatabaseStore),
            m_offlineStorage(offlineStorage)
        {
        }

        LogSessionDataProvider(std::string const& cacheFilePath):
            m_storageType(SessionStorageType::FileStore),
            m_cacheFilePath(cacheFilePath)
        {
        }

        std::shared_ptr<LogSessionData> GetLogSessionData();

    protected:
        std::shared_ptr<LogSessionData> GetLogSessionDataFromFile();
        std::shared_ptr<LogSessionData> GetLogSessionDataFromDB();
        bool parse(const std::string &, uint64_t &,  std::string &) ;

    private:
        SessionStorageType m_storageType;
        std::shared_ptr<IOfflineStorage> m_offlineStorage;
        std::string const m_cacheFilePath;
        uint64_t convertStrToLong(const std::string& );
        void writeFileContents(const std::string &, uint64_t, const std::string &);
        void remove_eol(std::string& );
    };
}
ARIASDK_NS_END
#endif



