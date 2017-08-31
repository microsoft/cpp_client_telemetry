#ifndef ARIA_LOGCONFIGURATION_HPP
#define ARIA_LOGCONFIGURATION_HPP

#include "Version.hpp"
#include "Enums.hpp"
#include "ctmacros.hpp"
#include <map>

#ifdef ARIASDK_PAL_SKYPE
    #include <httpstack/fwd.hpp>
    #include <ecsClientInterface.hpp>
    #include <ResourceManager/ResourceManagerPublic.hpp>
#endif

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*
    class IHttpClient;
    class IRuntimeConfig;
    class IBandwidthController;

	static const char* COLLECTOR_URL_UNITED_STATES = "https://us.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_GERMANY = "https://de.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_AUSTRALIA = "https://au.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_JAPAN = "https://jp.pipe.aria.microsoft.com/Collector/3.0/";
	static const char* COLLECTOR_URL_EUROPE = "https://eu.pipe.aria.microsoft.com/Collector/3.0/";


    static constexpr const char* const TRANSMITPROFILE_REALTIME = "REAL_TIME";
    static constexpr const char* const TRANSMITPROFILE_NEARREALTIME = "NEAR_REAL_TIME";
    static constexpr const char* const TRANSMITPROFILE_BESTEFFORT = "BEST_EFFORT";

    static constexpr const char* const CFG_BOOL_ENABLE_ANALYTICS = "enableLifecycleSession";
    static constexpr const char* const CFG_BOOL_ENABLE_MULTITENANT = "multiTenantEnabled";
    static constexpr const char* const CFG_BOOL_ENABLE_CRC32 = "enableCRC32";
    static constexpr const char* const CFG_BOOL_ENABLE_HMAC = "enableHMAC";
    static constexpr const char* const CFG_BOOL_ENABLE_DB_COMPRESS = "enableDBCompression";
    static constexpr const char* const CFG_BOOL_ENABLE_WAL_JOURNAL = "enableWALJournal";

    static constexpr const char* const CFG_STR_COLLECTOR_URL = "eventCollectorUri";
    static constexpr const char* const CFG_STR_CACHE_FILE_PATH = "cacheFilePath";
    
    static constexpr const char* const CFG_INT_CACHE_FILE_SIZE = "cacheFileSizeLimitInBytes";
    static constexpr const char* const CFG_INT_RAM_QUEUE_SIZE = "cacheMemorySizeLimitInBytes";
    static constexpr const char* const CFG_INT_RAM_QUEUE_BUFFERS = "maxDBFlushQueues";
    static constexpr const char* const CFG_INT_TRACE_LEVEL_MASK = "traceLevelMask";
    static constexpr const char* const CFG_INT_TRACE_LEVEL_MIN = "minimumTraceLevel";
    static constexpr const char* const CFG_INT_SDK_MODE = "sdkmode";
    static constexpr const char* const CFG_INT_MAX_TEARDOWN_TIME = "maxTeardownUploadTimeInSec";
    static constexpr const char* const CFG_INT_MAX_PENDING_REQ = "maxPendingHTTPRequests";
    static constexpr const char* const CFG_INT_MAX_PKG_DROP_ON_FULL = "maxPkgDropOnFull";
    static constexpr const char* const CFG_INT_CACHE_FILE_FULL_NOTIFICATION_PERCENTAGE = "cacheFileFullNotificationPercentage";
    static constexpr const char* const CFG_INT_CACHE_MEMORY_FULL_NOTIFICATION_PERCENTAGE = "cacheMemoryFullNotificationPercentage";
    
    


	enum SdkModeTypes
	{
		SdkModeTypes_Aria = 0, //This is default transmission mode
		SdkModeTypes_UTCAriaBackCompat = 1,
		SdkModeTypes_UTCCommonSchema = 2
	};

    struct ARIASDK_LIBABI LogConfiguration
    {
        /// <summary>[optional] Url of the collector for sending events.
        /// default will be false
        /// </summary>
        bool enableLifecycleSession;

        /// <summary>[optional] Enable multiTenant
        /// default will be true
        /// </summary>
        bool multiTenantEnabled;

        /// <summary>[optional] Size limit of the disk file used to cache events on client side.
        /// Additional events might cause older events in the file cache to be dropped.
        /// This size limit should be larger than the cacheMemorySizeLimitInBytes below
        /// </summary>
        unsigned int cacheFileSizeLimitInBytes;

        /// <summary>[optional] Memory size limit that allows events to be cached in memory.
        /// Additional events will cause older events to be flushed to disk file.
        /// </summary>
        unsigned int cacheMemorySizeLimitInBytes;

        /// [optional] Pointer to HTTP client object
        /// Target object must be kept alive for the lifetime of ILogManager.
        IHttpClient* httpClient = nullptr;

        /// [optional] Pointer to runtime configuration provider object
        /// Target object must be kept alive for the lifetime of ILogManager.
        IRuntimeConfig* runtimeConfig = nullptr;

        /// <summary>[optional] Debug trace module mask controls what modules may emit debug output.<br>
        /// default is 0 - monitor no modules</summary>
        unsigned int  traceLevelMask;

        /// <summary>[optional] Debug trace level mask controls global verbosity level.<br>
        /// default is ACTTraceLevel_Error</summary>
        ACTTraceLevel minimumTraceLevel;

        /// <summary>Api to set Aria SDK mode with Non UTC, UTC with common Schema or UTC with Aria Schema.<br>
        /// default is Non UTC</summary>
        SdkModeTypes sdkmode;

        /// <summary>[optional] Maximum amount of time (in seconds) allotted to upload in-ram and offline records on teardown.<br>
        /// If device is in a state where events are not allowed to be transmitted (offline, roaming, etc.), then the value is ignored.<br>
        /// default duration is 0</summary>
        unsigned int maxTeardownUploadTimeInSec;

        /// <summary>[optional] Maximum number of HTTP requests posted to HTTP stack.<br>
        /// This value controls how much RAM is allocated for pending HTTP requests.<br>
        /// Each request may consume up to ~1MB of RAM. On slow network connections<br>
        /// it may happen that there is a large number of requests pending.<br>
        /// default value is 4</summary>
        unsigned int maxPendingHTTPRequests;

        /// <summary>[optional] Maximum number of DB flush back buffers / queues.<br>
        /// Each Flush() operation or overflow of in-ram to offline storage is handled<br>
        /// asynchronously by swapping in-ram queue to a back buffer and scheduling<br>
        /// asynchronous task that saves the buffer to offline storage.<br>
        /// Each queue consumes up to <i>cacheMemorySizeLimitInBytes</i> bytes.<br>
        /// Default value is 3 queues (back buffers)</summary>
        unsigned int maxDBFlushQueues;

        /// <summary>
        /// LogConfiguration properties API allows to configure internal Aria settings
        /// not typically exposed to majority of customers. These settings may be OS /
        /// platform specific or experimental.
        /// </summary>
        /// 
        /// <summary>Internal ARIA SDK use only</summary>
        void SetProperty(char const* key, char const* value);

        /// <summary>Internal ARIA SDK use only</summary>
        char const*  GetProperty(char const* key) const;

        /// <summary>Internal ARIA SDK use only</summary>
        void  GetProperties(std::map<char const*, char const*>& outProperties) const;

        ///<summary>LogConfiguration constructor</summary>
        LogConfiguration();

        ///<summary>LogConfiguration copy-constructor</summary>
        LogConfiguration(const LogConfiguration &src);

        ///<summary>LogConfiguration move-constructor</summary>
        LogConfiguration(LogConfiguration&& src) noexcept;

        ///<summary>LogConfiguration assignment operator with copy semantics</summary>
        LogConfiguration& ARIASDK_LIBABI_CDECL operator=(const LogConfiguration &src);

        ///<summary>LogConfiguration destructor</summary>
        virtual ~LogConfiguration();

        /// <summary>Internal ARIA SDK use only</summary>
        void *m_impl;

    };


}}} // namespace Microsoft::Applications::Telemetry
#endif //MYAPPLICATION_EVENTPROPERTIES_H