#include "LogConfiguration.hpp"
#include "IHttpClient.hpp"
#include "IRuntimeConfig.hpp"
#include "IBandwidthController.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "pal/PAL.hpp"


namespace ARIASDK_NS_BEGIN {

    void LogConfiguration::SetMinimumTraceLevel(ACTTraceLevel minimumTraceLevel)
    {
        m_minimumTraceLevel = minimumTraceLevel;
    }

    ACTTraceLevel LogConfiguration::GetMinimumTraceLevel() const
    {
        return m_minimumTraceLevel;
    }
    void LogConfiguration::SetSdkModeType(SdkModeTypes sdkmode)
    {
        m_sdkmode = sdkmode;
    }
    SdkModeTypes LogConfiguration::GetSdkModeType() const
    {
        return m_sdkmode;
    }

    void LogConfiguration::SetProperty(char const* key, char const* value)
    {
        if (nullptr != key && nullptr != value)
        {
            std::string keyString(key);
            if (!keyString.empty())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                strProps[keyString] = value;
            }
        }
    }
    void LogConfiguration::SetIntProperty(char const* key, unsigned int value)
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                intProps[keyString] = value;
            }
        }

    }
    void LogConfiguration::SetBoolProperty(char const* key, bool value)
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty())
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                boolProps[keyString] = value;
            }
        }

    }
    void LogConfiguration::SetPointerProperty(char const* key, void* value)
    {
        UNREFERENCED_PARAMETER(key);
        UNREFERENCED_PARAMETER(value);
    }
   
    char const* LogConfiguration::GetProperty(char const* key, bool& error) const
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty() && strProps.find(keyString) != strProps.end())
            {
                error = false;
                return strProps.at(keyString).c_str();
            }
        }
        error = true;
        return "";
    }

    uint32_t LogConfiguration::GetIntProperty(char const* key, bool& error) const
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty() && intProps.find(keyString) != intProps.end())
            {
                error = false;
                return intProps.at(keyString);
            }
        }
        error = true;
        return 0;
    }
    bool LogConfiguration::GetBoolProperty(char const* key, bool& error) const
    {
        if (nullptr != key)
        {
            std::string keyString(key);
            if (!keyString.empty() && boolProps.find(keyString) != boolProps.end())
            {
                error = false;
                return boolProps.at(keyString);
            }
        }
        error = true;
        return false;
    }

    void* LogConfiguration::GetPointerProperty(char const* key, bool& error) const
    {
        UNREFERENCED_PARAMETER(key);
        error = true;
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
        bool error;
        std::string url = src.GetProperty(CFG_STR_COLLECTOR_URL, error);
        if (url.empty() || error == true)
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
        bool error;
        std::string url = src.GetProperty(CFG_STR_COLLECTOR_URL, error);
        if (url.empty() || error == true)
        {
            SetProperty(CFG_STR_COLLECTOR_URL, TC_DEFAULT_EVENT_COLLECTOR_URL_PROD);
        }
        return (*this);
    }

    ConfigKey ILogConfiguration::operator[](const char *key)
    {
        return ConfigKey(this, key);
    }

    bool ILogConfiguration::err = false;

} ARIASDK_NS_END