#define LOG_MODULE DBG_API

#include "../../EXPCommonClient.hpp"
#include "../../JsonHelper.hpp"
#include "ECSConfigCache.hpp"
#include "offline/FifoFileSTorage.hpp"
#include "pal/UtcHelpers.hpp"
#include "ECSClientUtils.hpp"

using namespace PAL;
using namespace std;

namespace Microsoft { namespace Applications { namespace Experimentation { namespace ECS {

    const unsigned int RetryTimeOutinMiliSec = 5000;
    ECSConfigCache::ECSConfigCache(const string& storagePath)
        : m_pOfflineStorage(NULL)
    {
        // save the offline storage path
        LOG_TRACE("[ECSClient]: initializing ECSConfigCache(%s)", storagePath.c_str() );

        //default storage path
        if (storagePath.find(PATH_SEPARATOR_CHAR) == std::string::npos)
        {
            std::string tempDirectroryPath = GetAppLocalTempDirectory();
            if (!tempDirectroryPath.empty())
            {
                m_OfflineStoragePath = tempDirectroryPath + storagePath;
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
        bool retrunValue = false;
        if (m_pOfflineStorage == NULL)
        {
            assert(!m_OfflineStoragePath.empty());
            m_pOfflineStorage = _CreateOfflineStorage(m_OfflineStoragePath);
            if (m_pOfflineStorage != NULL)
            {
                retrunValue = _LoadConfig();
                StopAndDestroyOfflineStorage();
            }
            else
            {
                LOG_ERROR("[ECSClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
            }
        }
        return retrunValue;
    }
    bool ECSConfigCache::_LoadConfig()
    {        
        LOG_TRACE("[ECSClient]: loading configs from local cache [%s]", m_OfflineStoragePath.c_str());
        char* buff = NULL;
        size_t size = 0;

        {
            std::lock_guard<std::mutex> lock(m_lock);
            if (m_pOfflineStorage)
            {
                int ret = m_pOfflineStorage->PopNextItem(&buff, size, NULL);
                if (ret < 0)
                {
                    LOG_ERROR("[ECSClient]: Failed to load config from local cache");
                    return false;
                }

                if (!ret || !size)
                {
                    LOG_TRACE("[ECSClient]: No config found in local cache");
                    return true;
                }
            }
        }

        // null-terminate the buffer
        buff[size - 1] = '\0';

        json configSettings;
        // parse the config into json::Variant
        try
        {
            configSettings = json::parse(buff);
            delete [] buff;
            buff = NULL;
        }
        catch (...)
        {
            delete[] buff;
            buff = NULL;
            LOG_ERROR("[ECSClient]: Failed to parse config loaded from local cache");
            return false;
        }


        LOG_TRACE("[ECSClient]: Config loaded from local cache has successfully parsed");

        // the parsed config should be json array
        if (!configSettings.is_array())
        {
            LOG_ERROR("[ECSClient]: Parsed config is of wrong format");
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
                LOG_WARN("[ECSClient]: No RequestName found in this config, skip it.");
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
            std::int64_t value = PAL::getUtcSystemTime() + DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX;
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

        LOG_TRACE("[ECSClient]: Done loading %u configs from offline storage [%s]", m_configs.size(), m_OfflineStoragePath.c_str());
        
        return true;
    }

    // currently, we only save the defauflt config, and the activeConfig
    bool ECSConfigCache::SaveConfig(const ECSConfig& config)
    {
        bool returnValue = false;
        if (m_pOfflineStorage == NULL)
        {
            assert(!m_OfflineStoragePath.empty());
            m_pOfflineStorage = _CreateOfflineStorage(m_OfflineStoragePath);
            if (m_pOfflineStorage != NULL)
            {
                returnValue = _SaveConfig(config);
                StopAndDestroyOfflineStorage();
            }
            else
            {
                LOG_ERROR("[ECSClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
                return false;
            }
        }
        return returnValue;
    }
    // currently, we only save the defauflt config, and the activeConfig
    bool ECSConfigCache::_SaveConfig(const ECSConfig& config)
    {
        LOG_TRACE("[ECSClient]: Saving configs to offline storage [%s]", m_OfflineStoragePath.c_str());        
        
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
            str += "\"";

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

        {
            std::lock_guard<std::mutex> lock(m_lock);
            if (m_pOfflineStorage)
            {
                // remove existing configs from the offline storage
                char* buffer = NULL;
                size_t size = 0;
                int ret = 0;
                while (true)
                {
                    ret = m_pOfflineStorage->PopNextItem(&buffer, size, NULL);
                    if (buffer != NULL)
                    {
                        // Free the memory allocated by PopNextItem to prevent memory leaks
                        free(buffer);
                        buffer = NULL;
                        size = 0;
                    }

                    if (ret < 0)
                    {
                        LOG_ERROR("[ECSClient]: Failed to clean-up existing configs, abort saving configs.");
                        break;
                    }

                    if (ret == 0)
                    {
                        // clean up all
                        break;
                    }
                }

                if (MAT::RES_SUCC == ret)
                {
                    ret = m_pOfflineStorage->SaveItem(configsStr.c_str(), configsStr.size() + 1, NULL);
                    if (MAT::RES_SUCC != ret)
                    {
                        LOG_ERROR("[ECSClient]: Failed to save configs to offline storage, error=%d).", ret);
                    }
                }
            }
        }
        LOG_TRACE("[ECSClient]: Done saving %u configs to offline storage", configsToSave.size());
        
        return true;
    }

    ARIASDK_NS::IStorage* ECSConfigCache::_CreateOfflineStorage(const string& storagePath)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        // create offline storage
        ARIASDK_NS::IStorage* pOfflineStorage = new MAT::FIFOFileStorage();
        if (!pOfflineStorage)
        {
            LOG_ERROR("[ECSClient]: Failed to create offline storage");
            return NULL;
        }

        // initialize offline storage
        int err = pOfflineStorage->Open(storagePath.c_str(), 0/*sizeLimitInBytes*/);
        if (err)
        {
            LOG_ERROR("[ECS]: initialize offline storage failed, err: %d", err);
            delete pOfflineStorage;
            return NULL;
        }

        return pOfflineStorage;
    }

    void ECSConfigCache::StopAndDestroyOfflineStorage()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        if (m_pOfflineStorage != NULL)
        {
            delete m_pOfflineStorage;
            m_pOfflineStorage = NULL;
        }
    }

}}}}

