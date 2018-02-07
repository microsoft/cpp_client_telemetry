#pragma once

#include "AFDClientConfig.hpp"
#include "offline/IStorage.hpp"
#include "pal/PAL.hpp"
#include <json.hpp>
#include <string>
#include <map>


namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace AFD {

                // struct to hold an individual config retrieved from AFD
                struct AFDConfig
                {
                    std::string     requestName;
                    std::string     etag;
                    std::int64_t      expiryUtcTimestamp;
                    std::int64_t    flightingVersion;
                    nlohmann::json    configSettings;
                    std::string     clientVersion;
                    std::vector<std::string>     features;
                    std::vector<std::string>     flights;
                    std::map<std::string, std::string> configs;
                    
                    AFDConfig()
                    {
                        etag = DEFAULT_CONFIG_ETAG;
                        expiryUtcTimestamp = Microsoft::Applications::Events::PAL::getUtcSystemTime() + DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN;
                        flightingVersion = 0;
                    }

                    std::int64_t GetExpiryTimeInSec() const
                    {
                        std::int64_t currUtcTimestamp = Microsoft::Applications::Events::PAL::getUtcSystemTime();
                        return (expiryUtcTimestamp <= currUtcTimestamp) ? 0 : (expiryUtcTimestamp - currUtcTimestamp);
                    }

                };

                class AFDConfigCache
                {
                public:
                    AFDConfigCache(const std::string& storagePath);
                    ~AFDConfigCache();

                    bool LoadConfig();
                    bool SaveConfig(const AFDConfig& config);
                    void StopAndDestroyOfflineStorage();

                    AFDConfig* AddConfig(const AFDConfig& config);
                    AFDConfig* GetConfigByRequestName(const std::string& requestName);

                private:
                    Microsoft::Applications::Events::IStorage* _CreateOfflineStorage(const std::string& storagePath);
                    bool _LoadConfig();
                    bool _SaveConfig(const AFDConfig& config);

                    std::string m_OfflineStoragePath;
                    Microsoft::Applications::Events::IStorage* m_pOfflineStorage;

                    std::map<std::string, AFDConfig> m_configs;
                    std::mutex                         m_lock;

#ifdef _USE_TEST_INJECTION_AFDCLIENT_
                    _USE_TEST_INJECTION_AFDCLIENT_
#endif
                };
            }
        }
    }
}