#ifndef MAT_LOGSESSIONDATA_PROVIDER_HPP
#define MAT_LOGSESSIONDATA_PROVIDER_HPP

#include "LogSessionData.hpp"
#include "IOfflineStorage.hpp"
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
            IOfflineStorage* offlineStorage)
            :
            m_logSessionData(nullptr),
            m_offlineStorage(offlineStorage),
            m_storageType(SessionStorageType::DatabaseStore)
        {
        }

        LogSessionDataProvider(
            std::string const& cacheFilePath)
            :
            m_logSessionData(nullptr),
            m_cacheFilePath(cacheFilePath),
            m_storageType(SessionStorageType::FileStore)
        {
        }

        void CreateLogSessionData();
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
        SessionStorageType m_storageType;
        std::unique_ptr<LogSessionData> m_logSessionData;
        uint64_t convertStrToLong(const std::string&);
        void writeFileContents(const std::string&, uint64_t, const std::string&);
        void remove_eol(std::string& );
    };
}
MAT_NS_END
#endif
