#define LOG_MODULE DBG_API

#include "../../EXPCommonClient.hpp"
#include "../../JsonHelper.hpp"
#include "AFDConfigCache.hpp"
#include "offline/FifoFileSTorage.hpp"
#include "pal/PAL.hpp"
#include "pal/UtcHelpers.hpp"
#include "AFDClientUtils.hpp"


using namespace Microsoft::Applications::Telemetry::PAL;
using namespace std;

namespace Microsoft {
    namespace Applications {
        namespace Experimentation {
            namespace AFD {
                const unsigned int RetryTimeOutinMiliSec = 5000;
                AFDConfigCache::AFDConfigCache(const string& storagePath)
                    : m_pOfflineStorage(NULL)
                {
                    // save the offline storage path
                    ARIASDK_LOG_DETAIL("[AFDClient]: initializing AFDConfigCache(%s)", storagePath.c_str());

                    //default storage path
                    if (storagePath.find(PATH_SEPARATOR_CHAR) == std::string::npos)
                    {
                        std::string tempDirectroryPath = GetAppLocalTempDirectory();
                        if (!tempDirectroryPath.empty())
                        {
                            m_OfflineStoragePath = tempDirectroryPath + PATH_SEPARATOR_CHAR + storagePath;
                        }
                    }
                    else
                    {
                        m_OfflineStoragePath = storagePath;
                    }

                    // always add a default config
                    AFDConfig config;
                    m_configs[config.requestName] = config;
                }

                AFDConfigCache::~AFDConfigCache()
                {
                    StopAndDestroyOfflineStorage();
                }

                AFDConfig* AFDConfigCache::AddConfig(const AFDConfig& config)
                {
                    m_configs[config.requestName] = config;

                    return GetConfigByRequestName(config.requestName);
                }

                AFDConfig* AFDConfigCache::GetConfigByRequestName(const string& requestName)
                {
                    map<string, AFDConfig>::iterator it = m_configs.find(requestName);

                    return (it == m_configs.end()) ? NULL : &it->second;
                }

                bool AFDConfigCache::LoadConfig()
                {
                    if (m_pOfflineStorage == NULL)
                    {
                        assert(!m_OfflineStoragePath.empty());
                        m_pOfflineStorage = _CreateOfflineStorage(m_OfflineStoragePath);
                        if (m_pOfflineStorage == NULL)
                        {
                            ARIASDK_LOG_ERROR("[AFDClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
                            return false;
                        }
                    }

                    ARIASDK_LOG_DETAIL("[AFDClient]: loading configs from local cache [%s]", m_OfflineStoragePath.c_str());
                    char* buff = NULL;
                    size_t size = 0;

                    int ret = m_pOfflineStorage->PopNextItem(&buff, size, NULL);
                    if (ret < 0)
                    {
                        ARIASDK_LOG_ERROR("[AFDClient]: Failed to load config from local cache");
                        return false;
                    }

                    if (!ret || !size)
                    {
                        ARIASDK_LOG_DETAIL("[AFDClient]: No config found in local cache");
                        return true;
                    }

                    // null-terminate the buffer
                    buff[size - 1] = '\0';

                    json configSettings;
                    // parse the config into json::Variant
                    try
                    {
                        configSettings = json::parse(buff);
                        free(buff);
                        buff = NULL;
                    }
                    catch (...)
                    {
                        free(buff);
                        buff = NULL;
                        ARIASDK_LOG_ERROR("[AFDClient]: Failed to parse config loaded from local cache");
                        return false;
                    }


                    ARIASDK_LOG_DETAIL("[AFDClient]: Config loaded from local cache has successfully parsed");

                    // the parsed config should be json array
                    if (!configSettings.is_array())
                    {
                        ARIASDK_LOG_ERROR("[AFDClient]: Parsed config is of wrong format");
                        return false;
                    }

                    for (auto it = configSettings.begin(); it != configSettings.end(); it++)
                    {
                        AFDConfig config;
                        json jconfig = it.value();

                        auto requestName = jconfig.find("RequestName");
                        if (jconfig.end() == requestName)
                        {
                            // can't get the RequestName for this config, ignore it
                            ARIASDK_LOG_WARNING("[AFDClient]: No RequestName found in this config, skip it.");
                            continue;
                        }

                        config.requestName = requestName.value().get<std::string>();

                        config.expiryUtcTimestamp = 0;
                        auto expiryUtcTimestamp = jconfig.find("expiryUtcTimestamp");
                        if (jconfig.end() != expiryUtcTimestamp)
                        {
                            if (expiryUtcTimestamp.value().is_number_integer())
                            {
                                config.expiryUtcTimestamp = expiryUtcTimestamp.value().get<int>();
                            }
                        }

                        config.clientVersion = "test";
                        auto clientVersion = jconfig.find("clientVersion");
                        if (jconfig.end() != clientVersion)
                        {
                            if (clientVersion.value().is_string())
                            {
                                config.clientVersion = clientVersion.value().get<std::string>();
                            }
                        }

                        auto itFeatures = jconfig.find("Features");

                        if (jconfig.end() != itFeatures)
                        {
                            for (auto itFeature = itFeatures.value().begin(); itFeature != itFeatures.value().end(); ++itFeature)
                            {
                                if (itFeature.value().is_string())
                                {
                                    config.features.push_back(itFeature.value().get<std::string>());
                                }
                            }
                        }

                        auto itFlights = jconfig.find("Flights");

                        if (jconfig.end() != itFlights)
                        {
                            for (auto itFlight = itFlights.value().begin(); itFlight != itFlights.value().end(); ++itFlight)
                            {
                                if (itFlight.value().is_string())
                                {
                                    config.flights.push_back(itFlight.value().get<std::string>());
                                }
                            }
                        }

                        // handle the case where local clock is reset
                        std::int64_t value = PAL::getUtcSystemTime() + Microsoft::Applications::Experimentation::DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX;
                        if (config.expiryUtcTimestamp > value)
                        {
                            config.expiryUtcTimestamp = value;
                        }

                        auto etag = jconfig.find("etag");
                        if (jconfig.end() != etag)
                        {
                            config.etag = etag.value().get<std::string>();
                        }

                        auto configSetting = jconfig.find("configSettings");
                        if (jconfig.end() != configSetting)
                        {
                            config.configSettings = configSetting.value();
                        }

                        // add this config into the in-memory map
                        m_configs[config.requestName] = config;
                    }

                    ARIASDK_LOG_DETAIL("[AFDClient]: Done loading %u configs from offline storage [%s]", m_configs.size(), m_OfflineStoragePath.c_str());
                    StopAndDestroyOfflineStorage();
                    return true;
                }

                // currently, we only save the defauflt config, and the activeConfig
                bool AFDConfigCache::SaveConfig(const AFDConfig& config)
                {
                    ARIASDK_LOG_DETAIL("[AFDClient]: Saving configs to offline storage [%s]", m_OfflineStoragePath.c_str());

                    if (m_pOfflineStorage == NULL)
                    {
                        assert(!m_OfflineStoragePath.empty());
                        m_pOfflineStorage = _CreateOfflineStorage(m_OfflineStoragePath);
                        if (m_pOfflineStorage == NULL)
                        {
                            ARIASDK_LOG_ERROR("[AFDClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
                            return false;
                        }
                    }

                    std::vector<AFDConfig> configsToSave;

                    // Save non-empty configs only
                    configsToSave.push_back(config);

                    std::string configsStr;
                    for (std::vector<AFDConfig>::iterator it = configsToSave.begin(); it != configsToSave.end(); it++)
                    {
                        std::string str;
                        str += "{";
                        // if (!(*it).requestName.empty())
                        {  // serialize 'RequestName'               
                            str += "\"RequestName\":\"";
                            str += (*it).requestName;
                            str += "\"";
                        }

                        if (!(*it).etag.empty())
                        {   // according to http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
                            // section 3.11, an entity tag consists of an opaque quoted string
                            if (!str.empty()) str += ",";
                            str += "\"etag\":\"\\\"";
                            str += (*it).etag;
                            //str.append((*it).etag, 0, (*it).etag.size() - 1);
                            str += "\\\"\"";
                        }

                        // serialize "expiryUtcTimestamp'
                        str += "\",\"expiryUtcTimestamp\":";
                        str += std::to_string((*it).expiryUtcTimestamp);// itoaBuffer;

                        if (!(*it).clientVersion.empty())
                        {
                            if (!str.empty()) str += ",";
                            str += "\"clientVersion\":";
                            str += "\"";
                            str += (*it).clientVersion;
                            str += "\"";
                        }

                        if ((*it).features.size() > 0)
                        {
                            if (!str.empty()) str += ",";
                            str += "\"Features\":[";
                            std::vector<std::string>::iterator iter;
                            bool notfirst = false;
                            for (iter = (*it).features.begin(); iter < (*it).features.end(); iter++)
                            {
                                if (notfirst)
                                {
                                    str += ",";
                                }
                                else
                                {
                                    notfirst = true;
                                }
                                str += "\"";
                                str += *iter;
                                str += "\"";
                            }
                            str += "]";
                        }

                        if ((*it).flights.size() > 0)
                        {
                            if (!str.empty()) str += ",";
                            str += "\"Flights\":[";
                            std::vector<std::string>::iterator iter;
                            bool notfirst = false;
                            for (iter = (*it).flights.begin(); iter < (*it).flights.end(); iter++)
                            {
                                if (notfirst)
                                {
                                    str += ",";
                                }
                                else
                                {
                                    notfirst = true;
                                }
                                str += "\"";
                                str += *iter;
                                str += "\"";
                            }
                            str += "]";
                        }

                        if (!str.empty()) str += ",";
                        // serialize 'configSettings'
                        str += "\"configSettings\":";
                        std::stringstream ss;
                        ss << (*it).configSettings;
                        str += ss.str();
                        str += "}";

                        if (!configsStr.empty())
                        {
                            configsStr += ',';
                        }
                        configsStr += str;
                    }
                    configsStr = "[" + configsStr + "]";

                    // remove existing configs from the offline storage
                    char* buffer = NULL;
                    size_t size = 0;
                    while (true)
                    {
                        int ret = m_pOfflineStorage->PopNextItem(&buffer, size, NULL);
                        if (buffer != NULL)
                        {
                            // Free the memory allocated by PopNextItem to prevent memory leaks
                            free(buffer);
                            buffer = NULL;
                            size = 0;
                        }

                        if (ret < 0)
                        {
                            ARIASDK_LOG_ERROR("[AFDClient]: Failed to clean-up existing configs, abort saving configs.");
                            StopAndDestroyOfflineStorage();
                            return false;
                        }

                        if (ret == 0)
                        {
                            // clean up all
                            break;
                        }
                    }

                    int ret = m_pOfflineStorage->SaveItem(configsStr.c_str(), configsStr.size() + 1, NULL);
                    if (RES_SUCC != ret)
                    {
                        ARIASDK_LOG_ERROR("[AFDClient]: Failed to save configs to offline storage, error=%d).", ret);
                        return false;
                    }

                    ARIASDK_LOG_DETAIL("[AFDClient]: Done saving %u configs to offline storage", configsToSave.size());

                    StopAndDestroyOfflineStorage();
                    return true;
                }

                Microsoft::Applications::Telemetry::IStorage* AFDConfigCache::_CreateOfflineStorage(const string& storagePath)
                {
                    // create offline storage
                    Microsoft::Applications::Telemetry::IStorage* pOfflineStorage = new FIFOFileStorage(); ;
                    if (!pOfflineStorage)
                    {
                        ARIASDK_LOG_ERROR("[AFDClient]: Failed to create offline storage");
                        return NULL;
                    }

                    // initialize offline storage
                    int err = pOfflineStorage->Open(storagePath.c_str(), 0/*sizeLimitInBytes*/);
                    if (err)
                    {
                        ARIASDK_LOG_ERROR("[AFD]: initialize offline storage failed, err: %d", err);
                        delete pOfflineStorage;
                        return NULL;
                    }

                    return pOfflineStorage;
                }

                void AFDConfigCache::StopAndDestroyOfflineStorage()
                {
                    if (m_pOfflineStorage != NULL)
                    {
                        delete m_pOfflineStorage;
                        m_pOfflineStorage = NULL;
                    }
                }

            }
        }
    }
}
