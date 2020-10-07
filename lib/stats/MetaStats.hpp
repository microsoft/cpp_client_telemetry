//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef METASTATS_HPP
#define METASTATS_HPP

#include "pal/PAL.hpp"

#include "api/IRuntimeConfig.hpp"

#include "Enums.hpp"
#include "CsProtocol_types.hpp"

#include <memory>
#include <algorithm>

namespace MAT_NS_BEGIN {

    /** \brief The first positive spot for the frequency distribution of package consecutive failure duration.
        <20s, 20s~40s, 40s~80s, 80s~160s, 160s~320s, 320s~640s, >640s
     */
    const unsigned int STATS_PACKAGE_CONSECUTIVE_FAILURE_FIRST_DURATION_IN_SECS = 20;

    /** \brief The factor used to calculate next spot of frequency distribution of package consecutive failure duration.
        <20s, 20s~40s, 40s~80s, 80s~160s, 160s~320s, 320s~640s, >640s
     */
    const unsigned int STATS_PACKAGE_CONSECUTIVE_FAILURE_NEXT_FACTOR = 2;

    /** \brief The total spots of frequency distribution of package consecutive failure duration.
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

    /** \brief The first positive spot for the frequency distribution for saved size, overwritten size.
        <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB
     */
    const unsigned int STATS_STORAGE_SIZE_FIRST_IN_KB = 8;

    /** \brief The factor used to calculate next spot of frequency distribution for saved size, overwritten size.
        <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB
     */
    const unsigned int STATS_STORAGE_SIZE_NEXT_FACTOR = 2;

    /** \brief The total spots of frequency distribution for saved size, overwritten size.
        <8KB, 8~16KB, 16~32KB, 32~64KB, 64KB~128KB, 128KB~256KB, 256KB~512KB,> 512KB
     */
    const unsigned int STATS_STORAGE_SIZE_TOTAL_SPOTS = 8;

    /// <summary>
    /// Define a new name for a map whose key, value are both unsigned int.
    /// </summary>
    typedef std::map<unsigned int, unsigned int> uint_uint_dict_t;

    /// <summary>
    /// Define a new name for a map whose key is string and value is unsigned int.
    /// </summary>
    typedef std::map<std::string, unsigned int> string_uint_dict_t;

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
        unsigned int banned;

        /// total number of records received
        unsigned int received;

        /// total number of metastats records received
        unsigned int receivedStats;

        /// the number of records rejected by SDK
        unsigned int rejected;

        /// distribution of records count by reason due to which record was rejected
        uint_uint_dict_t rejectedByReason;

        /// the number of dropped records
        unsigned int dropped;

        /// the number of overflown records
        unsigned int overflown;

        /// distribution of records count by reason due to which record was dropped
        uint_uint_dict_t droppedByReason;

        /// key: http return code
        /// value: the number of records declined by collector (either rejected or dropped due to server error) per each HTTP return code
        uint_uint_dict_t droppedByHTTPCode;

        /// the number of sent records
        unsigned int sent;

        /// the delta of events added/removed from in flight
        unsigned int inflight;

        /// min record size
        unsigned int minOfRecordSizeInBytes;

        /// max record size
        unsigned int maxOfRecordSizeInBytes;

        /// total records' size in bytes
        unsigned int totalRecordsSizeInBytes;

        ///<1KB, 1KB~2KB, 2KB~4KB, 4KB~8KB, 8KB~16KB, 16KB~32KB, 32KB~64KB, > 64KB \n
        ///key: min value of each size range \n
        ///value: the number of records with size in given range
        // uint_uint_dict_t sizeInKBytesDistribution;

        /// min record size which is dropped in SendAsync
        // unsigned int minOfDroppedRecordSizeInBytes;

        /// max record size which is dropped in SendAsync
        // unsigned int maxOfDroppedRecordSizeInBytes;

        /// reset all members
        void Reset()
        {
            banned = 0;
            received = 0;
            receivedStats = 0;
            dropped = 0;
            rejected = 0;
            overflown = 0;
            sent = 0;
            inflight = 0;

            minOfRecordSizeInBytes = static_cast<unsigned int>(~0);
            maxOfRecordSizeInBytes = 0;
            totalRecordsSizeInBytes = 0;

            droppedByReason.clear();
            rejectedByReason.clear();
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
        MATSDK_LOG_DECL_COMPONENT_CLASS();
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

            saveSizeInKBytesDistribution.clear();
            successSaveSizeInKBytesDistribution.clear();
            overwrittenSizeInKBytesDistribution.clear();
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

        /// SDK session ID GUID
        std::string sessionId;

        /// SDK sesion start UTC timestamp
        int64_t sessionStartTimestamp;

        /// Stats interval UTC timestamp
        int64_t statsStartTimestamp;

        /// the sequence number of the new telemetry stats within the session as indicated by the sessionId
        int64_t statsSequenceNum;

        /// Package statistics
        PackageStats packageStats;

        /// Retry count by code distribution
        uint_uint_dict_t retriesCountDistribution;

        /// RTT stats
        LatencyStats rttStats;

        RecordStats recordStats;

        std::map<EventLatency, RecordStats> recordStatsPerLatency;
        
        OfflineStorageStats offlineStorageStats;

        void Reset()
        {
            packageStats.Reset();
            retriesCountDistribution.clear();
            rttStats.Reset();
            recordStats.Reset();
            recordStatsPerLatency.clear();
            offlineStorageStats.Reset();
        }
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

        std::vector< ::CsProtocol::Record> generateStatsEvent(RollUpKind rollupKind);

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
        void rollup(std::vector< ::CsProtocol::Record>& records, RollUpKind rollupKind);

    protected:

        IRuntimeConfig&                 m_config;

        StatsConfig                     m_statsConfig;
        
        /// <summary>
        /// Overall telemetry stats
        /// </summary>
        TelemetryStats                  m_telemetryStats;

        /// <summary>
        /// Stats Session ID shared between all tenant stats
        /// </summary>
        std::string                     m_sessionId;

        /// <summary>
        /// Flag to enable per-tenant stats logic
        /// </summary>
        bool                            m_enableTenantStats;

        /// <summary>
        /// Per-tenant stats
        /// </summary>
        std::map<std::string, TelemetryStats>  m_telemetryTenantStats;

        const std::map<EventLatency, std::string> m_latency_pfx =
        {
            { EventLatency_Normal,       "ln_" },
            { EventLatency_CostDeferred, "ld_" },
            { EventLatency_RealTime,     "lr_" },
            { EventLatency_Max,          "lm_" }
        };

        const std::map<EventRejectedReason, std::string>   m_reject_reasons =
        {
            { REJECTED_REASON_VALIDATION_FAILED,            "rej_inv" },
            { REJECTED_REASON_OLD_RECORD_VERSION,           "rej_old" },
            { REJECTED_REASON_INVALID_CLIENT_MESSAGE_TYPE,  "rej_typ" },
            { REJECTED_REASON_REQUIRED_ARGUMENT_MISSING,    "rej_ams" },
            { REJECTED_REASON_EVENT_NAME_MISSING,           "rej_nms" },
            { REJECTED_REASON_EVENT_SIZE_LIMIT_EXCEEDED,    "rej_siz" },
            { REJECTED_REASON_EVENT_BANNED,                 "rej_ban" },
            { REJECTED_REASON_EVENT_EXPIRED,                "rej_exp" },
            { REJECTED_REASON_SERVER_DECLINED,              "rej_403" },
            { REJECTED_REASON_TENANT_KILLED,                "rej_kl" }
        };

    private:
        void snapStatsToRecord(std::vector< ::CsProtocol::Record>& records, RollUpKind rollupKind, TelemetryStats& telemetryStats);
    };

} MAT_NS_END

#endif

