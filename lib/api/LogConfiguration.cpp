#include "LogConfiguration.hpp"
#include "IHttpClient.hpp"
#include "IRuntimeConfig.hpp"
#include "IBandwidthController.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "pal/PAL.hpp"


namespace ARIASDK_NS_BEGIN {

    struct StrCompare : public std::binary_function<const char*, const char*, bool> {
    public:
        bool operator() (const char* str1, const char* str2) const
        {
            return std::strcmp(str1, str2) < 0;
        }
    };

    typedef std::map<const char*, const char*, StrCompare> propeties_map;

    /// <summary>
    /// LogConfiguration implementation that allows to convert string values to various
    /// destination types (integer, unsigned and boolean) most commonly used by Aria SDK.
    /// This class is internal and not exposed for customer use.
    /// </summary>
    class LogConfigurationImpl : public propeties_map
    {

    public:
        std::mutex rw_mutex;

        template<typename T>
        bool get(const char* key, T& output)
        {
            const char * value = (*this)[key];
            return parse(value, output);
        }

    protected:

        /// <summary>
        /// Convert string to signed integer.
        /// </summary>
        /// <param name="str"></param>
        /// <param name="output"></param>
        /// <returns></returns>
        bool parse(const char *str, int &output)
        {
            output = std::stoi(str);
            return true;
        }

        /// <summary>
        /// Convert string to unsigned integer.
        /// </summary>
        /// <param name="str"></param>
        /// <param name="output"></param>
        /// <returns></returns>
        bool parse(const char *str, unsigned &output)
        {
            output = (unsigned)std::stoul(str);
            return true;
        }

        /// <summary>
        /// Convert string to boolean:
        /// 
        /// "true"  | "TRUE"        -> true
        /// "1"                     -> true
        /// NULL                    -> false
        /// Any other string value  -> false
        /// 
        /// </summary>
        /// <param name="str">String representation of a bool</param>
        /// <param name="output">bool</param>
        /// <returns></returns>
        bool parse(const char *str, bool &output)
        {
            output = false;
            if (str != NULL)
            {
                output = (_stricmp(str, "true") == 0) || (_stricmp(str, "1") == 0);
            }
            return output;
        }

    };

    /// <summary>Internal ARIA SDK use only</summary>
    void LogConfiguration::SetProperty(char const* key, char const* value)
    {
        LogConfigurationImpl *props = (LogConfigurationImpl *)m_impl;
        std::lock_guard<std::mutex> lock(props->rw_mutex);
        (*props)[key] = value;
    }

    /// <summary>Internal ARIA SDK use only</summary>
    char const* LogConfiguration::GetProperty(char const* key) const
    {
        LogConfigurationImpl *props = (LogConfigurationImpl *)m_impl;
        std::lock_guard<std::mutex> lock(props->rw_mutex);
        if (props->end() != props->find(key))
        {
            return (*props)[key];
        }
        else
        {
            return "";
        }
    }

    /// <summary>Internal ARIA SDK use only</summary>
    void LogConfiguration::GetProperties(std::map<char const*, char const*>& outProperties) const
    {
        LogConfigurationImpl *props = (LogConfigurationImpl *)m_impl;
        std::lock_guard<std::mutex> lock(props->rw_mutex);
        outProperties.clear();
        outProperties.insert(props->begin(), props->end());
    }

    ///<summary>LogConfiguration constructor</summary>
    LogConfiguration::LogConfiguration()
        : enableLifecycleSession(false)
        , cacheFileSizeLimitInBytes(3145728)        // 3 MB
        , cacheMemorySizeLimitInBytes(524288)       // 512 K
        , httpClient(nullptr)
        , runtimeConfig(nullptr)
        , sdkmode(SdkModeTypes::SdkModeTypes_Aria)
        , multiTenantEnabled(true)
        , maxTeardownUploadTimeInSec(0)
        , maxPendingHTTPRequests(4)                 // 4
        , maxDBFlushQueues(3)
        , m_impl(NULL)
        , traceLevelMask(0)
        , minimumTraceLevel(ACTTraceLevel_Error)
    {
        ARIASDK_LOG_DETAIL("new instance: this=%p", this);
        m_impl = new LogConfigurationImpl();
        if (m_impl)
        {
            LogConfigurationImpl *props = (LogConfigurationImpl *)m_impl;
            (*props)[CFG_STR_COLLECTOR_URL] = TC_DEFAULT_EVENT_COLLECTOR_URL_PROD;
            (*props)[CFG_INT_CACHE_FILE_FULL_NOTIFICATION_PERCENTAGE] = "75";
            (*props)[CFG_INT_CACHE_MEMORY_FULL_NOTIFICATION_PERCENTAGE] = "75";
            
            
            ARIASDK_LOG_DETAIL("propeties_map=%p", m_impl);
        }
    }

    LogConfiguration::LogConfiguration(const LogConfiguration &src)
        : LogConfiguration()
    {
        ARIASDK_LOG_DETAIL("copy constructor: this=%p", this);
        (*this) = src;
        ARIASDK_LOG_DETAIL("propeties_map=%p", m_impl);
    }

    LogConfiguration::LogConfiguration(LogConfiguration&& src) noexcept
        : LogConfiguration((const LogConfiguration &)src)
    {
        ARIASDK_LOG_DETAIL("move constructor: this=%p", this);
        m_impl = new LogConfigurationImpl();
        ARIASDK_LOG_DETAIL("propeties_map=%p", m_impl);
        std::move(src);
    }

    LogConfiguration::~LogConfiguration()
    {
        if (m_impl != NULL)
        {
            LogConfigurationImpl *m = reinterpret_cast<LogConfigurationImpl*>(m_impl);
            ARIASDK_LOG_DETAIL("destroying propeties_map=%p", m);
            delete m;
            m_impl = NULL;
        }
    }

    LogConfiguration& LogConfiguration::operator=(const LogConfiguration &src)
    {
        this->enableLifecycleSession = src.enableLifecycleSession;
        this->cacheFileSizeLimitInBytes = src.cacheFileSizeLimitInBytes;
        this->cacheMemorySizeLimitInBytes = src.cacheMemorySizeLimitInBytes;
        this->httpClient = src.httpClient;
        this->runtimeConfig = src.runtimeConfig;
        this->sdkmode = src.sdkmode;
        this->multiTenantEnabled = src.multiTenantEnabled;
        this->maxTeardownUploadTimeInSec = src.maxTeardownUploadTimeInSec;
        this->maxDBFlushQueues = src.maxDBFlushQueues;

        std::string url = src.GetProperty(CFG_STR_COLLECTOR_URL);
        if (!url.empty())
        {
            SetProperty(CFG_STR_COLLECTOR_URL, url.c_str());
        }
        else
        {
            SetProperty(CFG_STR_COLLECTOR_URL, TC_DEFAULT_EVENT_COLLECTOR_URL_PROD);
        }

        // Copy the map contents
        LogConfigurationImpl *props = (LogConfigurationImpl *)m_impl;
        props->clear();

        LogConfigurationImpl *src_props = (LogConfigurationImpl *)(src.m_impl);
        props->insert((*src_props).begin(), (*src_props).end());

        return (*this);
    }



} ARIASDK_NS_END