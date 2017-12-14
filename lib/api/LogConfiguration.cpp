#include "LogConfiguration.hpp"
#include "IHttpClient.hpp"
#include "IRuntimeConfig.hpp"
#include "IBandwidthController.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "pal/PAL.hpp"


namespace ARIASDK_NS_BEGIN {

    EVTStatus LogConfiguration::SetMinimumTraceLevel(ACTTraceLevel minimumTraceLevel)
    {
        m_minimumTraceLevel = minimumTraceLevel;
        return  EVTStatus::EVTStatus_OK;
    }

    ACTTraceLevel LogConfiguration::GetMinimumTraceLevel() const
    {
        return m_minimumTraceLevel;
    }
    EVTStatus LogConfiguration::SetSdkModeType(SdkModeTypes sdkmode)
    {
        m_sdkmode = sdkmode;
        return  EVTStatus::EVTStatus_OK;
    }
    SdkModeTypes LogConfiguration::GetSdkModeType() const
    {
        return m_sdkmode;
    }

    EVTStatus LogConfiguration::SetProperty(char const* key, char const* value)
    {
        if (nullptr != key && nullptr != value)
        {
            std::string keyString(key);
            if (!keyString.empty())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                strProps[keyString] = value;
                return  EVTStatus::EVTStatus_OK;
            }
        }
        return  EVTStatus::EVTStatus_Fail;
    }
    EVTStatus LogConfiguration::SetIntProperty(char const* key, unsigned int value)
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                intProps[keyString] = value;
                return  EVTStatus::EVTStatus_OK;
            }
        }
        return  EVTStatus::EVTStatus_Fail;
    }
    EVTStatus LogConfiguration::SetBoolProperty(char const* key, bool value)
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                boolProps[keyString] = value;
                return  EVTStatus::EVTStatus_OK;
            }
        }
        return  EVTStatus::EVTStatus_Fail;
    }
    EVTStatus LogConfiguration::SetPointerProperty(char const* key, void* value)
    {
        UNREFERENCED_PARAMETER(key);
        UNREFERENCED_PARAMETER(value);
        return  EVTStatus::EVTStatus_OK;
    }
   
    char const* LogConfiguration::GetProperty(char const* key, EVTStatus& error) const
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty() && strProps.find(keyString) != strProps.end())
            {
                error = EVTStatus::EVTStatus_OK;
                return strProps.at(keyString).c_str();
            }
        }
        error =  EVTStatus::EVTStatus_Fail;
        return "";
    }

    uint32_t LogConfiguration::GetIntProperty(char const* key, EVTStatus& error) const
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty() && intProps.find(keyString) != intProps.end())
            {
                error = EVTStatus::EVTStatus_OK;
                return intProps.at(keyString);
            }
        }
        error = EVTStatus::EVTStatus_Fail;
        return 0;
    }
    bool LogConfiguration::GetBoolProperty(char const* key, EVTStatus& error) const
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty() && boolProps.find(keyString) != boolProps.end())
            {
                error = EVTStatus::EVTStatus_OK;
                return boolProps.at(keyString);
            }
        }
        error = EVTStatus::EVTStatus_Fail;
        return false;
    }

    void* LogConfiguration::GetPointerProperty(char const* key, EVTStatus& error) const
    {
        UNREFERENCED_PARAMETER(key);
        error = EVTStatus::EVTStatus_Fail;
        return nullptr;
    }

    ///<summary>LogConfiguration constructor</summary>
    LogConfiguration::LogConfiguration()      
    {
        ARIASDK_LOG_DETAIL("new instance: this=%p", this);

        SetMinimumTraceLevel(ACTTraceLevel::ACTTraceLevel_Error);
        SetSdkModeType(SdkModeTypes::SdkModeTypes_Aria);
        SetBoolProperty(CFG_BOOL_ENABLE_ANALYTICS, false);
        SetIntProperty(CFG_INT_CACHE_FILE_SIZE, 3145728);        // 3 MB
        SetIntProperty(CFG_INT_RAM_QUEUE_SIZE, 524288);      // 512 K
        SetPointerProperty("httpClient", nullptr);
        SetPointerProperty("runtimeConfig", nullptr);
        SetBoolProperty(CFG_BOOL_ENABLE_MULTITENANT, true);
        SetIntProperty(CFG_INT_MAX_TEARDOWN_TIME, 0);
        SetIntProperty(CFG_INT_MAX_PENDING_REQ, 4);
        SetIntProperty(CFG_INT_RAM_QUEUE_BUFFERS, 3);
        SetIntProperty(CFG_INT_TRACE_LEVEL_MASK, 0);
        SetProperty(CFG_STR_COLLECTOR_URL,TC_DEFAULT_EVENT_COLLECTOR_URL_PROD);
        SetIntProperty(CFG_INT_CACHE_FILE_FULL_NOTIFICATION_PERCENTAGE,75);
        SetIntProperty(CFG_INT_CACHE_MEMORY_FULL_NOTIFICATION_PERCENTAGE, 75);
    }

    LogConfiguration::LogConfiguration(const LogConfiguration &src)        
    {
        ARIASDK_LOG_DETAIL("copy constructor: this=%p", this);
        m_minimumTraceLevel = src.m_minimumTraceLevel;
        m_sdkmode = src.m_sdkmode;

        strProps.insert(src.strProps.begin(), src.strProps.end());
        intProps.insert(src.intProps.begin(), src.intProps.end());
        boolProps.insert(src.boolProps.begin(), src.boolProps.end());
        EVTStatus error;
        std::string url = src.GetProperty(CFG_STR_COLLECTOR_URL, error);
        if (url.empty() || error == EVTStatus::EVTStatus_Fail)
        {
            SetProperty(CFG_STR_COLLECTOR_URL, TC_DEFAULT_EVENT_COLLECTOR_URL_PROD);
        }
    }

    LogConfiguration::LogConfiguration(LogConfiguration&& src) noexcept
        : LogConfiguration((const LogConfiguration &)src)
    {
        ARIASDK_LOG_DETAIL("move constructor: this=%p", this);
        std::move(src);
    }

    LogConfiguration::~LogConfiguration()
    {
    }

    LogConfiguration& LogConfiguration::operator=(const LogConfiguration &src)
    {
        strProps.clear();
        intProps.clear();
        boolProps.clear();
        m_minimumTraceLevel = src.m_minimumTraceLevel;
        m_sdkmode = src.m_sdkmode;

        strProps.insert(src.strProps.begin(), src.strProps.end());
        intProps.insert(src.intProps.begin(), src.intProps.end());
        boolProps.insert(src.boolProps.begin(), src.boolProps.end());
        EVTStatus error;
        std::string url = src.GetProperty(CFG_STR_COLLECTOR_URL, error);
        if (url.empty() || error == EVTStatus::EVTStatus_Fail)
        {
            SetProperty(CFG_STR_COLLECTOR_URL, TC_DEFAULT_EVENT_COLLECTOR_URL_PROD);
        }
        return (*this);
    }

    ConfigKey ILogConfiguration::operator[](const char *key)
    {
        return ConfigKey(this, key);
    }

} ARIASDK_NS_END