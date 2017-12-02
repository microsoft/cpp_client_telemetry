#ifndef ARIA_ILOGCONFIGURATION_HPP
#define ARIA_ILOGCONFIGURATION_HPP

#include "Version.hpp"
#include "Enums.hpp"
#include "ctmacros.hpp"
#include "stdint.h"

// *INDENT-OFF*
namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
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

            static constexpr const char* const CFG_STR_PRAGMA_JOURNAL_MODE = "PRAGMA_journal_mode";
            static constexpr const char* const CFG_STR_PRAGMA_SYNCHRONOUS = "PRAGMA_synchronous";


            class ARIASDK_LIBABI ILogConfiguration
            {
            public:
                //virtual ~ILogConfguration() {}

                /// <summary>[optional] Debug trace level mask controls global verbosity level.<br>
                /// default is ACTTraceLevel_Error</summary>
                virtual ACTStatus SetMinimumTraceLevel(ACTTraceLevel minimumTraceLevel) = 0;

                /// <summary>[optional] Debug trace level mask controls global verbosity level.<br>
                /// default is ACTTraceLevel_Error</summary>
                virtual ACTTraceLevel GetMinimumTraceLevel() const = 0;

                /// <summary>Api to set Aria SDK mode with Non UTC, UTC with common Schema or UTC with Aria Schema.<br>
                /// default is Non UTC</summary>
                virtual ACTStatus SetSdkModeType(SdkModeTypes sdkmode) = 0;

                /// <summary>Api to get Aria SDK mode with Non UTC, UTC with common Schema or UTC with Aria Schema.<br>
                /// default is Non UTC</summary>
                virtual SdkModeTypes GetSdkModeType() const = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to configure Aria string setting
                /// </summary>
                /// 
                virtual ACTStatus SetProperty(char const* key, char const* value) = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to configure Aria int setting
                /// </summary>
                /// 
                virtual ACTStatus SetIntProperty(char const* key, uint32_t value) = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to configure Aria bool setting
                /// </summary>
                /// 
                virtual ACTStatus SetBoolProperty(char const* key, bool value) = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to configure Aria pointer setting
                /// </summary>
                /// 
                virtual ACTStatus SetPointerProperty(char const* key, void* value) = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to get Aria string setting
                /// </summary>
                /// 
                virtual char const* GetProperty(char const* key, ACTStatus& error) const = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to get Aria int setting
                /// </summary>
                /// 
                virtual uint32_t GetIntProperty(char const* key, ACTStatus& error) const = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to get Aria bool setting
                /// </summary>
                /// 
                virtual bool GetBoolProperty(char const* key, ACTStatus& error) const = 0;

                /// <summary>
                /// ILogConfiguration properties API allows to get Aria pointer setting
                /// </summary>
                /// 
                virtual void* GetPointerProperty(char const* key, ACTStatus& error) const = 0;
            };
        }
    }
} // namespace Microsoft::Applications::Telemetry
#endif 