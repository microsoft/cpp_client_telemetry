// Copyright (c) Microsoft. All rights reserved.

#include "MetaStats.hpp"
#include <utils/Utils.hpp>
#include <algorithm>

namespace ARIASDK_NS_BEGIN {

/// <summary>
///  Maximum number of priorities supported.
///  This needs to be kept in sync with EventPriority in API
/// </summary>
static int const g_EventPriority_Count = 5;

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

// Event rejected due to legit reasoning
enum EventRejectedReason
{
    REJECTED_REASON_REQUIRED_ARGUMENT_MISSING,
    REJECTED_REASON_EVENT_NAME_MISSING,
    REJECTED_REASON_INVALID_CLIENT_MESSAGE_TYPE,
    REJECTED_REASON_VALIDATION_FAILED,
    REJECTED_REASON_OLD_RECORD_VERSION,
    REJECTED_REASON_EVENT_EXPIRED,
    REJECTED_REASON_COUNT
};
static unsigned const gc_NumRejectedReasons = REJECTED_REASON_COUNT;


/// <summary>
/// A structure defining some configurations used in collecting stats,
/// including sending frequency and frequency distributions of multiple data sets.
/// </summary>
struct StatsConfig
{
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
/// A structure to define all the telemetry statistics.
/// </summary>
struct TelemetryStats
{
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

    /// the utc timestamp of starting collecting new telemetry stats.
    int64_t statsStartTimestamp;

    /// the sequence number of the new telemetry stats within the session as indicated by the sessionId
    int64_t statsSequenceNum;

    /// a structure on package stats.
    struct PackageStats
    {
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
            totalPkgsNotToBeAcked = 0;
            totalPkgsToBeAcked = 0;
            totalMetastatsOnlyPkgsToBeAcked = 0;
            totalPkgsAcked = 0;
            totalMetastatsOnlyPkgsAcked = 0;
            successPkgsAcked = 0;
            dropPkgsAcked = 0;
            retryPkgsAcked = 0;
            totalBandwidthConsumedInBytes = 0;
        }
    } packageStats;

    /// a structure on bandwidth stats
    struct BandwidthStats
    {
        /// number of bandwidth allocation changes by resouce manager
        unsigned int bwChangedCount;

        /// total bandwidth in Bps allocated by resource manager
        /// used to calculate the Bps on average
        unsigned int totalBwInBps;

        /// max bandwidth in Bps allocated by resource manager
        unsigned int maxBwInBps;

        /// min bandwidth in Bps allocated by resource manager
        unsigned int minBwInBps;

        /// reset all members
        void Reset()
        {
            bwChangedCount = static_cast<unsigned int>(0);
            totalBwInBps = static_cast<unsigned int>(0);
            maxBwInBps = static_cast<unsigned int>(0);
            minBwInBps = static_cast<unsigned int>(~0);
        }
    } bandwidthStats;

    /// a structure on httpstack retries
    struct InternalHttpStackRetriesStats
    {
        ///key: internal httpstack retries count: 0, 1, 2, 3. 4 \n
        ///value: the count of packages, whose retries number equals to given key
        uint_uint_dict_t retriesCountDistribution;
    } internalHttpStackRetriesStats;

    /// a structure on latency stats
    struct LatencyStats
    {
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
    } rttStats, logToSuccessfulSendLatencyPerPriority[g_EventPriority_Count];

    /// a structure on record stats
    struct RecordStats
    {
        /// total number of records received
        unsigned int bannedCount;

        /// total number of records received
        unsigned int receivedCount;

        /// total number of metastats records received
        unsigned int receivedMetastatsCount;

        /// the number of records rejected by SDK
        unsigned int rejectedCount;

        /// distribution of records count by reason due to which record was rejected
        unsigned int rejectedCountReasonDistribution[gc_NumRejectedReasons];

        /// the number of dropped records
        unsigned int droppedCount;

        /// distribution of records count by reason due to which record was dropped
        unsigned int droppedCountReasonDistribution[gc_NumDroppedReasons];

        /// key: http return code
        /// value: the number of records declined by collector (either rejected or dropped due to server error) per each HTTP return code
        uint_uint_dict_t droppedCountPerHttpReturnCode;

        /// the number of sent records
        unsigned int sentCount;

        /// the number of sent records from the current session
        unsigned int sentCountFromCurrentSession;

        /// the number of sent records from previous sessions
        unsigned int sentCountFromPreviousSession;

        /// the total number of records per semantic
        std::map<std::string, unsigned int> semanticToRecordCountMap;

        /// TODO: do we need this anymore if we are going to count the reasons frop dropping events?
        /// the total number of exceptions per semantic
        std::map<std::string, unsigned int> semanticToExceptionCountMap;

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
            sentCount = 0;
            sentCountFromCurrentSession = 0;
            sentCountFromPreviousSession = 0;
            minOfRecordSizeInBytes = static_cast<unsigned int>(~0);
            maxOfRecordSizeInBytes = 0;
            totalRecordsSizeInBytes = 0;
            minOfDroppedRecordSizeInBytes = static_cast<unsigned int>(~0);;
            maxOfDroppedRecordSizeInBytes = 0;
            std::fill_n(droppedCountReasonDistribution, gc_NumDroppedReasons, 0);
            std::fill_n(rejectedCountReasonDistribution, gc_NumRejectedReasons, 0);
        }
    } recordStats, recordStatsPerPriority[g_EventPriority_Count];

    /// a structure on offline storage stats
    struct OfflineStorageStats
    {
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
        }
    } offlineStorageStats;
};


static char const* ActRollUpKindToString(ActRollUpKind rollupKind)
{
    switch (rollupKind) {
        case ACT_STATS_ROLLUP_KIND_START:   return "start";

        case ACT_STATS_ROLLUP_KIND_STOP:    return "stop";

        case ACT_STATS_ROLLUP_KIND_ONGOING: return "ongoing";

        default:                            return "unknown";
    }
}

/// <summary>
/// Initialize keys in each frequency distribution
/// </summary>
/// <param name="firstValue">the first non-zero value of distribution spot</param>
/// <param name="increment">used to calculate next spot</param>
/// <param name="totalSpot">total number of spots, including the first 0</param>
/// <param name="distribution">map</param>
/// <param name="factor">if true, next spot = last spot * increment; otherwise, next spot = last spot + increment</param>
void initDistributionKeys(unsigned int firstValue, unsigned int increment, unsigned int totalSpot, uint_uint_dict_t& distribution, bool factor = true)
{
    distribution.clear();
    distribution[0] = 0;
    unsigned int lastkey = 0;
    if (factor) {
        for (unsigned int i = 1; i < totalSpot; ++i) {
            unsigned int key = (lastkey == 0) ? firstValue : (increment * lastkey);
            distribution[key] = 0;
            lastkey = key;
        }
    } else {
        for (unsigned int i = 1; i < totalSpot; ++i) {
            unsigned int key = (lastkey == 0) ? firstValue : (lastkey + increment);
            distribution[key] = 0;
            lastkey = key;
        }
    }
}

/// <summary>
/// Update the occurence within the corresponding group of Map distribution
/// </summary>
/// <param name="distribution">a distribution to be updated</param>
/// <param name="value">unsigned int, sample value, must be in some group of the given distribution</param>
void updateMap(uint_uint_dict_t& distribution, unsigned int value)
{
    if (distribution.empty()) {
        return;
    }

    uint_uint_dict_t::iterator it = distribution.begin();
    for (; it != distribution.end(); ++it) {
        if (value < it->first) {
            break;
        }
    }

    if (it == distribution.begin()) {
        // If value is not in any range, we still put it in first range.
        //ARIASDK_LOG_WARNING("value %u is less than distribution start (< %u)", value, it->first);
        it->second++;
    } else {
        (--it)->second++;
    }
}

/// <summary>
/// A template function with typename T.
/// The definition and implement must be in the same file.
/// Only Clear values of each frequency distribution while keys are maintained.
/// </summary>
/// <param name="distribution">map<T, unsigned int></param>
template<typename T>
void clearMapValues(std::map<T, unsigned int>& distribution)
{
    for (auto& item : distribution) {
        item.second = 0;
    }
}

template<typename T>
static void insertNonZero(std::map<std::string, std::string>& target, std::string const& key, T const& value)
{
    if (value != 0) {
        target[key] = toString(value);
    }
}

/// <summary>
/// Add A Map struture map to Record Extension Field
/// For example,
/// 1) range distribution
/// map = { 1:2, 2 : 3, 3 : 4 }
/// record.extension = { "name_0_1":"2", "name_1_2" : "3", "name_3_plus" : "4" }
/// 2) otherwise
/// record.extension = { "name_1":"2", "name_2" : "3", "name_3" : "4" }
/// </summary>
/// <param name="record">telemetry::Record</param>
/// <param name="distributionName">prefix of extension key name</param>
/// <param name="distribution">map<unsigned int, unsigned int></param>
/// <param name="range">indicate if the frequency distribution stored in map is based on multiple groups or multiple points</param>
static void addAggregatedMapToRecordFields(::AriaProtocol::Record& record, std::string const& distributionName,
    uint_uint_dict_t const& distribution, bool range = true)
{
    if (distribution.empty()) {
        return;
    }
    std::map<std::string, std::string>& ext = record.Extension;
    uint_uint_dict_t::const_iterator it, next;
    std::string fieldValue;

    // ext field will be in format distributionName_i_j
    for (it = distribution.begin(), next = it; it != distribution.end(); ++it) {
        ++next;
        if (next == distribution.end()) {
            if (range) {
                fieldValue += ">" + toString(it->first) + ":" + toString(it->second);
            } else {
                fieldValue += toString(it->first) + ":" + toString(it->second);
            }
        } else {
            if (range) {
                fieldValue += toString(it->first) + "-" + toString(next->first) + ":" + toString(it->second) + ",";
            } else {
                fieldValue += toString(it->first) + ":" + toString(it->second) + ",";
            }
        }
    }

    ext[distributionName] = fieldValue;
}

/// <summary>
/// Add A Map struture to Record Extension Field
/// For example,
/// map= {"a":2, "b":3, "c":4}
/// record.extension = {"name_a":"2", "name_b":"3", "name_c":"4"}
/// </summary>
/// <param name="record">telemetry::Record</param>
/// <param name="distributionName">prefix of the key name in record extension map</param>
/// <param name="distribution">map<std::string, unsigned int>, key is the source of event</param>
static void addAggregatedMapToRecordFields(::AriaProtocol::Record& record, std::string const& distributionName,
    string_uint_dict_t const& distribution)
{
    if (distribution.empty()) {
        return;
    }

    std::string fieldValue;

    // ext field will be in format distributionPrefix_i if distributionPrefix given
    for (std::map<std::string, unsigned int>::const_iterator it = distribution.begin(), next = it;
        it != distribution.end(); ++it)
    {
        ++next;
        if (next == distribution.end()) {
            fieldValue += it->first + ":" + toString(it->second);
        } else {
            fieldValue += it->first + ":" + toString(it->second) + ",";
        }
    }

    record.Extension[distributionName] = fieldValue;
}

/// <summary>
/// Add count per each HTTP recode code to Record Extension Field
/// </summary>
/// <param name="record">telemetry::Record</param>
/// <param name="distributionName">prefix of the key name in record extension map</param>
/// <param name="distribution">map<unsigned int, unsigned int>, key is the http return code, value is the count</param>
static void addCountsPerHttpReturnCodeToRecordFields(::AriaProtocol::Record& record, std::string const& prefix,
    uint_uint_dict_t const countsPerHttpReturnCodeMap)
{
    if (countsPerHttpReturnCodeMap.empty()) {
        return;
    }

    for (auto const& item : countsPerHttpReturnCodeMap) {
        insertNonZero(record.Extension, prefix + "_" + toString(item.first), item.second);
    }
}

/// <summary>
/// Add rejected count by reason to Record Extension Field
/// </summary>
/// <param name="record">BondTypes::Record</param>
/// <param name="prefix">prefix of the key name in record extension map</param>
/// <param name="recordsRejectedCountReasonDistribution">count of rejected records by reason due to which records were rejected</param>
static void addRecordsPerRejectedReasonToRecordFields(::AriaProtocol::Record& record, std::string const& prefix, const unsigned int recordsRejectedCountByReasonDistribution[])
{
    std::map<std::string, std::string>& extension = record.Extension;

    insertNonZero(extension, prefix + "invalid_message_type",        recordsRejectedCountByReasonDistribution[REJECTED_REASON_INVALID_CLIENT_MESSAGE_TYPE]);
    insertNonZero(extension, prefix + "required_argument_missing",   recordsRejectedCountByReasonDistribution[REJECTED_REASON_REQUIRED_ARGUMENT_MISSING]);
    insertNonZero(extension, prefix + "event_name_missing",          recordsRejectedCountByReasonDistribution[REJECTED_REASON_EVENT_NAME_MISSING]);
    insertNonZero(extension, prefix + "validation_failed",           recordsRejectedCountByReasonDistribution[REJECTED_REASON_VALIDATION_FAILED]);
    insertNonZero(extension, prefix + "old_record_version",          recordsRejectedCountByReasonDistribution[REJECTED_REASON_OLD_RECORD_VERSION]);
    insertNonZero(extension, prefix + "event_expired",               recordsRejectedCountByReasonDistribution[REJECTED_REASON_EVENT_EXPIRED]);
}

//---

ARIASDK_LOG_INST_COMPONENT_CLASS(MetaStats, "AriaSDK.MetaStats", "Aria SDK statistics");

MetaStats::MetaStats(IRuntimeConfig const& runtimeConfig, ContextFieldsProvider const& parentContext)
  : m_runtimeConfig(runtimeConfig),
    m_statsConfig(new StatsConfig),
    m_telemetryStats(new TelemetryStats),
    m_baseDecorator(""),
    m_semanticContextDecorator(parentContext)
{
    m_telemetryStats->statsStartTimestamp = PAL::getUtcSystemTimeMs();
    resetStats(true);
    //TODO: extend IRuntimeConfig to include these vars
    m_telemetryStats->offlineStorageEnabled = true;
    m_telemetryStats->resourceManagerEnabled = false;
    m_telemetryStats->ecsClientEnabled = false;
}

MetaStats::~MetaStats()
{
}

void MetaStats::resetStats(bool start)
{
    ARIASDK_LOG_DETAIL("resetStats");

    //clear packageStats
    TelemetryStats::PackageStats& packageStats = m_telemetryStats->packageStats;
    packageStats.Reset();

    //clear bandwidthStats
    TelemetryStats::BandwidthStats& bandwidthStats = m_telemetryStats->bandwidthStats;
    bandwidthStats.Reset();

    //clear rttStats
    TelemetryStats::LatencyStats& rttStats = m_telemetryStats->rttStats;
    rttStats.Reset();

    // clear sdkToCollectorLatencyPerPriority stats
    for (auto& item : m_telemetryStats->logToSuccessfulSendLatencyPerPriority) {
        item.Reset();
    }

    //clear recordStats
    TelemetryStats::RecordStats& recordStats = m_telemetryStats->recordStats;
    recordStats.Reset();

    //clear recordStatsPerPriority
    for (auto& item : m_telemetryStats->recordStatsPerPriority) {
        item.Reset();
    }

    //clear offlineStorageStats
    TelemetryStats::OfflineStorageStats& storageStats = m_telemetryStats->offlineStorageStats;
    storageStats.Reset();

    m_telemetryStats->statsStartTimestamp = PAL::getUtcSystemTimeMs();

    if (start) {
        m_telemetryStats->statsSequenceNum = 0;
        m_telemetryStats->sessionStartTimestamp = m_telemetryStats->statsStartTimestamp;
        m_telemetryStats->sessionId = PAL::generateUuidString();
        ARIASDK_LOG_DETAIL("session start, session ID: %s", m_telemetryStats->sessionId.c_str());

        initDistributionKeys(m_statsConfig->rtt_first_duration_in_millisecs, m_statsConfig->rtt_next_factor,
            m_statsConfig->rtt_total_spots, rttStats.latencyDistribution);

        for (auto& item : m_telemetryStats->logToSuccessfulSendLatencyPerPriority) {
            initDistributionKeys(m_statsConfig->latency_first_duration_in_millisecs, m_statsConfig->latency_next_factor,
                m_statsConfig->latency_total_spots, item.latencyDistribution);
        }

        initDistributionKeys(m_statsConfig->record_size_first_in_kb, m_statsConfig->record_size_next_factor,
            m_statsConfig->record_size_total_spots, recordStats.sizeInKBytesDistribution);

        if (m_telemetryStats->offlineStorageEnabled) {
            initDistributionKeys(m_statsConfig->storage_size_first_in_kb, m_statsConfig->storage_size_next_factor,
                m_statsConfig->storage_size_total_spots, storageStats.saveSizeInKBytesDistribution);
            initDistributionKeys(m_statsConfig->storage_size_first_in_kb, m_statsConfig->storage_size_next_factor,
                m_statsConfig->storage_size_total_spots, storageStats.overwrittenSizeInKBytesDistribution);
        }
    } else {
        ARIASDK_LOG_DETAIL("ongoing stats, session ID: %s", m_telemetryStats->sessionId.c_str());
        m_telemetryStats->statsSequenceNum += 1;
        packageStats.dropPkgsPerHttpReturnCode.clear();
        packageStats.retryPkgsPerHttpReturnCode.clear();

        TelemetryStats::InternalHttpStackRetriesStats& httpstackStats = m_telemetryStats->internalHttpStackRetriesStats;
        httpstackStats.retriesCountDistribution.clear();

        clearMapValues(rttStats.latencyDistribution);

        for (auto& item : m_telemetryStats->logToSuccessfulSendLatencyPerPriority) {
            clearMapValues(item.latencyDistribution);
        }

        clearMapValues(recordStats.semanticToRecordCountMap);
        clearMapValues(recordStats.semanticToExceptionCountMap);
        clearMapValues(recordStats.sizeInKBytesDistribution);

        recordStats.droppedCountPerHttpReturnCode.clear();

        if (m_telemetryStats->offlineStorageEnabled) {
            clearMapValues(storageStats.saveSizeInKBytesDistribution);
            clearMapValues(storageStats.overwrittenSizeInKBytesDistribution);
        }
    }
}

void MetaStats::snapStatsToRecord(std::vector< ::AriaProtocol::Record>& records, ActRollUpKind rollupKind)
{
    ARIASDK_LOG_DETAIL("snapStatsToRecord");

    ::AriaProtocol::Record record;
    record.Type                      = "client_telemetry";
    record.EventType                 = "act_stats";
    record.Extension["act_stats_id"] = m_telemetryStats->sessionId;

    std::map<std::string, std::string>& ext = record.Extension;

    //basic Fields
    //Add the tenantID (not the entire tenantToken) to the stats event
    ext["TenantId"] = m_telemetryStats->tenantId;

    // session fileds
    insertNonZero(ext, "session_start_timestamp", m_telemetryStats->sessionStartTimestamp);
    insertNonZero(ext, "stats_start_timestamp", m_telemetryStats->statsStartTimestamp);
    insertNonZero(ext, "stats_end_timestamp", PAL::getUtcSystemTimeMs());
    ext["stats_rollup_kind"] = ActRollUpKindToString(rollupKind);
    insertNonZero(ext, "stats_send_frequency_secs", m_runtimeConfig.GetMetaStatsSendIntervalSec());

    if (m_telemetryStats->offlineStorageEnabled) {
        const TelemetryStats::OfflineStorageStats& storageStats = m_telemetryStats->offlineStorageStats;
        ext["offline_storage_format_type"] = storageStats.storageFormat;
        if (!storageStats.lastFailureReason.empty()) {
            ext["offline_storage_last_failure"] = storageStats.lastFailureReason;
        }
        insertNonZero(ext, "config_offline_storage_size_bytes", storageStats.fileSizeInBytes);
    }

    //package stats
    const TelemetryStats::PackageStats& packageStats = m_telemetryStats->packageStats;
    insertNonZero(ext, "requests_not_to_be_acked", packageStats.totalPkgsNotToBeAcked);
    insertNonZero(ext, "requests_to_be_acked", packageStats.totalPkgsToBeAcked);
    insertNonZero(ext, "requests_acked", packageStats.totalPkgsAcked);
    insertNonZero(ext, "requests_acked_succeeded", packageStats.successPkgsAcked);
    insertNonZero(ext, "requests_acked_retried", packageStats.retryPkgsAcked);
    insertNonZero(ext, "requests_acked_dropped", packageStats.dropPkgsAcked);
    addCountsPerHttpReturnCodeToRecordFields(record, "requests_acked_dropped_on_HTTP", packageStats.dropPkgsPerHttpReturnCode);
    addCountsPerHttpReturnCodeToRecordFields(record, "requests_acked_retried_on_HTTP", packageStats.retryPkgsPerHttpReturnCode);

    insertNonZero(ext, "rm_bw_bytes_consumed_count", packageStats.totalBandwidthConsumedInBytes);

    //bandwidth stats
    const TelemetryStats::BandwidthStats& bandwidthStats = m_telemetryStats->bandwidthStats;
    if (bandwidthStats.bwChangedCount > 0) {
        ARIASDK_LOG_DETAIL("max, min, avg bandwidth are added to record extension field");

        insertNonZero(ext, "rm_bw_allocation_changes", bandwidthStats.bwChangedCount);
        insertNonZero(ext, "rm_bw_allocated_avg", bandwidthStats.totalBwInBps / bandwidthStats.bwChangedCount);
        insertNonZero(ext, "rm_bw_allocated_max", bandwidthStats.maxBwInBps);
        insertNonZero(ext, "rm_bw_allocated_min", bandwidthStats.minBwInBps);
    }

    //InternalHttpStackRetriesStats
    if (packageStats.totalPkgsAcked > 0) {
        ARIASDK_LOG_DETAIL("httpstack_retries stats is added to record extension field");
        addAggregatedMapToRecordFields(record, "requests_fail_on_HTTP_retries_count_distribution",
            m_telemetryStats->internalHttpStackRetriesStats.retriesCountDistribution, false);
    }

    //RTTStats
    if (packageStats.successPkgsAcked > 0) {
        ARIASDK_LOG_DETAIL("rttStats is added to record ext field");
        const TelemetryStats::LatencyStats& rttStats = m_telemetryStats->rttStats;
        insertNonZero(ext, "rtt_millisec_max", rttStats.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "rtt_millisec_min", rttStats.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "rtt_millisec_distribution", rttStats.latencyDistribution);
    }

    //RecordStats
    const TelemetryStats::RecordStats& recordStats = m_telemetryStats->recordStats;

    insertNonZero(ext, "records_banned_count", recordStats.bannedCount);

    insertNonZero(ext, "records_received_count", recordStats.receivedCount);

    insertNonZero(ext, "records_sent_count", recordStats.sentCount);
    insertNonZero(ext, "records_sent_curr_session", recordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "records_sent_prev_session", recordStats.sentCountFromPreviousSession);

    insertNonZero(ext, "records_rejected_count", recordStats.rejectedCount);
    addRecordsPerRejectedReasonToRecordFields(record, "records_rejected_", recordStats.rejectedCountReasonDistribution);

    insertNonZero(ext, "records_dropped_count", recordStats.droppedCount);
    insertNonZero(record.Extension, "records_dropped_offline_storage_save_failed", recordStats.droppedCountReasonDistribution[DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED]);
    insertNonZero(record.Extension, "records_dropped_offline_storage_overflow",    recordStats.droppedCountReasonDistribution[DROPPED_REASON_OFFLINE_STORAGE_OVERFLOW]);
    insertNonZero(record.Extension, "records_dropped_server_declined_4xx",         recordStats.droppedCountReasonDistribution[DROPPED_REASON_SERVER_DECLINED_4XX]);
    insertNonZero(record.Extension, "records_dropped_server_declined_5xx",         recordStats.droppedCountReasonDistribution[DROPPED_REASON_SERVER_DECLINED_5XX]);
    insertNonZero(record.Extension, "records_dropped_server_declined_other",       recordStats.droppedCountReasonDistribution[DROPPED_REASON_SERVER_DECLINED_OTHER]);
    insertNonZero(record.Extension, "records_dropped_retry_exceeded",              recordStats.droppedCountReasonDistribution[DROPPED_REASON_RETRY_EXCEEDED]);
    addCountsPerHttpReturnCodeToRecordFields(record, "records_dropped_on_HTTP", recordStats.droppedCountPerHttpReturnCode);

    addAggregatedMapToRecordFields(record, "exceptions_per_eventtype_count", recordStats.semanticToExceptionCountMap);
    addAggregatedMapToRecordFields(record, "records_per_eventtype_count", recordStats.semanticToRecordCountMap);

    if (recordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("source stats and record size stats in recordStats"
            " are added to record ext field");
        insertNonZero(ext, "record_size_bytes_max", recordStats.maxOfRecordSizeInBytes);
        insertNonZero(ext, "record_size_bytes_min", recordStats.minOfRecordSizeInBytes);
        insertNonZero(ext, "records_received_size_bytes", recordStats.totalRecordsSizeInBytes);
        addAggregatedMapToRecordFields(record, "record_size_kb_distribution", recordStats.sizeInKBytesDistribution);
    }

    // per priority RecordStats

    // Low priority RecordStats
    const TelemetryStats::RecordStats& lowPriorityrecordStats = m_telemetryStats->recordStatsPerPriority[EventPriority_Low];
    insertNonZero(ext, "low_priority_records_banned_count",                 lowPriorityrecordStats.bannedCount);
    insertNonZero(ext, "low_priority_records_received_count",               lowPriorityrecordStats.receivedCount);
    insertNonZero(ext, "low_priority_records_sent_count",                   lowPriorityrecordStats.sentCount);
    insertNonZero(ext, "low_priority_records_sent_count_current_session",   lowPriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "low_priority_records_sent_count_previous_sessions", lowPriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "low_priority_records_dropped_count",                lowPriorityrecordStats.droppedCount);
    insertNonZero(ext, "low_priority_records_rejected_count",               lowPriorityrecordStats.rejectedCount);
    if (lowPriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("Low priority source stats and record size stats in recordStats"
            " are added to record ext field");
        insertNonZero(ext, "low_priority_records_received_size_bytes",      lowPriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (lowPriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyLow = m_telemetryStats->logToSuccessfulSendLatencyPerPriority[EventPriority_Low];
        insertNonZero(ext, "low_priority_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyLow.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "low_priority_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyLow.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "low_priority_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyLow.latencyDistribution);
    }

    // Normal priority RecordStats
    const TelemetryStats::RecordStats& normalPriorityrecordStats = m_telemetryStats->recordStatsPerPriority[EventPriority_Normal];
    insertNonZero(ext, "normal_priority_records_banned_count",                 normalPriorityrecordStats.bannedCount);
    insertNonZero(ext, "normal_priority_records_received_count",               normalPriorityrecordStats.receivedCount);
    insertNonZero(ext, "normal_priority_records_sent_count",                   normalPriorityrecordStats.sentCount);
    insertNonZero(ext, "normal_priority_records_sent_count_current_session",   normalPriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "normal_priority_records_sent_count_previous_sessions", normalPriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "normal_priority_records_dropped_count",                normalPriorityrecordStats.droppedCount);
    insertNonZero(ext, "normal_priority_records_rejected_count",               normalPriorityrecordStats.rejectedCount);
    if (normalPriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("Normal priority source stats and record size stats in recordStats"
            " are added to record ext field");
        insertNonZero(ext, "normal_priority_records_received_size_bytes",      normalPriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (normalPriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyNormal = m_telemetryStats->logToSuccessfulSendLatencyPerPriority[EventPriority_Normal];
        insertNonZero(ext, "normal_priority_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyNormal.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "normal_priority_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyNormal.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "normal_priority_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyNormal.latencyDistribution);
    }

    // High priority RecordStats
    const TelemetryStats::RecordStats& highPriorityrecordStats = m_telemetryStats->recordStatsPerPriority[EventPriority_High];
    insertNonZero(ext, "high_priority_records_banned_count",                 highPriorityrecordStats.bannedCount);
    insertNonZero(ext, "high_priority_records_received_count",               highPriorityrecordStats.receivedCount);
    insertNonZero(ext, "high_priority_records_sent_count",                   highPriorityrecordStats.sentCount);
    insertNonZero(ext, "high_priority_records_sent_count_current_session",   highPriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "high_priority_records_sent_count_previous_sessions", highPriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "high_priority_records_dropped_count",                highPriorityrecordStats.droppedCount);
    insertNonZero(ext, "high_priority_records_rejected_count",               highPriorityrecordStats.rejectedCount);
    if (highPriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("High priority source stats and record size stats in recordStats"
            " are added to record ext field");
        insertNonZero(ext, "high_priority_records_received_size_bytes",      highPriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (highPriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyHigh = m_telemetryStats->logToSuccessfulSendLatencyPerPriority[EventPriority_High];
        insertNonZero(ext, "high_priority_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyHigh.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "high_priority_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyHigh.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "high_priority_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyHigh.latencyDistribution);
    }

    // Immediate priority RecordStats
    const TelemetryStats::RecordStats& immediatePriorityrecordStats = m_telemetryStats->recordStatsPerPriority[EventPriority_Immediate];
    insertNonZero(ext, "immediate_priority_records_banned_count",                 immediatePriorityrecordStats.bannedCount);
    insertNonZero(ext, "immediate_priority_records_received_count",               immediatePriorityrecordStats.receivedCount);
    insertNonZero(ext, "immediate_priority_records_sent_count",                   immediatePriorityrecordStats.sentCount);
    insertNonZero(ext, "immediate_priority_records_sent_count_current_session",   immediatePriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "immediate_priority_records_sent_count_previous_sessions", immediatePriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "immediate_priority_records_dropped_count",                immediatePriorityrecordStats.droppedCount);
    insertNonZero(ext, "immediate_priority_records_rejected_count",               immediatePriorityrecordStats.rejectedCount);
    if (immediatePriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("Immediate priority source stats and record size stats in recordStats are added to record ext field");
        insertNonZero(ext, "immediate_priority_records_received_size_bytes",      immediatePriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (immediatePriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyImmediate = m_telemetryStats->logToSuccessfulSendLatencyPerPriority[EventPriority_Immediate];
        insertNonZero(ext, "immediate_priority_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyImmediate.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "immediate_priority_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyImmediate.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "immediate_priority_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyImmediate.latencyDistribution);
    }

#if 0
    // Background priority RecordStats
    TelemetryStats::RecordStats& bgPriorityrecordStats = m_telemetryStats->recordStatsPerPriority[EventPriority_Background];
    insertNonZero(ext, "bg_priority_records_banned_count",                 bgPriorityrecordStats.bannedCount);
    insertNonZero(ext, "bg_priority_records_received_count",               bgPriorityrecordStats.receivedCount);
    insertNonZero(ext, "bg_priority_records_sent_count",                   bgPriorityrecordStats.sentCount);
    insertNonZero(ext, "bg_priority_records_sent_count_current_session",   bgPriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "bg_priority_records_sent_count_previous_sessions", bgPriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "bg_priority_records_dropped_count",                bgPriorityrecordStats.droppedCount);
    insertNonZero(ext, "bg_priority_records_rejected_count",               bgPriorityrecordStats.rejectedCount);
    if (bgPriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("Background priority source stats and record size stats in recordStats are added to record ext field");
        insertNonZero(ext, "bg_priority_records_received_size_bytes", bgPriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (bgPriorityrecordStats.sentCount > 0) {
        TelemetryStats::LatencyStats& logToSuccessfulSendLatencyBg = m_telemetryStats->logToSuccessfulSendLatencyPerPriority[EventPriority_Background];
        insertNonZero(ext, "bg_priority_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyBg.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "bg_priority_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyBg.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "bg_priority_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyBg.latencyDistribution);
    }
#endif

    m_semanticContextDecorator.decorate(record);
    m_baseDecorator.decorate(record, EventPriority_Normal);
    records.push_back(record);
}

void MetaStats::clearStats()
{
    ARIASDK_LOG_DETAIL("clearStats");

    //clear distribution in packageStats
    m_telemetryStats->packageStats.dropPkgsPerHttpReturnCode.clear();
    m_telemetryStats->packageStats.retryPkgsPerHttpReturnCode.clear();

    //clear distribution in internalHttpStackRetriesStats
    m_telemetryStats->internalHttpStackRetriesStats.retriesCountDistribution.clear();

    //clear distribution in rttStats
    m_telemetryStats->rttStats.latencyDistribution.clear();

    for (auto& item : m_telemetryStats->logToSuccessfulSendLatencyPerPriority) {
        item.latencyDistribution.clear();
    }

    //clear disributions in recordStats
    TelemetryStats::RecordStats& recordStats = m_telemetryStats->recordStats;
    recordStats.sizeInKBytesDistribution.clear();
    recordStats.semanticToRecordCountMap.clear();
    recordStats.semanticToExceptionCountMap.clear();
    recordStats.droppedCountPerHttpReturnCode.clear();

    //clear disributions in offlineStorageStats
    TelemetryStats::OfflineStorageStats& storageStats = m_telemetryStats->offlineStorageStats;
    storageStats.saveSizeInKBytesDistribution.clear();
    storageStats.overwrittenSizeInKBytesDistribution.clear();
}

bool MetaStats::hasStatsDataAvailable() const
{
    return (m_telemetryStats->recordStats.rejectedCount > 0 ||   // not used
           m_telemetryStats->recordStats.bannedCount > 0 ||      // not used
           m_telemetryStats->recordStats.droppedCount > 0 ||     // not used
           m_telemetryStats->recordStats.receivedCount > m_telemetryStats->recordStats.receivedMetastatsCount ||
           m_telemetryStats->packageStats.totalPkgsAcked > m_telemetryStats->packageStats.totalMetastatsOnlyPkgsAcked ||
           m_telemetryStats->packageStats.totalPkgsToBeAcked > m_telemetryStats->packageStats.totalMetastatsOnlyPkgsToBeAcked);
}

std::vector< ::AriaProtocol::Record> MetaStats::generateStatsEvent(ActRollUpKind rollupKind)
{
    ARIASDK_LOG_DETAIL("generateStatsEvent");

    std::vector< ::AriaProtocol::Record> records;

    if (hasStatsDataAvailable()) {
        snapStatsToRecord(records, rollupKind);

        resetStats(false);
    }

    if (rollupKind == ACT_STATS_ROLLUP_KIND_STOP) {
        clearStats();
    }

    return records;
}

void MetaStats::updateOnEventIncoming(unsigned size, EventPriority priority, bool metastats)
{
    TelemetryStats::RecordStats& recordStats = m_telemetryStats->recordStats;
    recordStats.receivedCount++;
    if (metastats) {
        recordStats.receivedMetastatsCount++;
    }

    updateMap(recordStats.sizeInKBytesDistribution, size / 1024);

    recordStats.maxOfRecordSizeInBytes = std::max<unsigned>(recordStats.maxOfRecordSizeInBytes, size);
    recordStats.minOfRecordSizeInBytes = std::min<unsigned>(recordStats.minOfRecordSizeInBytes, size);
    recordStats.totalRecordsSizeInBytes += size;

    if (priority >= 0) {
        TelemetryStats::RecordStats& recordStatsPerPriority = m_telemetryStats->recordStatsPerPriority[priority];
        recordStatsPerPriority.receivedCount++;
        recordStatsPerPriority.totalRecordsSizeInBytes += size;
    }
}

void MetaStats::updateOnPostData(unsigned postDataLength, bool metastatsOnly)
{
    m_telemetryStats->packageStats.totalBandwidthConsumedInBytes += postDataLength;
    m_telemetryStats->packageStats.totalPkgsToBeAcked++;
    if (metastatsOnly) {
        m_telemetryStats->packageStats.totalMetastatsOnlyPkgsToBeAcked++;
    }
}

void MetaStats::updateOnPackageSentSucceeded(EventPriority eventPriority, unsigned retryFailedTimes, unsigned durationMs, std::vector<unsigned> const& latencyToSendMs, bool metastatsOnly)
{
    unsigned const recordsSentCount = static_cast<unsigned>(latencyToSendMs.size());

    TelemetryStats::PackageStats& packageStats = m_telemetryStats->packageStats;
    packageStats.totalPkgsAcked++;
    packageStats.successPkgsAcked++;
    if (metastatsOnly) {
        packageStats.totalMetastatsOnlyPkgsAcked++;
    }
    m_telemetryStats->internalHttpStackRetriesStats.retriesCountDistribution[retryFailedTimes]++;

    //duration: distribution, max, min
    TelemetryStats::LatencyStats& rttStats = m_telemetryStats->rttStats;
    updateMap(rttStats.latencyDistribution, durationMs);
    rttStats.maxOfLatencyInMilliSecs = std::max<unsigned>(rttStats.maxOfLatencyInMilliSecs, durationMs);
    rttStats.minOfLatencyInMilliSecs = std::min<unsigned>(rttStats.minOfLatencyInMilliSecs, durationMs);

    TelemetryStats::LatencyStats& logToSuccessfulSendLatency = m_telemetryStats->logToSuccessfulSendLatencyPerPriority[eventPriority];
    for (unsigned latencyMs : latencyToSendMs) {
        updateMap(logToSuccessfulSendLatency.latencyDistribution, latencyMs);
        logToSuccessfulSendLatency.maxOfLatencyInMilliSecs = std::max(logToSuccessfulSendLatency.maxOfLatencyInMilliSecs, latencyMs);
        logToSuccessfulSendLatency.minOfLatencyInMilliSecs = std::min(logToSuccessfulSendLatency.minOfLatencyInMilliSecs, latencyMs);
    }

    TelemetryStats::RecordStats& recordStats = m_telemetryStats->recordStats;
    recordStats.sentCount += recordsSentCount;

    //TODO: fix it after ongoing stats implemented or discarded
    if (1) {
        recordStats.sentCountFromCurrentSession += recordsSentCount;
    } else {
        recordStats.sentCountFromPreviousSession += recordsSentCount;
    }

    //update per-priority record stats
    if (eventPriority >= 0) {
        TelemetryStats::RecordStats& recordStatsPerPriority = m_telemetryStats->recordStatsPerPriority[eventPriority];
        recordStatsPerPriority.sentCount += recordsSentCount;
        //TODO: fix it after ongoing stats implemented or discarded
        if (1) {
            recordStatsPerPriority.sentCountFromCurrentSession += recordsSentCount;
        } else {
            recordStatsPerPriority.sentCountFromPreviousSession += recordsSentCount;
        }
    }
}

void MetaStats::updateOnPackageFailed(int statusCode)
{
    TelemetryStats::PackageStats& packageStats = m_telemetryStats->packageStats;
    packageStats.totalPkgsAcked++;
    packageStats.dropPkgsAcked++;
    packageStats.dropPkgsPerHttpReturnCode[statusCode]++;
}

void MetaStats::updateOnPackageRetry(int statusCode, unsigned retryFailedTimes)
{
    TelemetryStats::PackageStats& packageStats = m_telemetryStats->packageStats;
    packageStats.totalPkgsAcked++;
    packageStats.retryPkgsAcked++;
    packageStats.retryPkgsPerHttpReturnCode[statusCode]++;

    m_telemetryStats->internalHttpStackRetriesStats.retriesCountDistribution[retryFailedTimes]++;
}

void MetaStats::updateOnRecordsDropped(EventDroppedReason reason, unsigned droppedCount)
{
    m_telemetryStats->recordStats.droppedCountReasonDistribution[reason] += droppedCount;
}

void MetaStats::updateOnStorageOpened(std::string const& type)
{
    m_telemetryStats->offlineStorageStats.storageFormat = type;
}

void MetaStats::updateOnStorageFailed(std::string const& reason)
{
    m_telemetryStats->offlineStorageStats.lastFailureReason = reason;
}


} ARIASDK_NS_END
