#pragma once

#include "AFDClientConfig.hpp"
#include "WindowsEnvironmentInfo.h"
#include <json.hpp>
#include <string>
#include <map>

// forward-declaration
namespace common
{
    class IOfflineStorage;
}

namespace Microsoft { namespace Applications { namespace Experimentation { namespace AFD {

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

    AFDConfig()
    {
        etag = DEFAULT_CONFIG_ETAG;
        expiryUtcTimestamp = 0;
		flightingVersion = 0;
    }

	std::int64_t GetExpiryTimeInSec() const
    {

		std::int64_t currUtcTimestamp = common::GetCurrentTimeStamp();

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
    common::IOfflineStorage* _CreateOfflineStorage(const std::string& storagePath);

private:
    std::string m_OfflineStoragePath;
    common::IOfflineStorage* m_pOfflineStorage;

    std::map<std::string, AFDConfig> m_configs;

#ifdef _USE_TEST_INJECTION_AFDCLIENT_
    _USE_TEST_INJECTION_AFDCLIENT_
#endif
};

}}}}