#define LOG_MODULE DBG_API

#pragma unmanaged

#include "ECSConfigCache.hpp"
#include "IOfflineStorage.hpp"
#include "WindowsEnvironmentInfo.h"
#include "common/TraceHelper.hpp"

#include "common/JsonHelper.hpp"
#include "common/Misc.hpp"

#include "ECSClientUtils.hpp"

using namespace common;
using namespace std;

namespace Microsoft { namespace Applications { namespace Experimentation { namespace ECS {

	const unsigned int RetryTimeOutinMiliSec = 5000;
    ECSConfigCache::ECSConfigCache(const string& storagePath)
        : m_pOfflineStorage(NULL)
    {
        // save the offline storage path
        TRACE("[ECSClient]: initializing ECSConfigCache(%s)", storagePath.c_str() );

        //default storage path
        if (storagePath.find(PATH_SEPARATOR) == std::string::npos)
        {
#ifdef _WINRT_DLL
            TRACE("[ECSClient]: Running on WINRT...");
            // This is not a path, but a filename. Windows Store apps must store their data in local storage folder.
            // Prepend the local storage path to filename.
            m_OfflineStoragePath = GetStoragePath() + PATH_SEPARATOR + storagePath;
#else
			std::string tempDirectroryPath = GetAppLocalTempDirectory();
			if (!tempDirectroryPath.empty())
			{
				m_OfflineStoragePath = tempDirectroryPath + SPL_PATH_SEPARATOR_CHAR + storagePath;
			}
#endif
        } 
        else
        {
            m_OfflineStoragePath = storagePath;
        }

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
                LOG_ERROR("[ECSClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
                return false;
            }
        }

        TRACE("[ECSClient]: loading configs from local cache [%s]", m_OfflineStoragePath.c_str());
        char* buff = NULL;
        size_t size = 0;

        int ret = m_pOfflineStorage->PopNextItem(&buff, size, NULL);
        if (ret < 0)
        {
            LOG_ERROR("[ECSClient]: Failed to load config from local cache");
            return false;
        }

        if (!ret || !size)
        {
            TRACE("[ECSClient]: No config found in local cache");
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
			LOG_ERROR("[ECSClient]: Failed to parse config loaded from local cache");
			return false;
		}


        TRACE("[ECSClient]: Config loaded from local cache has successfully parsed");

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
			std::int64_t value = common::GetCurrentTimeStamp() + DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MAX;
			if (config.expiryUtcTimestamp > value)
			{
				config.expiryUtcTimestamp = value;
			}

			auto etag = jconfig.find("etag");
			if (jconfig.end() != etag)
			{
				config.etag = etag.value().get<std::string>();
			}

			auto configSettings = jconfig.find("configSettings");
			if (jconfig.end() != configSettings)
			{
				config.configSettings = configSettings.value();
			}

			// add this config into the in-memory map
			m_configs[config.requestName] = config;
        }

        TRACE("[ECSClient]: Done loading %u configs from offline storage [%s]", m_configs.size(), m_OfflineStoragePath.c_str());
		StopAndDestroyOfflineStorage();
        return true;
    }

    // currently, we only save the defauflt config, and the activeConfig
    bool ECSConfigCache::SaveConfig(const ECSConfig& config)
    {
        TRACE("[ECSClient]: Saving configs to offline storage [%s]", m_OfflineStoragePath.c_str());

        if (m_pOfflineStorage == NULL)
        {
            assert(!m_OfflineStoragePath.empty());
            m_pOfflineStorage = _CreateOfflineStorage(m_OfflineStoragePath);
            if (m_pOfflineStorage == NULL)
            {
                LOG_ERROR("[ECSClient]: Failed to create offline storage at [%s]", m_OfflineStoragePath.c_str());
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
                str += "\",\"etag\":\"\\";
                str.append((*it).etag, 0, (*it).etag.size() - 1);
                str += "\\\"";
            }

            // serialize "expiryUtcTimestamp'
            str += "\",\"expiryUtcTimestamp\":";
            static char itoaBuffer[20] = {};
            sprintf(itoaBuffer, "%lld", (int64_t)(*it).expiryUtcTimestamp);
            str += itoaBuffer;

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
                LOG_ERROR("[ECSClient]: Failed to clean-up existing configs, abort saving configs.");
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
            LOG_ERROR("[ECSClient]: Failed to save configs to offline storage, error=%d).", ret);
            return false;
        }

        TRACE("[ECSClient]: Done saving %u configs to offline storage", configsToSave.size());
		StopAndDestroyOfflineStorage();
        return true;
    }

    IOfflineStorage* ECSConfigCache::_CreateOfflineStorage(const string& storagePath)
    {
        // create offline storage
        IOfflineStorage* pOfflineStorage = IOfflineStorage::Create();
        if (!pOfflineStorage)
        {
            LOG_ERROR("[ECSClient]: Failed to create offline storage");
            return NULL;
        }

        // initialize offline storage
        int err = pOfflineStorage->Initialize(storagePath.c_str(), 0/*sizeLimitInBytes*/);
        if (err)
        {
            LOG_ERROR("[ECS]: initialize offline storage failed, err: %d", err);
            IOfflineStorage::Destroy(pOfflineStorage);
            return NULL;
        }

        // start offline storage
        err = pOfflineStorage->Start(RetryTimeOutinMiliSec);
        if (err)
        {
            LOG_ERROR("[ECS]: Failed to start offline storage, error=%d", err);
            IOfflineStorage::Destroy(pOfflineStorage);
            return NULL;
        }

        return pOfflineStorage;
    }

    void ECSConfigCache::StopAndDestroyOfflineStorage()
    {
        if (m_pOfflineStorage != NULL)
        {
            m_pOfflineStorage->Stop();
            IOfflineStorage::Destroy(m_pOfflineStorage);

            m_pOfflineStorage = NULL;
        }
    }

}}}}

