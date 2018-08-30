#ifndef METASTATS_HPP
#define METASTATS_HPP

#include "pal/PAL.hpp"

#include "api/IRuntimeConfig.hpp"

#include <Enums.hpp>
#include "bond/generated/AriaProtocol_types.hpp"

#include <Config.hpp>

#include <memory>
#include <algorithm>

namespace ARIASDK_NS_BEGIN {

    /// <summary>
    /// Define a new name for a map whose key, value are both unsigned int.
    /// </summary>
    typedef std::map<unsigned int, unsigned int> uint_uint_dict_t;

    /// <summary>
    /// Define a new name for a map whose key is string and value is unsigned int.
    /// </summary>
    typedef std::map<std::string, unsigned int> string_uint_dict_t;

    /// <summary>
    /// Define a new name for a map to store information per source and event type.
    /// </summary>
    typedef std::map<std::string, std::map<std::string, unsigned int>> eventsMap;

    /// <summary>
    /// Class defining some configurations used in collecting stats,
    /// including sending frequency and frequency distributions of multiple data sets.
    /// </summary>
    class StatsConfig
    {
    public:
        /// The first positive spot for the frequency distribution of rtt.
        /// <100ms, 100ms~200ms, 200ms~400ms, 400ms~800ms, 800ms~1600ms, 1600s~3200ms, >3200ms
        unsigned int rtt_first_duration_in_millisecs;

        /// The factor used to calculate next spot of frequency distribution of rtt.
        unsigned int rtt_next_factor;

        /// The total spots of frequency distribution of rtt.
        unsigned int rtt_total_spots;

        /// The first positive spot for the frequency distribution of latency.
        /// <100ms, 100ms~200ms, 200ms~400ms, 400ms~800ms, 800ms~1600ms, 1600s~3200ms, >3200ms
        unsigned int latency_first_duration_in_millisecs;

        /// The factor used to calculate next spot of frequency distribution of latency.
        unsigned int latency_next_factor;

        /// The total spots of frequency distribution of latency.
        unsigned int latency_total_spots;

        /// The first positive spot for the frequency distribution of record size.
        /// <1KB, 1KB~2KB, 2KB~4KB, 4KB~8KB, 8KB~16KB, 16KB~32KB, 32KB~64KB, > 64KB
        unsigned int record_size_first_in_kb; // unit: kb

                                              /// The factor used to calculate next spot of frequency distribution for record size.
        unsigned int record_size_next_factor;

        /// The total spots of frequency distribution of record size.
        unsigned int record_size_total_spots;

        /// The first positive spot for the frequency distribution for saved size,
        /// overwritten size.
        /// <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB
        unsigned int storage_size_first_in_kb;

        /// The factor used to calculate next spot of frequency distribution
        /// for saved size, overwritten size.
        unsigned int storage_size_next_factor;

        /// The total spots of frequency distribution for saved size
        unsigned int storage_size_total_spots;

        /// A Constructor. \n
        /// Set internal configurations of TelemetryStats, including sending frequency of the stats and
        /// default group partitions of each frequency distribution
        StatsConfig()
        {
            //define the group partitions for the frequency distribution of round trip time
            rtt_first_duration_in_millisecs = STATS_RTT_FIRST_DURATION_IN_MILLISECS;  // in milliseconds
            rtt_next_factor = STATS_RTT_NEXT_FACTOR;
            rtt_total_spots = STATS_RTT_TOTAL_SPOTS;

            //define the group partitions for the frequency distribution of latency
            latency_first_duration_in_millisecs = STATS_LATENCY_FIRST_DURATION_IN_MILLISECS;  // in milliseconds
            latency_next_factor = STATS_LATENCY_NEXT_FACTOR;
            latency_total_spots = STATS_LATENCY_TOTAL_SPOTS;

            //define the group partitions for the frequency distribution of single record size
            record_size_first_in_kb = STATS_RECORD_SIZE_FIRST_IN_KB; //in kilobytes
            record_size_next_factor = STATS_RECORD_SIZE_NEXT_FACTOR;
            record_size_total_spots = STATS_RECORD_SIZE_TOTAL_SPOTS;

            //define the group partitions for the frequency distribution of size saved and
            //overwritten each time in the storage
            storage_size_first_in_kb = STATS_STORAGE_SIZE_FIRST_IN_KB; //in kilobytes
            storage_size_next_factor = STATS_STORAGE_SIZE_NEXT_FACTOR;
            storage_size_total_spots = STATS_STORAGE_SIZE_TOTAL_SPOTS;
        }
    };

    /// <summary>
    /// Record statistics
    /// </summary>
    class RecordStats
    {

    public:

        /// total number of records received
        unsigned int bannedCount;

        /// total number of records received
        unsigned int receivedCount;

        /// total number of metastats records received
        unsigned int receivedMetastatsCount;

        /// the number of records rejected by SDK
        unsigned int rejectedCount;

        /// distribution of records count by reason due to which record was rejected
        uint_uint_dict_t rejectedCountReasonDistribution;

        /// the number of dropped records
        unsigned int droppedCount;

        /// the number of overflown records
        unsigned int overflownCount;

        /// distribution of records count by reason due to which record was dropped
        uint_uint_dict_t droppedCountReasonDistribution;

        /// key: http return code
        /// value: the number of records declined by collector (either rejected or dropped due to server error) per each HTTP return code
        uint_uint_dict_t droppedCountPerHttpReturnCode;

        /// the number of sent records
        unsigned int sentCount;

        /// the number of sent records from the current session
        unsigned int sentCountFromCurrentSession;

        /// the number of sent records from previous sessions
        unsigned int sentCountFromPreviousSession;

        /// the delta of events added/removed from in flight
        unsigned int inflightCount;

        /// the total number of records per semantic
        string_uint_dict_t semanticToRecordCountMap;

        /// TODO: do we need this anymore if we are going to count the reasons frop dropping events?
        /// the total number of exceptions per semantic
        string_uint_dict_t semanticToExceptionCountMap;

        /// min record size
        unsigned int minOfRecordSizeInBytes;

        /// max record size
        unsigned int maxOfRecordSizeInBytes;

        /// total records' size in bytes
        unsigned int totalRecordsSizeInBytes;

        ///<1KB, 1KB~2KB, 2KB~4KB, 4KB~8KB, 8KB~16KB, 16KB~32KB, 32KB~64KB, > 64KB \n
        ///key: min value of each size range \n
        ///value: the number of records with size in given range
        uint_uint_dict_t sizeInKBytesDistribution;

        /// min record size which is dropped in SendAsync
        unsigned int minOfDroppedRecordSizeInBytes;

        /// max record size which is dropped in SendAsync
        unsigned int maxOfDroppedRecordSizeInBytes;

        /// reset all members
        void Reset()
        {
            bannedCount = 0;
            receivedCount = 0;
            receivedMetastatsCount = 0;
            droppedCount = 0;
            rejectedCount = 0;
            overflownCount = 0;
            sentCount = 0;
            sentCountFromCurrentSession = 0;
            sentCountFromPreviousSession = 0;
            inflightCount = 0;

            semanticToRecordCountMap.clear();
            semanticToExceptionCountMap.clear();

            minOfRecordSizeInBytes = static_cast<unsigned int>(~0);
            maxOfRecordSizeInBytes = 0;
            totalRecordsSizeInBytes = 0;

            minOfDroppedRecordSizeInBytes = static_cast<unsigned int>(~0);;
            maxOfDroppedRecordSizeInBytes = 0;

            sizeInKBytesDistribution.clear();

            droppedCountReasonDistribution.clear();
            rejectedCountReasonDistribution.clear();
        }

        RecordStats()
        {
            Reset();
        }

        ~RecordStats()
        {
            static size_t count = 0;
            count++;
            LOG_TRACE("RecordStats destroyed: %u", count);
        }

    protected:
        ARIASDK_LOG_DECL_COMPONENT_CLASS();
    };

    /// <summary>
    /// Package statistics
    /// </summary>
    class PackageStats
    {
    public:
        /// total number of package send AND expect to be acknowledged
        unsigned int totalPkgsToBeAcked;

        /// total number of package send AND expect to be acknowledged with metastats events only
        unsigned int totalMetastatsOnlyPkgsToBeAcked;

        /// total number of package send AND don't expect to be acknowledged (fire and forget)
        unsigned int totalPkgsNotToBeAcked;

        /// total number of package sends that are acknowledged
        unsigned int totalPkgsAcked;

        /// total number of package sends that are acknowledged with metastats events only
        unsigned int totalMetastatsOnlyPkgsAcked;

        /// number of successful package sends that are acknowledged
        unsigned int successPkgsAcked;

        /// number of failed package sends that are acknowledged and deemed to be retried
        unsigned int retryPkgsAcked;

        /// number of dropped package sends that are acknowledged and deemed to be dropped
        unsigned int dropPkgsAcked;

        /// key: error code
        /// value: number of dropped packages per each HTTP return code
        uint_uint_dict_t dropPkgsPerHttpReturnCode;

        uint_uint_dict_t retryPkgsPerHttpReturnCode;

        /// the total size of packages
        unsigned int totalBandwidthConsumedInBytes;

        /// reset all members
        void Reset()
        {
            totalPkgsToBeAcked = 0;
            totalMetastatsOnlyPkgsToBeAcked = 0;
            totalPkgsNotToBeAcked = 0;
            totalPkgsAcked = 0;
            totalMetastatsOnlyPkgsAcked = 0;
            successPkgsAcked = 0;
            retryPkgsAcked = 0;
            dropPkgsAcked = 0;
            dropPkgsPerHttpReturnCode.clear();
            retryPkgsPerHttpReturnCode.clear();
            totalBandwidthConsumedInBytes = 0;
        }

        PackageStats()
        {
            Reset();
        }
    };

    /// <summary>
    /// Latency statistics.
    /// </summary>
    class LatencyStats
    {
    public:
        /// max latency
        unsigned int maxOfLatencyInMilliSecs;

        /// min latency
        unsigned int minOfLatencyInMilliSecs;

        /// <100ms, 100ms~200ms, 200ms~400ms, 400ms~800ms, 800ms~1600ms, 1600s~3200ms, >3200ms \n
        /// key: min value of each range \n
        /// value: the number of success packages with latency in given range \n
        uint_uint_dict_t latencyDistribution;

        /// reset all members
        void Reset()
        {
            maxOfLatencyInMilliSecs = static_cast<unsigned int>(0);
            minOfLatencyInMilliSecs = static_cast<unsigned int>(~0);
        }

        LatencyStats()
        {
            Reset();
        }

    };

    /// <summary>
    /// Offline storage statistics.
    /// </summary>
    class OfflineStorageStats
    {
    public:
        /// storage format
        std::string storageFormat;

        std::string lastFailureReason;

        /// storage file size
        size_t fileSizeInBytes;

        /// the total number of load invoked
        unsigned int loadCalled;

        /// the total number of save invoked
        unsigned int saveCalled;

        /// the number of successful save invoked
        unsigned int successSaveCalled;

        /// the number of times save failed
        unsigned int failedSaveCalled;

        /// the total number of overwritten invoked among all save operations
        unsigned int overwrittenInvoked;

        /// The number of records saved
        unsigned int recordSavedCount;

        /// The number of records retrieved
        unsigned int recordRetrievedCount;

        /// Records currently in online storage;
        // ++ with recordSavedCount, -- with recordRetrievedCount; can be negative
        int inOnlineCount;

        /// The number of records dropped while saving
        unsigned int recordDroppedCount;

        /// <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB \n
        /// key: min value of each size range \n
        /// value: the number of save operation for FiFoFile with saved size in given range
        uint_uint_dict_t saveSizeInKBytesDistribution;

        /// <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB \n
        /// key: min value of each size range \n
        /// value: the number of successful save operation for FiFoFile with saved size in given range
        uint_uint_dict_t successSaveSizeInKBytesDistribution;

        /// <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB \n
        /// key: min value of each size range \n
        // value: the number of overwritten operation for FiFoFile with overwritten size in given range
        uint_uint_dict_t overwrittenSizeInKBytesDistribution;

        /// reset all members
        void Reset()
        {
            fileSizeInBytes = 0;
            loadCalled = 0;
            saveCalled = 0;
            successSaveCalled = 0;
            failedSaveCalled = 0;
            overwrittenInvoked = 0;
            recordSavedCount = 0;
            recordDroppedCount = 0;
            recordRetrievedCount = 0;
            inOnlineCount = 0;
        }

        OfflineStorageStats()
        {
            Reset();
        }
    };

    /// <summary>
    /// Class describing telemetry stats of a specific tenant.
    /// </summary>
    class TelemetryStats
    {
    public:
        /// tenant Id
        std::string tenantId;

        /// ui version
        std::string uiVersion;

        /// clienttelemetry::VER
        int configVersion;

        /// ecsclient::VER
        int ecsConfigVersion;

        // The maximum in-memory cache size in bytes
        unsigned int configInmemoryCacheSizeBytes;

        /// true if offline storage is enabled.
        bool offlineStorageEnabled;

        /// true if resource manager is enabled.
        bool resourceManagerEnabled;

        /// true if ecs client is used
        bool ecsClientEnabled;

        /// a guid created when starting the SCT client.
        std::string sessionId;

        /// the utc timestamp of starting the SCT client.
        int64_t sessionStartTimestamp;

        /// the utc timestamp of starting the SCT client.
        int64_t session_startup_time_in_millisec;

        /// the utc timestamp of starting collecting new telemetry stats.
        int64_t statsStartTimestamp;

        /// the sequence number of the new telemetry stats within the session as indicated by the sessionId
        int64_t statsSequenceNum;

        PackageStats packageStats;

        uint_uint_dict_t retriesCountDistribution;
        
        LatencyStats rttStats;
        std::map<EventLatency, LatencyStats> logToSuccessfulSendLatencyPerLatency;

        RecordStats recordStats;
        std::map<EventLatency, RecordStats> recordStatsPerLatency;

        OfflineStorageStats offlineStorageStats;
    };

    
    /// <summary>
    /// MetaStats class:
    /// * aggregats all per-tenant and overall stats.
    /// * handles various internal SDK callbacks.
    /// </summary>
    class MetaStats
    {
    public:
        MetaStats(IRuntimeConfig& config);
        ~MetaStats();

        std::vector< ::AriaProtocol::Record> generateStatsEvent(RollUpKind rollupKind);

        void updateOnEventIncoming(std::string const& tenanttoken, unsigned size, EventLatency latency, bool metastats);
        void updateOnPostData(unsigned postDataLength, bool metastatsOnly);
        void updateOnPackageSentSucceeded(std::map<std::string, std::string> const& recordIdsAndTenantids, EventLatency eventLatency, unsigned retryFailedTimes, unsigned durationMs, std::vector<unsigned> const& latencyToSendMs, bool metastatsOnly);
        void updateOnPackageFailed(int statusCode);
        void updateOnPackageRetry(int statusCode, unsigned retryFailedTimes);
        void updateOnRecordsDropped(EventDroppedReason reason, std::map<std::string, size_t> const& droppedCount);
        void updateOnRecordsOverFlown(std::map<std::string, size_t> const& overflownCount);
        void updateOnRecordsRejected(EventRejectedReason reason, std::map<std::string, size_t> const& rejectedCount);
        void updateOnStorageOpened(std::string const& type);
        void updateOnStorageFailed(std::string const& reason);

    protected:
        /// <summary>
        /// Clear all frequency distributions. Copied as is from old SCT, not sure it's needed.
        /// </summary>
        void clearStats();

        /// <summary>
        /// Check if there's any stats data available to be sent.
        /// </summary>
        bool hasStatsDataAvailable() const;

        /// <summary>
        /// Reset TelemetryStats
        /// If this function is called during starting SCT, keys of each map for frequency distribution are set;
        /// Otherwise, when this function is called after a stats event is sent, values of each map are cleared.
        /// </summary>
        /// <param name="start">bool, indicate if it is called during starting sct</param>
        void resetStats(bool start = true);

        /// <summary>
        /// stats records created
        /// </summary>
        void snapStatsToRecord(std::vector< ::AriaProtocol::Record>& records, RollUpKind rollupKind);

    protected:

        IRuntimeConfig&                 m_config;

        // TODO: [MG] - allow stats configuration provisioning via IRuntimeConfig above
        StatsConfig                     m_statsConfig;
        
        /// <summary>
        /// Overall telemetry stats
        /// </summary>
        TelemetryStats                  m_telemetryStats;
        
        /// <summary>
        /// Per-tenant stats
        /// </summary>
        std::map<std::string, TelemetryStats>  m_telemetryTenantStats;

    private:
        void privateSnapStatsToRecord(std::vector< ::AriaProtocol::Record>& records, RollUpKind rollupKind, TelemetryStats& telemetryStats);
        void privateClearStats(TelemetryStats& telemetryStats);
    };

} ARIASDK_NS_END

#endif