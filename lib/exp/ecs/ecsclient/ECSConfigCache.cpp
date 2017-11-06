#define LOG_MODULE DBG_API

#include "../../EXPCommonClient.hpp"
#include "../../JsonHelper.hpp"
#include "ECSConfigCache.hpp"
#include "offline/FifoFileSTorage.hpp"
#include "pal/UtcHelpers.hpp"
#include "ECSClientUtils.hpp"

using namespace Microsoft::Applications::Telemetry::PAL;
using namespace std;

namespace Microsoft { namespace Applications { namespace Experimentation { namespace ECS {

    const unsigned int RetryTimeOutinMiliSec = 5000;
    ECSConfigCache::ECSConfigCache(const string& storagePath)
        : m_pOfflineStorage(NULL)
    {
        // save the offline storage path
        ARIASDK_LOG_DETAIL("[ECSClient]: initializing ECSConfigCache(%s)", storagePath.c_str() );

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

        int* xPtr = nullptr;
        int IntptrSize = sizeof(xPtr);
        if (IntptrSize > 4) // on 64 bit system, we want session to have different file because FIFO has trouble opening 32 bit file in 64 bit mode
        {
            m_OfflineStoragePath = m_OfflineStoragePath + "64";
        }
        UNREFERENCED_PARAMETER(xPtr);

        // always add a default config
        ECSConfig config;
        m_configs[config.requestName] = config;
    }

    ECSConfigCache::~ECSConfigCache()
    {
        StopAndDestroyOfflineStorage();
    }

    ECSConfig* ECSConfigCache::AddConfig(const ECSConfig& config)
    {
        m_configs[config.requestName] = config;

        return GetConfigByRequestName(config.requestName);
    }

    ECSConfig* ECSConfigCache::GetConfigByRequestName(const string& requestName)
    {
        map<string, ECSConfig>::iterator it = m_configs.find(requestName);

        return (it == m_configs.end()) ? NULL : &it->second;
    }

    bool ECSConfigCache::LoadConfig()
    {
        if (m_pOfflineStorage == NULL)
        {
            assert(!m_OfflineStoragePath.empty());
            m_pOfflineStorage = _CreateOfflineStorage(m_OfflineStoragePath);
            if (m_pOfflineStorage == NULL)
            {
                ARIASDK_LOG_ERROR("[ECSClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
                return false;
            }
        }

        ARIASDK_LOG_DETAIL("[ECSClient]: loading configs from local cache [%s]", m_OfflineStoragePath.c_str());
        char* buff = NULL;
        size_t size = 0;

        int ret = m_pOfflineStorage->PopNextItem(&buff, size, NULL);
        if (ret < 0)
        {
            ARIASDK_LOG_ERROR("[ECSClient]: Failed to load config from local cache");
            return false;
        }

        if (!ret || !size)
        {
            ARIASDK_LOG_DETAIL("[ECSClient]: No config found in local cache");
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
            ARIASDK_LOG_ERROR("[ECSClient]: Failed to parse config loaded from local cache");
            return false;
        }


        ARIASDK_LOG_DETAIL("[ECSClient]: Config loaded from local cache has successfully parsed");

        // the parsed config should be json array
        if (!configSettings.is_array())
        {
            ARIASDK_LOG_ERROR("[ECSClient]: Parsed config is of wrong format");
            return false;
        }

        for (auto it = configSettings.begin(); it != configSettings.end(); it++)
        {
            ECSConfig config;
            json jconfig = it.value();

            auto requestName = jconfig.find("RequestName");
            if (jconfig.end() == requestName)
            {
                // can't get the RequestName for this config, ignore it
                ARIASDK_LOG_WARNING("[ECSClient]: No RequestName found in this config, skip it.");
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

            // handle the case where local clock is reset
            std::int64_t value = Microsoft::Applications::Telemetry::PAL::getUtcSystemTime() + DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX;
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

        ARIASDK_LOG_DETAIL("[ECSClient]: Done loading %u configs from offline storage [%s]", m_configs.size(), m_OfflineStoragePath.c_str());
        StopAndDestroyOfflineStorage();
        return true;
    }

    // currently, we only save the defauflt config, and the activeConfig
    bool ECSConfigCache::SaveConfig(const ECSConfig& config)
    {
        ARIASDK_LOG_DETAIL("[ECSClient]: Saving configs to offline storage [%s]", m_OfflineStoragePath.c_str());

        if (m_pOfflineStorage == NULL)
        {
            assert(!m_OfflineStoragePath.empty());
            m_pOfflineStorage = _CreateOfflineStorage(m_OfflineStoragePath);
            if (m_pOfflineStorage == NULL)
            {
                ARIASDK_LOG_ERROR("[ECSClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
                return false;
            }
        }

        std::vector<ECSConfig> configsToSave;

        // Save non-empty configs only
        configsToSave.push_back(config);

        std::string configsStr;
        for (std::vector<ECSConfig>::iterator it = configsToSave.begin(); it != configsToSave.end(); it++)
        {
            // serialize 'RequestName'
            std::string str;
            str += "{\"RequestName\":\"";
            str += (*it).requestName;

            if (!(*it).etag.empty())
            {
                // according to http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
                // section 3.11, an entity tag consists of an opaque quoted string
                if (!str.empty()) str += ",";
                str += "\"etag\":\"\\";
                str.append((*it).etag, 0, (*it).etag.size() - 1);
                str += "\\\"";
            }

            // serialize "expiryUtcTimestamp'
            str += "\",\"expiryUtcTimestamp\":";
            str += std::to_string((*it).expiryUtcTimestamp);

            // serialize "clientVersion'
            str += ",\"clientVersion\":";
            str += "\"";
            str += (*it).clientVersion; 
            str += "\"";
            
            // serialize 'configSettings'
            str += ",\"configSettings\":";
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
                ARIASDK_LOG_ERROR("[ECSClient]: Failed to clean-up existing configs, abort saving configs.");
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
        if (Microsoft::Applications::Telemetry::RES_SUCC != ret)
        {
            ARIASDK_LOG_ERROR("[ECSClient]: Failed to save configs to offline storage, error=%d).", ret);
            return false;
        }

        ARIASDK_LOG_DETAIL("[ECSClient]: Done saving %u configs to offline storage", configsToSave.size());
        StopAndDestroyOfflineStorage();
        return true;
    }

    ARIASDK_NS::IStorage* ECSConfigCache::_CreateOfflineStorage(const string& storagePath)
    {
        // create offline storage
        ARIASDK_NS::IStorage* pOfflineStorage = new Microsoft::Applications::Telemetry::FIFOFileStorage();
        if (!pOfflineStorage)
        {
            ARIASDK_LOG_ERROR("[ECSClient]: Failed to create offline storage");
            return NULL;
        }

        // initialize offline storage
        int err = pOfflineStorage->Open(storagePath.c_str(), 0/*sizeLimitInBytes*/);
        if (err)
        {
            ARIASDK_LOG_ERROR("[ECS]: initialize offline storage failed, err: %d", err);
            delete pOfflineStorage;
            return NULL;
        }

        return pOfflineStorage;
    }

    void ECSConfigCache::StopAndDestroyOfflineStorage()
    {
        if (m_pOfflineStorage != NULL)
        {
            delete m_pOfflineStorage;
            m_pOfflineStorage = NULL;
        }
    }

}}}}

