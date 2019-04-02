#pragma once
#include "mat/config.h"
#ifdef HAVE_MAT_EXP

#include "modules/exp/IDataStorage.hpp"

#include "ECSClientConfig.hpp"
#include "pal/PAL.hpp"

#include "json.hpp"

#include <string>
#include <map>

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace ECS {

                // struct to hold an individual config retrieved from ECS
                struct ECSConfig
                {
                    std::string     requestName;
                    std::string     etag;
                    std::int64_t    expiryUtcTimestamp;
                    nlohmann::json  configSettings;
                    std::string     clientVersion;

                    ECSConfig()
                    {
                        etag = DEFAULT_CONFIG_ETAG;
                        expiryUtcTimestamp = PAL::getUtcSystemTime() + DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN;
                    }

                    std::int64_t GetExpiryTimeInSec() const
                    {
                        std::int64_t currUtcTimestamp = PAL::getUtcSystemTime();

                        return (expiryUtcTimestamp <= currUtcTimestamp) ? 0 : (expiryUtcTimestamp - currUtcTimestamp);
                    }

                };

                class ECSConfigCache
                {
                public:
                    ECSConfigCache(const std::string& storagePath);
                    ~ECSConfigCache();

                    bool LoadConfig();
                    bool SaveConfig(const ECSConfig& config);
                    void StopAndDestroyOfflineStorage();

                    ECSConfig* AddConfig(const ECSConfig& config);
                    ECSConfig* GetConfigByRequestName(const std::string& requestName);

                private:
                    MAT::IDataStorage* _CreateOfflineStorage(const std::string& storagePath);
                    bool _LoadConfig();
                    bool _SaveConfig(const ECSConfig& config);
                    std::string m_OfflineStoragePath;
                    MAT::IDataStorage* m_pOfflineStorage;

                    std::map<std::string, ECSConfig> m_configs;
                    std::mutex                       m_lock;

#ifdef _USE_TEST_INJECTION_ECSCLIENT_
                    _USE_TEST_INJECTION_ECSCLIENT_
#endif
                };

            }
        }
    }
}
#endif
