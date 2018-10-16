#pragma once

#include "Version.hpp"

namespace ARIASDK_NS_BEGIN
{
    //TODO: Per application config
    const char* const TC_DEFAULT_UI_VERSION = "1000/1.8.0.0";

    /// Default collector url to send events to
    const char* const TC_DEFAULT_EVENT_COLLECTOR_URL_PROD = "https://self.events.data.microsoft.com/OneCollector/1.0";

    // Default size limit for storing events in offline storage file
    // It should be enough to save all messages in cache queue.
    const unsigned int TC_DEFAULT_OFFLINE_STORAGE_SIZE_IN_BYTES = 3 * 1024 * 1024;

    // If the size of events cache queues exceeds this limit, the additional events will be saved to storage file.
    const unsigned int TIC_DEFAULT_CACHE_QUEUE_SIZE_IN_BYTES = 512 * 1024;

    // Internal configurations value/limitation

    // Default upper bound limit in KB of data collector request size. 
    // By default each collector request should not exceed 3MB.
    const unsigned int TIC_DEFAULT_MAX_PACKAGE_SIZE_IN_KBytes = 3000;

    // Default buffer time in millisec to send events of batching.
    const unsigned int TIC_DEFAULT_BUFFER_TIME_IN_MILLISECS = 2000;

    // Min size of offline storage file in byts.
    const unsigned int TIC_MIN_OFFLINE_STORAGE_SIZE_IN_BYTES = 512 * 1024;

    // The factor used to calculate how long the record will be stored in offline storage
    // [Deprecated]
    const unsigned int TIC_OFFLINE_STORAGE_RETENTION_FACTOR = 24 * 60 * 60;

    // Interval for sending feedback to resource manager.
    const unsigned int TIC_RM_FEEDBACK_INTERVAL_IN_MILLISEC = 300;

    // Hard limit for sending events. 
    // If the speed of sending events is too fast such that the total messages size exceeds 2 MB, 
    // the newly coming events will be dropped.
    // [Deprecated]
    const unsigned int TIC_TOTAL_MESSAGES_SIZE_LIMIT_IN_BYTES = 2 * 1024 * 1024;

    // Max number of events of a particular priority to send continously
    // [Deprecated]
    const unsigned int TIC_MAX_CONTINUOUS_SENT_COUNT_PER_PRIORITY = 50;

    // Max number of events to upgrade thier priority to higher priority
    // [Deprecated]
    const unsigned int TIC_MAX_UPGRADE_COUNT_PER_PRIORITY = 10;

    // The threshold to give up putting events back to queue. 
    // By setting this, the maximized retry count is HttpClientManager::MAX_RETRY_TIMES * TIC_RETRY_TIMES_STOP_THRESHOD.
    const unsigned int TIC_RETRY_TIMES_STOP_THRESHOD = 3;

    // Timeout in seconds to invoke m_sendingEventsTimer to retry sending previously failed events
    // after they were put back into cache queue.
    // [Deprecated]
    const unsigned int TIC_RETRY_TIMEOUT_IN_SEC = 60 * 60;

    // Reserved size in bytes for httpstack header.
    const unsigned int TIC_RESERVED_HTTPSTACK_HEADER_SIZE_IN_BYTES = 500;

    // Package type
    // Each package has the same type field in its header.
    const char* const PKG_PROPERTY_TYPE = "Client";

    // Package version
    // Each package has the same version field in its header.
    //const char* const PKG_PROPERTY_VERSION = PAL::getSdkVersion();

    // Package server_id in ids
    // Each package has the same ids field with key service_id in its header.
    const char* const PKG_PROPERTY_SERVICE_ID_STR = "1";

    // Package schema
    // Each package has the same schema field in its header.
    const unsigned int PKG_PROPERTY_SCHEMA = 2;

    /** \brief Package header size. The header is used to store some information in front of
               the serialized package. There are 4 bytes of the header. The first byte is the
               record version number, the other 3 bytes are reserved.
     */
    const unsigned int PKG_HEADER_SIZE = 4; // unit: byte

    // Use lower case for following three constant strings
    const char* const ACT_DEFAULT_LOGGER_NAME_HASH_PREFIX = "act_default_tenanttoken" "@@@";

    // ACT_DEFAULT_TENANTTOKEN is for internal use ONLY to represent the case where client doesn't specify a tenant, for eg. old skype client
    // it shouldn't to be sent to the Collector.
    const char* const ACT_DEFAULT_TENANTTOKEN = "act_default_tenanttoken";

    const char* const ACT_DEFAULT_SOURCE = "act_default_source";

#define STATS_EVENT_TYPE "act_stats"

    // ECS client name for ACT configuration, for now use "Skype" but consider to use a separate name like 'Aria'
    const char* const ACT_ECS_CLIENT = "Skype";

    // ECS client version for ACT configuration
    const char* const ACT_ECS_CLIENT_VERSION = "10001/1.0.0.0";

    // ECS agent name for ACT configuration
    const char* const ACT_ECS_AGENT = "ACT";

    // Key of ECS configuration for event send frequency
    const char* const ACT_ECS_KEY_FREQUENCY = "Frequency";

    // Key of ECS configuration for send frequency of stats event
    const char* const ACT_ECS_KEY_FREQUENCY_STATS = STATS_EVENT_TYPE;

    // Key of ECS configuration for kill-switch
    const char* const ACT_ECS_KEY_PRIORITY = "Priority";

    // Key of ECS configuration for kill-switch of source priority
    const char* const ACT_ECS_KEY_SOURCE = "Source";

    // Key of ECS configuration for kill-switch of events priority
    const char* const ACT_ECS_KEY_EVENTS = "Events";

    // Key of ECS configuration for client retention time (in days)
    const char* const ACT_ECS_KEY_RETENTION = "Retention";

    const char* const ACT_ECS_KEY_RETENTION_EXPIRY_TIME = "Retention_expiry_time";

    const int ACT_RETENTION_DEFAULT = -1;

    const int ACT_RETENTION_MIN = -1;

    const int ACT_RETENTION_MAX = 7;

    const int SCT_V3_RECORD_VERSION = 4;

    // Default frequency to send stats record. 
    // By default, every 1 minutes or more, a stats record is generated.
    const unsigned int STATS_SENT_FREQUENCY_THRESHOLD_IN_SECS = 60;

    /** \brief The first positive spot for the frequency distribution of
               package consecutive failure duration.

        <20s, 20s~40s, 40s~80s, 80s~160s, 160s~320s, 320s~640s, >640s
     */
    const unsigned int STATS_PACKAGE_CONSECUTIVE_FAILURE_FIRST_DURATION_IN_SECS = 20;

    /** \brief The factor used to calculate next spot of frequency distribution of
               package consecutive failure duration.

        <20s, 20s~40s, 40s~80s, 80s~160s, 160s~320s, 320s~640s, >640s
     */
    const unsigned int STATS_PACKAGE_CONSECUTIVE_FAILURE_NEXT_FACTOR = 2;

    /** \brief The total spots of frequency distribution of package consecutive
               failure duration.

        <20s, 20s~40s, 40s~80s, 80s~160s, 160s~320s, 320s~640s, >640s
     */
    const unsigned int STATS_PACKAGE_CONSECUTIVE_FAILURE_TOTAL_SPOTS = 7;

    /** \brief The first positive spot for the frequency distribution of rtt.

        <100ms, 100ms~200ms, 200ms~400ms, 400ms~800ms, 800ms~1600ms, 1600s~3200ms, >3200ms
     */
    const unsigned int STATS_RTT_FIRST_DURATION_IN_MILLISECS = 100;

    /** \brief The factor used to calculate next spot of frequency distribution of rtt.
     */
    const unsigned int STATS_RTT_NEXT_FACTOR = 2;

    /** \brief The total spots of frequency distribution of rtt.
     */
    const unsigned int STATS_RTT_TOTAL_SPOTS = 7;

    /** \brief The first positive spot for the frequency distribution of latency.

    <1000ms, 1000ms~2000ms, 2000ms~4000ms, 4000ms~8000ms, 8000ms~16000ms, 16000ms~32000ms, >32000ms
    */
    const unsigned int STATS_LATENCY_FIRST_DURATION_IN_MILLISECS = 1000;

    /** \brief The factor used to calculate next spot of the latency distribution.
    */
    const unsigned int STATS_LATENCY_NEXT_FACTOR = 2;

    /** \brief The total spots of the latency distribution.
    */
    const unsigned int STATS_LATENCY_TOTAL_SPOTS = 7;

    /** \brief The first positive spot for the frequency distribution of record size.

        <1KB, 1KB~2KB, 2KB~4KB, 4KB~8KB, 8KB~16KB, 16KB~32KB, 32KB~64KB, > 64KB
     */
    const unsigned int STATS_RECORD_SIZE_FIRST_IN_KB = 1; // unit: kB

    /** \brief The factor used to calculate next spot of frequency distribution of record size.

        <1KB, 1KB~2KB, 2KB~4KB, 4KB~8KB, 8KB~16KB, 16KB~32KB, 32KB~64KB, > 64KB
     */
    const unsigned int STATS_RECORD_SIZE_NEXT_FACTOR = 2;

    /** \brief The total spots of frequency distribution of record size.

        <1KB, 1KB~2KB, 2KB~4KB, 4KB~8KB, 8KB~16KB, 16KB~32KB, 32KB~64KB, > 64KB
     */
    const unsigned int STATS_RECORD_SIZE_TOTAL_SPOTS = 8;

    /** \brief The first positive spot for the frequency distribution for saved size
               , overwritten size.

        <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB
     */
    const unsigned int STATS_STORAGE_SIZE_FIRST_IN_KB = 8;

    /** \brief The factor used to calculate next spot of frequency distribution
               for saved size, overwritten size.

        <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB
     */
    const unsigned int STATS_STORAGE_SIZE_NEXT_FACTOR = 2;

    /** \brief The total spots of frequency distribution for saved size, overwritten size.

        <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB
     */
    const unsigned int STATS_STORAGE_SIZE_TOTAL_SPOTS = 8;

    /*
        We need this to make sure that when we read from offline storage we have the correct
        lenght of the GUID. This will only prevent certain corruption
    */
    const unsigned int SESSION_GUID_SIZE = 36;

    /*
       This specific size comes from the fact that we plan to use 2 blocks of memory
       and each block of memory has 32 * 1024 so we limit the file size to twice that
    */
    const unsigned int SESSION_OFFLINE_SIZE = 1024 * 1024;

    /*
       Start triggering notifications on this mark if storage is near full
     */
    const unsigned int STORAGE_NEAR_FULL_PCT = 75;

    /*
       Limiter for the min duration of time between UploadNow API calls
     */
    const unsigned int UPLOAD_NOW_LIMITER = 30; // 30 seconds

    const char* const EVENTRECORD_TYPE_CUSTOM_EVENT = "custom";
    const char* const EVENTRECORD_TYPE_LOG_TRACE = "LOG_TRACE";

    const char* const COMMONFIELDS_APP_ID = "AppInfo.Id";
    const char* const COMMONFIELDS_APP_VERSION = "AppInfo.Version";
    const char* const COMMONFIELDS_APP_LANGUAGE = "AppInfo.Language";
    const char* const COMMONFIELDS_APP_EXPERIMENTIDS = "AppInfo.ExperimentIds";
    const char* const COMMONFIELDS_APP_EXPERIMENTETAG = "AppInfo.ETag";
    const char* const COMMONFIELDS_APP_EXPERIMENT_IMPRESSION_ID = "AppInfo.ImpressionId";
    const char* const COMMONFIELDS_DEVICE_ID = "DeviceInfo.Id";
    const char* const COMMONFIELDS_DEVICE_MAKE = "DeviceInfo.Make";
    const char* const COMMONFIELDS_DEVICE_MODEL = "DeviceInfo.Model";
    const char* const COMMONFIELDS_NETWORK_PROVIDER = "DeviceInfo.NetworkProvider";
    const char* const COMMONFIELDS_NETWORK_TYPE = "DeviceInfo.NetworkType";
    const char* const COMMONFIELDS_NETWORK_COST = "DeviceInfo.NetworkCost";
    const char* const COMMONFIELDS_OS_NAME = "DeviceInfo.OsName";
    const char* const COMMONFIELDS_OS_VERSION = "DeviceInfo.OsVersion";
    const char* const COMMONFIELDS_OS_BUILD = "DeviceInfo.OsBuild";
    const char* const COMMONFIELDS_DEVICE_CLASS = "DeviceInfo.Class";


    const char* const COMMONFIELDS_EVENT_SDK_TYPE = "S_t";
    const char* const COMMONFIELDS_EVENT_SDK_OSNAME = "S_p";
    const char* const COMMONFIELDS_EVENT_SDK_SKU = "S_u";
    const char* const COMMONFIELDS_EVENT_SDK_PROJECTION = "S_j";
    const char* const COMMONFIELDS_EVENT_SDK_VER_NUM = "S_v";
    const char* const COMMONFIELDS_EVENT_SDK_ECS = "S_e";

    const char* const COMMONFIELDS_USER_ID = "UserInfo.Id";
    const char* const COMMONFIELDS_USER_MSAID = "UserInfo.MsaId";
    const char* const COMMONFIELDS_USER_ANID = "UserInfo.ANID";
    const char* const COMMONFIELDS_USER_ADVERTISINGID = "UserInfo.AdvertisingId";
    const char* const COMMONFIELDS_USER_LANGUAGE = "UserInfo.Language";
    const char* const COMMONFIELDS_USER_TIMEZONE = "UserInfo.TimeZone";

    const char* const COMMONFIELDS_PIPELINEINFO_ACCOUNT = "PipelineInfo.AccountId";

    const char* const  CUSTOMFIELD_EVENT_PRIORITY = "eventpriority";

    const char* const LOG_SESSION = "LogSession";
    const char* const SESSION_SDKU_ID = "SessionSDKUid";
    const char* const SESSION_FIRST_TIME = "Session.FirstLaunchTime";

    const char* const EventInfo_Source = "EventInfo.Source";
    const char* const EventInfo_Name = "EventInfo.Name";
    const char* const EventInfo_InitId = "EventInfo.InitId";
    const char* const EventInfo_Sequence = "EventInfo.Sequence";
    const char* const COMMONFIELDS_EVENT_TIME = "EventInfo.Time";
    const char* const COMMONFIELDS_EVENT_SDKVERSION = "EventInfo.SdkVersion";
    const char* const COMMONFIELDS_EVENT_CRC32 = "EventInfo.CRC32";

    const char* const SESSION_STATE = "Session.State";
    const char* const SESSION_ID = "Session.Id";
    const char* const SESSION_DURATION = "Session.Duration";
    const char* const SESSION_DURATION_BUCKET = "Session.DurationBucket";
    const char* const SESSION_SDKUID = "DeviceInfo.SDKUid";

    bool  const ENABLE_HMAC_SHA256 = true;
    const char* const HMAC_ENCRYPTION_KEY = "bdyt7XVQVWYzXCviDbKMDktZZBIbjA7g9pdjXEYtsSUNPQHpBPofMgx9SrFFNZI9";
    const char* const HMAC_CLIENT_ID = "CppNativeLibrary";

    const char* const TICKETS_PREPEND_STRING = "1000";

    //---------------------------------------------------------------------------
    // Metastats field name (FN) aliases
    //---------------------------------------------------------------------------

    // main counts
    const char* const FN_RECEIVED_COUNT = "records_received_count";
    //const char* const FN_TRIED_SEND_COUNT             = "records_tried_to_send_count";
    const char* const FN_SENT_COUNT = "records_sent_count";
    const char* const FN_DROPPED_COUNT = "records_dropped_count";			    //no longer includes overflow
    const char* const FN_REJECTED_COUNT = "r_count";						    //was records_rejected_count
    const char* const FN_SENT_CURR_SESSION = "records_sent_curr_session";          //should this be shorter?
    const char* const FN_SENT_PREV_SESSION = "records_sent_prev_session";          //should this be shorter?

    // intermediate state
    const char* const FN_IN_FLIGHT = "infl";
    const char* const FN_IN_STORAGE = "inol";

    // offline storage counts
    const char* const FN_OFFLINE_READ = "ol_r";
    const char* const FN_OFFLINE_WRITE = "ol_w";
    const char* const FN_OFFLINE_FORMAT = "ol_f";                               //offline_storage_format_type
    const char* const FN_OFFLINE_LAST_FAIL = "ol_last_fail";                       //offline_storage_last_failure
    const char* const FN_OFFLINE_CONFIG_BYTES = "ol_bytes";                           //config_offline_storage_size_bytes


    // reject reasons
    const char* const FN_REJECT_SIZE_LIMIT = "r_size";
    const char* const FN_REJECT_INVALID = "r_inv"; 								// invalid event or property name; invalid event tenant??
    //const char* const FN_REJECT_INVALID_PII           = "r_invpii";
    //const char* const FN_REJECT_UNKNOWN               = "r_unk";
    const char* const FN_REJECT_BANNED = "r_ban";							    //was records_banned_count
    const char* const FN_REJECT_SERVER_REJECTED = "r_403";
    //const char* const FN_REJECT_KILLED                = "r_kl";
    //const char* const FN_REJECT_PAUSED                = "r_ps";
    //invalid_message_type??
    //required_argument_missing??
    //old_record_version??
    //event_expired??


    // drop reasons
    //const char* const FN_DROP_OFFLINE_DISABLED        = "d_disk_off";
    const char* const FN_DROP_SERVER_DECLINED = "h";
    //const char* const FN_DROP_CODE_ERROR              = "d_assert";
    //const char* const FN_DROP_IMMEDIATE_CODE          = "d_im_h_%d";
    //const char* const FN_DROP_IMMEDIATE_NOT_200       = "d_im_h";
    //const char* const FN_DROP_BAD_TENANT              = "d_bad_tenant";
    const char* const FN_DROP_DISK_FULL = "d_disk_full"; 						//was records_dropped_offline_storage_overflow
    const char* const FN_DROP_IO_FAIL = "d_io_fail";							//was records_dropped_offline_storage_save_failed
    //const char* const FN_DROP_BOND_FAIL               = "d_bond_fail";                        //records_dropped_serialization_failed AND records_dropped_categorization_failed from V1?
    //const char* const FN_DROP_UNKNOWN                 = "d_unk";
    const char* const FN_DROP_RETRY_LIMIT = "d_retry_lmt";						//was records_dropped_retry_exceeded

    // retry
    const char* const FN_RETRY = "retry";

    // retry reasons
    //const char* const FN_RETRY_HTTP                   = "rt_h_%d";
    //const char* const FN_RETRY_UNKNOWN                = "rt_unk";
    //const char* const FN_RETRY_CANCELLED              = "rt_cancel";
    //const char* const FN_RETRY_BAD_URL                = "rt_badurl";
    //const char* const FN_RETRY_TIMED_OUT              = "rt_timeout";
    //const char* const FN_RETRY_BAD_HOST               = "rt_badhost";
    //const char* const FN_RETRY_BAD_CONNECTION         = "rt_badconn";
    //const char* const FN_RETRY_LOST_CONNECTION        = "rt_lostconn";
    //const char* const FN_RETRY_DNS                    = "rt_dns";
    //const char* const FN_RETRY_TOO_MANY_REDIRECTS     = "rt_maxred";
    //const char* const FN_RETRY_RESOURCE_UNAVAILABLE   = "rt_nores";
    //const char* const FN_RETRY_NO_INTERNET            = "rt_nonet";

    // wake up tpm notification
    //const char* const FN_WAKE_UP_TPM                  = "tpm_wakeup";

    // Other
    //record_size_bytes_max
    //record_size_bytes_min
    //records_received_size_bytes
    //record_size_kb_distribution

    //requests_acked_succeeded
    //requests_acked_retried
    //requests_acked_dropped

    //exceptions_per_eventtype_count
    //records_per_eventtype_count

    // Package stats
    //requests_acked_dropped_on_HTTP
    //requests_acked_retried_on_HTTP


    // Priority-based reason codes
    // Low Priority
    //const char* const FN_LP_REJECT_BANNED             = "lp_r_ban";	                                //*_records_banned_count
    //const char* const FN_LP_RECEIVED_COUNT            = "lp_records_received_count";                 //*_records_received_count
    //const char* const FN_LP_SENT_COUNT                = "lp_records_sent_count";	                        //*_records_sent_count
    //const char* const FN_LP_SENT_CURR_SESSION         = "lp_records_sent_curr_session";           //*_records_sent_count_current_session
    //const char* const FN_LP_SENT_PREV_SESSION         = "lp_records_sent_prev_session";           //*_records_sent_count_previous_sessions
    //const char* const FN_LP_DROPPED_COUNT             = "lp_records_dropped_count";                   //*_records_dropped_count
    //const char* const FN_LP_REJECTED_COUNT            = "lp_r_count";                                //*_records_dropped_count                                                                                                
    //const char* const FN_LP_RECEIVED_BYTES            = "lp_records_received_size_bytes";            //*_records_received_size_bytes
    //const char* const FN_LP_LOG_SUCCESS_SEND_LATENCY_MILLISEC_MAX = "lp_log_to_successful_send_latency_millisec_max"; //*_log_to_successful_send_latency_millisec_max
    //const char* const FN_LP_LOG_SUCCESS_SEND_LATENCY_MILLISEC_MIN = "lp_log_to_successful_send_latency_millisec_min";//*_log_to_successful_send_latency_millisec_min


    // Normal Priority


    // High Priority


    // Immediate Priority


    // Background Priority



} ARIASDK_NS_END
