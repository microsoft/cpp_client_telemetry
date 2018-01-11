// Copyright (c) Microsoft. All rights reserved.

#include "MetaStats.hpp"
#include <utils/Utils.hpp>
#include <algorithm>

namespace ARIASDK_NS_BEGIN {

/// <summary>
///  Maximum number of priorities supported.
///  This needs to be kept in sync with EventLatency in API
/// </summary>
static int const g_EventLatency_Count = 5;

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

	/// the utc timestamp of starting the SCT client.
	int64_t session_startup_time_in_millisec;

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
    } rttStats, logToSuccessfulSendLatencyPerLatency[g_EventLatency_Count];

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

        /// the number of overflown records
        unsigned int overflownCount;

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
    } recordStats, recordStatsPerLatency[g_EventLatency_Count];

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
static void insertNonZero(std::map<std::string, ::AriaProtocol::Value>& target, std::string const& key, T const& value)
{
    if (value != 0)
    {
        ::AriaProtocol::Value temp;
        temp.stringValue = toString(value);
        target[key] = temp;
    }
}

/// <summary>
/// Add A Map struture map to Record Extension Field
/// For example,
/// 1) range distribution
/// map = { 1:2, 2 : 3, 3 : 4 }
/// record.data.properties = { "name_0_1":"2", "name_1_2" : "3", "name_3_plus" : "4" }
/// 2) otherwise
/// record.data.properties = { "name_1":"2", "name_2" : "3", "name_3" : "4" }
/// </summary>
/// <param name="record">telemetry::Record</param>
/// <param name="distributionName">prefix of extension key name</param>
/// <param name="distribution">map<unsigned int, unsigned int></param>
/// <param name="range">indicate if the frequency distribution stored in map is based on multiple groups or multiple points</param>
static void addAggregatedMapToRecordFields(::AriaProtocol::CsEvent& record, std::string const& distributionName,
    uint_uint_dict_t const& distribution, bool range = true)
{
    if (distribution.empty()) {
        return;
    }
    if (record.data.size() == 0)
    {
        ::AriaProtocol::Data data;
        record.data.push_back(data);
    }

    std::map<std::string, ::AriaProtocol::Value>& ext = record.data[0].properties;
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

    ::AriaProtocol::Value temp;
    temp.stringValue = fieldValue;
    ext[distributionName] = temp;
}

/// <summary>
/// Add A Map struture to Record Extension Field
/// For example,
/// map= {"a":2, "b":3, "c":4}
/// record.data.properties = {"name_a":"2", "name_b":"3", "name_c":"4"}
/// </summary>
/// <param name="record">telemetry::Record</param>
/// <param name="distributionName">prefix of the key name in record extension map</param>
/// <param name="distribution">map<std::string, unsigned int>, key is the source of event</param>
static void addAggregatedMapToRecordFields(::AriaProtocol::CsEvent& record, std::string const& distributionName,
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
    ::AriaProtocol::Value temp;;
    temp.stringValue = fieldValue;
    if (record.data.size() == 0)
    {
        ::AriaProtocol::Data data;
        record.data.push_back(data);
    }
    record.data[0].properties[distributionName] = temp;
}

/// <summary>
/// Add count per each HTTP recode code to Record Extension Field
/// </summary>
/// <param name="record">telemetry::Record</param>
/// <param name="distributionName">prefix of the key name in record extension map</param>
/// <param name="distribution">map<unsigned int, unsigned int>, key is the http return code, value is the count</param>
static void addCountsPerHttpReturnCodeToRecordFields(::AriaProtocol::CsEvent& record, std::string const& prefix,
    uint_uint_dict_t const countsPerHttpReturnCodeMap)
{
    if (countsPerHttpReturnCodeMap.empty()) {
        return;
    }

    if (record.data.size() == 0)
    {
        ::AriaProtocol::Data data;
        record.data.push_back(data);
    }
    for (auto const& item : countsPerHttpReturnCodeMap) {
        insertNonZero(record.data[0].properties, prefix + "_" + toString(item.first), item.second);
    }
}

/// <summary>
/// Add rejected count by reason to Record Extension Field
/// </summary>
/// <param name="record">BondTypes::Record</param>
/// <param name="recordsRejectedCountReasonDistribution">count of rejected records by reason due to which records were rejected</param>
static void addRecordsPerRejectedReasonToRecordFields(::AriaProtocol::CsEvent& record, const unsigned int recordsRejectedCountByReasonDistribution[])
{
    if (record.data.size() == 0)
    {
        ::AriaProtocol::Data data;
        record.data.push_back(data);
    }

    std::map<std::string, ::AriaProtocol::Value>& extension = record.data[0].properties;

    insertNonZero(extension, "r_inv",  recordsRejectedCountByReasonDistribution[REJECTED_REASON_INVALID_CLIENT_MESSAGE_TYPE]);
    insertNonZero(extension, "r_inv",  recordsRejectedCountByReasonDistribution[REJECTED_REASON_REQUIRED_ARGUMENT_MISSING]);
    insertNonZero(extension, "r_inv",  recordsRejectedCountByReasonDistribution[REJECTED_REASON_EVENT_NAME_MISSING]);
    insertNonZero(extension, "r_inv",  recordsRejectedCountByReasonDistribution[REJECTED_REASON_VALIDATION_FAILED]);
    insertNonZero(extension, "r_inv",  recordsRejectedCountByReasonDistribution[REJECTED_REASON_OLD_RECORD_VERSION]);
    insertNonZero(extension, "r_exp",  recordsRejectedCountByReasonDistribution[REJECTED_REASON_EVENT_EXPIRED]);
    insertNonZero(extension, "r_403",  recordsRejectedCountByReasonDistribution[REJECTED_REASON_SERVER_DECLINED]);
    insertNonZero(extension, "r_kl",   recordsRejectedCountByReasonDistribution[REJECTED_REASON_TENANT_KILLED]);
    insertNonZero(extension, "r_size", recordsRejectedCountByReasonDistribution[REJECTED_REASON_EVENT_SIZE_LIMIT_EXCEEDED]);
}

//---

ARIASDK_LOG_INST_COMPONENT_CLASS(MetaStats, "EventsSDK.MetaStats", "Aria SDK statistics");

MetaStats::MetaStats(IRuntimeConfig const& runtimeConfig, ContextFieldsProvider const& parentContext)
  : m_runtimeConfig(runtimeConfig),
    m_statsConfig(new StatsConfig),
    m_telemetryStats(new TelemetryStats),
    m_baseDecorator(""),
    m_semanticContextDecorator(parentContext)
{
    m_telemetryStats->statsStartTimestamp = PAL::getUtcSystemTimeMs();
	m_telemetryStats->session_startup_time_in_millisec = m_telemetryStats->statsStartTimestamp;
    resetStats(true);
    //TODO: extend IRuntimeConfig to include these vars
    m_telemetryStats->offlineStorageEnabled = true;
    m_telemetryStats->resourceManagerEnabled = false;
    m_telemetryStats->ecsClientEnabled = false;   
}

MetaStats::~MetaStats()
{
    for (auto tenantStat : m_telemetryTenantStats)
    {
        delete tenantStat.second;
    }
    m_telemetryTenantStats.clear();
}

void MetaStats::resetStats(bool start)
{
    ARIASDK_LOG_DETAIL("resetStats");

    for (auto tenantStats : m_telemetryTenantStats)
    {
        TelemetryStats* telemetryStats = tenantStats.second;
        //clear packageStats
        TelemetryStats::PackageStats& packageStats = telemetryStats->packageStats;
        packageStats.Reset();

        //clear rttStats
        TelemetryStats::LatencyStats& rttStats = telemetryStats->rttStats;
        rttStats.Reset();

        // clear sdkToCollectorLatencyPerPriority stats
        for (auto& item : telemetryStats->logToSuccessfulSendLatencyPerLatency) {
            item.Reset();
        }

        //clear recordStats
        TelemetryStats::RecordStats& recordStats = telemetryStats->recordStats;
        recordStats.Reset();

        //clear recordStatsPerPriority
        for (auto& item : telemetryStats->recordStatsPerLatency) {
            item.Reset();
        }

        //clear offlineStorageStats
        TelemetryStats::OfflineStorageStats& storageStats = telemetryStats->offlineStorageStats;
        storageStats.Reset();

        telemetryStats->statsStartTimestamp = PAL::getUtcSystemTimeMs();

        if (start) {
            telemetryStats->statsSequenceNum = 0;
            telemetryStats->sessionStartTimestamp = telemetryStats->statsStartTimestamp;
            telemetryStats->sessionId = PAL::generateUuidString();
            ARIASDK_LOG_DETAIL("session start, session ID: %s", telemetryStats->sessionId.c_str());

            initDistributionKeys(m_statsConfig->rtt_first_duration_in_millisecs, m_statsConfig->rtt_next_factor,
                m_statsConfig->rtt_total_spots, rttStats.latencyDistribution);

            for (auto& item : telemetryStats->logToSuccessfulSendLatencyPerLatency) {
                initDistributionKeys(m_statsConfig->latency_first_duration_in_millisecs, m_statsConfig->latency_next_factor,
                    m_statsConfig->latency_total_spots, item.latencyDistribution);
            }

            initDistributionKeys(m_statsConfig->record_size_first_in_kb, m_statsConfig->record_size_next_factor,
                m_statsConfig->record_size_total_spots, recordStats.sizeInKBytesDistribution);

            if (telemetryStats->offlineStorageEnabled) {
                initDistributionKeys(m_statsConfig->storage_size_first_in_kb, m_statsConfig->storage_size_next_factor,
                    m_statsConfig->storage_size_total_spots, storageStats.saveSizeInKBytesDistribution);
                initDistributionKeys(m_statsConfig->storage_size_first_in_kb, m_statsConfig->storage_size_next_factor,
                    m_statsConfig->storage_size_total_spots, storageStats.overwrittenSizeInKBytesDistribution);
            }
        }
        else {
            ARIASDK_LOG_DETAIL("ongoing stats, session ID: %s", telemetryStats->sessionId.c_str());
            telemetryStats->statsSequenceNum += 1;
            packageStats.dropPkgsPerHttpReturnCode.clear();
            packageStats.retryPkgsPerHttpReturnCode.clear();

            TelemetryStats::InternalHttpStackRetriesStats& httpstackStats = telemetryStats->internalHttpStackRetriesStats;
            httpstackStats.retriesCountDistribution.clear();

            clearMapValues(rttStats.latencyDistribution);

            for (auto& item : telemetryStats->logToSuccessfulSendLatencyPerLatency) {
                clearMapValues(item.latencyDistribution);
            }

            clearMapValues(recordStats.semanticToRecordCountMap);
            clearMapValues(recordStats.semanticToExceptionCountMap);
            clearMapValues(recordStats.sizeInKBytesDistribution);

            recordStats.droppedCountPerHttpReturnCode.clear();

            if (telemetryStats->offlineStorageEnabled) {
                clearMapValues(storageStats.saveSizeInKBytesDistribution);
                clearMapValues(storageStats.overwrittenSizeInKBytesDistribution);
            }
        }
    }
}

void MetaStats::privateSnapStatsToRecord(std::vector< ::AriaProtocol::CsEvent>& records,
                                         ActRollUpKind rollupKind,
                                         TelemetryStats* telemetryStats)
{
    ::AriaProtocol::CsEvent record;
    record.baseType = "act_stats";

    ::AriaProtocol::Value temp;
    temp.stringValue = telemetryStats->sessionId;

    if (record.data.size() == 0)
    {
        ::AriaProtocol::Data data;
        record.data.push_back(data);
    }
    std::map<std::string, ::AriaProtocol::Value>& ext = record.data[0].properties;

    ext["act_stats_id"] = temp;

    //basic Fields
    //Add the tenantID (not the entire tenantToken) to the stats event

    std::string statTenantToken = m_runtimeConfig.GetMetaStatsTenantToken();
    record.iKey = "O:" + statTenantToken.substr(0, statTenantToken.find('-'));;
    record.name = "stats";

    // session fileds
    insertNonZero(ext, "session_start_timestamp", telemetryStats->sessionStartTimestamp);
    insertNonZero(ext, "stats_start_timestamp", telemetryStats->statsStartTimestamp);
    insertNonZero(ext, "session_startup_time_in_millisec", telemetryStats->session_startup_time_in_millisec);
    insertNonZero(ext, "stats_end_timestamp", PAL::getUtcSystemTimeMs());
    ::AriaProtocol::Value rollupKindValue;
    rollupKindValue.stringValue = ActRollUpKindToString(rollupKind);
    ext["stats_rollup_kind"] = rollupKindValue;
    insertNonZero(ext, "stats_send_frequency_secs", m_runtimeConfig.GetMetaStatsSendIntervalSec());

    if (telemetryStats->offlineStorageEnabled) {
        const TelemetryStats::OfflineStorageStats& storageStats = telemetryStats->offlineStorageStats;
        ::AriaProtocol::Value storageFormatValue;
        storageFormatValue.stringValue = storageStats.storageFormat;
        ext["offline_storage_format_type"] = storageFormatValue;
        if (!storageStats.lastFailureReason.empty())
        {
            ::AriaProtocol::Value lastFailureReasonValue;
            lastFailureReasonValue.stringValue = storageStats.lastFailureReason;
            ext["offline_storage_last_failure"] = lastFailureReasonValue;
        }
        insertNonZero(ext, "config_offline_storage_size_bytes", storageStats.fileSizeInBytes);
    }

    //package stats
    const TelemetryStats::PackageStats& packageStats = telemetryStats->packageStats;
    insertNonZero(ext, "requests_not_to_be_acked", packageStats.totalPkgsNotToBeAcked);
    insertNonZero(ext, "requests_to_be_acked", packageStats.totalPkgsToBeAcked);
    insertNonZero(ext, "requests_acked", packageStats.totalPkgsAcked);
    insertNonZero(ext, "requests_acked_succeeded", packageStats.successPkgsAcked);
    insertNonZero(ext, "requests_acked_retried", packageStats.retryPkgsAcked);
    insertNonZero(ext, "requests_acked_dropped", packageStats.dropPkgsAcked);
    addCountsPerHttpReturnCodeToRecordFields(record, "requests_acked_dropped_on_HTTP", packageStats.dropPkgsPerHttpReturnCode);
    addCountsPerHttpReturnCodeToRecordFields(record, "requests_acked_retried_on_HTTP", packageStats.retryPkgsPerHttpReturnCode);

    insertNonZero(ext, "rm_bw_bytes_consumed_count", packageStats.totalBandwidthConsumedInBytes);
      
    //InternalHttpStackRetriesStats
    if (packageStats.totalPkgsAcked > 0) {
        ARIASDK_LOG_DETAIL("httpstack_retries stats is added to record extension field");
        addAggregatedMapToRecordFields(record, "requests_fail_on_HTTP_retries_count_distribution",
            telemetryStats->internalHttpStackRetriesStats.retriesCountDistribution, false);
    }

    //RTTStats
    if (packageStats.successPkgsAcked > 0) {
        ARIASDK_LOG_DETAIL("rttStats is added to record ext field");
        const TelemetryStats::LatencyStats& rttStats = telemetryStats->rttStats;
        insertNonZero(ext, "rtt_millisec_max", rttStats.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "rtt_millisec_min", rttStats.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "rtt_millisec_distribution", rttStats.latencyDistribution);
    }

    //RecordStats
    const TelemetryStats::RecordStats& recordStats = telemetryStats->recordStats;

    insertNonZero(ext, "r_ban", recordStats.bannedCount);//records_banned_count

    insertNonZero(ext, "rcv", recordStats.receivedCount);// records_received_count

    insertNonZero(ext, "snt", recordStats.sentCount);//records_sent_count
    insertNonZero(ext, "records_sent_curr_session", recordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "records_sent_prev_session", recordStats.sentCountFromPreviousSession);

    insertNonZero(ext, "rej", recordStats.rejectedCount);//records_rejected_count
    addRecordsPerRejectedReasonToRecordFields(record, recordStats.rejectedCountReasonDistribution);

    insertNonZero(ext, "drp", recordStats.droppedCount);//records_dropped_count
    insertNonZero(ext, "d_disk_full", recordStats.overflownCount);
    insertNonZero(ext, "d_io_fail",   recordStats.droppedCountReasonDistribution[DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED]);    
    insertNonZero(ext, "d_retry_lmt", recordStats.droppedCountReasonDistribution[DROPPED_REASON_RETRY_EXCEEDED]);
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

    // Low priority RecordStats
    const TelemetryStats::RecordStats& lowPriorityrecordStats = telemetryStats->recordStatsPerLatency[EventLatency_Normal];
    insertNonZero(ext, "ln_r_ban", lowPriorityrecordStats.bannedCount);
    insertNonZero(ext, "ln_rcv",   lowPriorityrecordStats.receivedCount);
    insertNonZero(ext, "ln_snt",   lowPriorityrecordStats.sentCount);
    insertNonZero(ext, "ln_records_sent_count_current_session", lowPriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "ln_records_sent_count_previous_sessions", lowPriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "ln_drp",   lowPriorityrecordStats.droppedCount);
    insertNonZero(ext, "ln_d_disk_full", lowPriorityrecordStats.overflownCount);
    insertNonZero(ext, "ln_rej",   lowPriorityrecordStats.rejectedCount);
    if (lowPriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("Low priority source stats and record size stats in recordStats"
            " are added to record ext field");
        insertNonZero(ext, "ln_records_received_size_bytes", lowPriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (lowPriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyLow = telemetryStats->logToSuccessfulSendLatencyPerLatency[EventLatency_Normal];
        insertNonZero(ext, "ln_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyLow.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "n_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyLow.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "ln_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyLow.latencyDistribution);
    }

    // Normal priority RecordStats
    const TelemetryStats::RecordStats& normalPriorityrecordStats = telemetryStats->recordStatsPerLatency[EventLatency_CostDeferred];
    insertNonZero(ext, "ld_r_ban", normalPriorityrecordStats.bannedCount);
    insertNonZero(ext, "ld_rcv",   normalPriorityrecordStats.receivedCount);
    insertNonZero(ext, "ld_snt",   normalPriorityrecordStats.sentCount);
    insertNonZero(ext, "ld_records_sent_count_current_session", normalPriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "ld_records_sent_count_previous_sessions", normalPriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "ld_drp",   normalPriorityrecordStats.droppedCount);
    insertNonZero(ext, "ld_d_disk_full", normalPriorityrecordStats.overflownCount);
    insertNonZero(ext, "ld_rej",   normalPriorityrecordStats.rejectedCount);
    if (normalPriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("Normal priority source stats and record size stats in recordStats"
            " are added to record ext field");
        insertNonZero(ext, "ld_records_received_size_bytes", normalPriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (normalPriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyNormal = telemetryStats->logToSuccessfulSendLatencyPerLatency[EventLatency_CostDeferred];
        insertNonZero(ext, "ld_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyNormal.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "ld_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyNormal.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "ld_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyNormal.latencyDistribution);
    }

    // High priority RecordStats
    const TelemetryStats::RecordStats& highPriorityrecordStats = telemetryStats->recordStatsPerLatency[EventLatency_RealTime];
    insertNonZero(ext, "lr_r_ban", highPriorityrecordStats.bannedCount);
    insertNonZero(ext, "lr_rcv", highPriorityrecordStats.receivedCount);
    insertNonZero(ext, "lr_snt", highPriorityrecordStats.sentCount);
    insertNonZero(ext, "lr_records_sent_count_current_session", highPriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "lr_records_sent_count_previous_sessions", highPriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "lr_drp", highPriorityrecordStats.droppedCount);
    insertNonZero(ext, "ld_d_disk_full", normalPriorityrecordStats.overflownCount);
    insertNonZero(ext, "lr_rej", highPriorityrecordStats.rejectedCount);
    if (highPriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL("High priority source stats and record size stats in recordStats"
            " are added to record ext field");
        insertNonZero(ext, "lr_records_received_size_bytes", highPriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (highPriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyHigh = telemetryStats->logToSuccessfulSendLatencyPerLatency[EventLatency_RealTime];
        insertNonZero(ext, "lr_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyHigh.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "lr_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyHigh.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "lr_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyHigh.latencyDistribution);
    }

    // Immediate priority RecordStats
    const TelemetryStats::RecordStats& immediatePriorityrecordStats = telemetryStats->recordStatsPerLatency[EventLatency_Max];
    insertNonZero(ext, "lm_r_ban", immediatePriorityrecordStats.bannedCount);
    insertNonZero(ext, "lm_rcv",   immediatePriorityrecordStats.receivedCount);
    insertNonZero(ext, "lm_snt",   immediatePriorityrecordStats.sentCount);
    insertNonZero(ext, "lm_records_sent_count_current_session", immediatePriorityrecordStats.sentCountFromCurrentSession);
    insertNonZero(ext, "lm_records_sent_count_previous_sessions", immediatePriorityrecordStats.sentCountFromPreviousSession);
    insertNonZero(ext, "lm_drp",   immediatePriorityrecordStats.droppedCount);
    insertNonZero(ext, "lm_snt",   immediatePriorityrecordStats.rejectedCount);
    if (immediatePriorityrecordStats.receivedCount > 0) {
        ARIASDK_LOG_DETAIL(" high latency source stats and record size stats in recordStats are added to record ext field");
        insertNonZero(ext, "lm_records_received_size_bytes", immediatePriorityrecordStats.totalRecordsSizeInBytes);
    }
    if (immediatePriorityrecordStats.sentCount > 0) {
        const TelemetryStats::LatencyStats& logToSuccessfulSendLatencyImmediate = telemetryStats->logToSuccessfulSendLatencyPerLatency[EventLatency_Max];
        insertNonZero(ext, "lm_log_to_successful_send_latency_millisec_max", logToSuccessfulSendLatencyImmediate.maxOfLatencyInMilliSecs);
        insertNonZero(ext, "lm_log_to_successful_send_latency_millisec_min", logToSuccessfulSendLatencyImmediate.minOfLatencyInMilliSecs);
        addAggregatedMapToRecordFields(record, "lm_log_to_successful_send_latency_millisec_distribution", logToSuccessfulSendLatencyImmediate.latencyDistribution);
    }

    m_semanticContextDecorator.decorate(record);
    m_baseDecorator.decorate(record);

    records.push_back(record);
}

void MetaStats::snapStatsToRecord(std::vector< ::AriaProtocol::CsEvent>& records, ActRollUpKind rollupKind)
{
    ARIASDK_LOG_DETAIL("snapStatsToRecord");
      
    for (auto tenantStats : m_telemetryTenantStats)
    {
        TelemetryStats* telemetryStats = tenantStats.second;      
        privateSnapStatsToRecord(records, rollupKind, telemetryStats);       
    }

    if (ActRollUpKind::ACT_STATS_ROLLUP_KIND_ONGOING != rollupKind)
    {
        TelemetryStats* telemetryStats = m_telemetryStats.get();    
        std::string statTenantToken = m_runtimeConfig.GetMetaStatsTenantToken();
        telemetryStats->tenantId = statTenantToken.substr(0, statTenantToken.find('-'));
        privateSnapStatsToRecord(records, rollupKind, telemetryStats);
    }
}

void MetaStats::privateClearStats(TelemetryStats* telemetryStats)
{
    //clear distribution in packageStats
    telemetryStats->packageStats.dropPkgsPerHttpReturnCode.clear();
    telemetryStats->packageStats.retryPkgsPerHttpReturnCode.clear();

    //clear distribution in internalHttpStackRetriesStats
    telemetryStats->internalHttpStackRetriesStats.retriesCountDistribution.clear();

    //clear distribution in rttStats
    telemetryStats->rttStats.latencyDistribution.clear();

    for (auto& item : telemetryStats->logToSuccessfulSendLatencyPerLatency) {
        item.latencyDistribution.clear();
    }

    //clear disributions in recordStats
    TelemetryStats::RecordStats& recordStats = telemetryStats->recordStats;
    recordStats.sizeInKBytesDistribution.clear();
    recordStats.semanticToRecordCountMap.clear();
    recordStats.semanticToExceptionCountMap.clear();
    recordStats.droppedCountPerHttpReturnCode.clear();

    //clear disributions in offlineStorageStats
    TelemetryStats::OfflineStorageStats& storageStats = telemetryStats->offlineStorageStats;
    storageStats.saveSizeInKBytesDistribution.clear();
    storageStats.overwrittenSizeInKBytesDistribution.clear();
}

void MetaStats::clearStats()
{
    ARIASDK_LOG_DETAIL("clearStats");

    for (auto tenantStats : m_telemetryTenantStats)
    {
        TelemetryStats* telemetryStats = tenantStats.second;
        privateClearStats(telemetryStats);      
    }
    TelemetryStats* overallTelemetryStats = m_telemetryStats.get();
    privateClearStats(overallTelemetryStats);
}

bool MetaStats::hasStatsDataAvailable() const
{
    unsigned int rejectedCount = 0;
    unsigned int bannedCount = 0;
    unsigned int droppedCount = 0;
    unsigned int receivedCountnotStats = 0;
   
    for (auto tenantStats : m_telemetryTenantStats)
    {
        TelemetryStats* telemetryStats = tenantStats.second;
        rejectedCount += telemetryStats->recordStats.rejectedCount;
        bannedCount += telemetryStats->recordStats.bannedCount;
        droppedCount += telemetryStats->recordStats.droppedCount;
        receivedCountnotStats += telemetryStats->recordStats.receivedCount - telemetryStats->recordStats.receivedMetastatsCount;
    }
    return (rejectedCount > 0 ||   // not used
            bannedCount > 0 ||      // not used
            droppedCount > 0 ||     // not used
            receivedCountnotStats > 0 ||
           m_telemetryStats->packageStats.totalPkgsAcked > m_telemetryStats->packageStats.totalMetastatsOnlyPkgsAcked ||
           m_telemetryStats->packageStats.totalPkgsToBeAcked > m_telemetryStats->packageStats.totalMetastatsOnlyPkgsToBeAcked);
}

std::vector< ::AriaProtocol::CsEvent> MetaStats::generateStatsEvent(ActRollUpKind rollupKind)
{
    ARIASDK_LOG_DETAIL("generateStatsEvent");

    std::vector< ::AriaProtocol::CsEvent> records;

    if (hasStatsDataAvailable() || rollupKind != ActRollUpKind::ACT_STATS_ROLLUP_KIND_ONGOING) {
        snapStatsToRecord(records, rollupKind);

        resetStats(false);
    }

    if (rollupKind == ACT_STATS_ROLLUP_KIND_STOP) {
        clearStats();
    }

    return records;
}

void MetaStats::updateOnEventIncoming(std::string const& tenanttoken, unsigned size, EventLatency latency, bool metastats)
{
    if (!metastats)
    {
        if (m_telemetryTenantStats.end() == m_telemetryTenantStats.find(tenanttoken))
        {
            TelemetryStats* stats = new TelemetryStats();
            stats->tenantId = tenanttoken.substr(0, tenanttoken.find('-'));
            m_telemetryTenantStats[tenanttoken] = stats;            
        }

        TelemetryStats::RecordStats& recordStats = m_telemetryTenantStats[tenanttoken]->recordStats;
        recordStats.receivedCount++;

        updateMap(recordStats.sizeInKBytesDistribution, size / 1024);

        recordStats.maxOfRecordSizeInBytes = std::max<unsigned>(recordStats.maxOfRecordSizeInBytes, size);
        recordStats.minOfRecordSizeInBytes = std::min<unsigned>(recordStats.minOfRecordSizeInBytes, size);
        recordStats.totalRecordsSizeInBytes += size;

        if (latency >= 0) {
            TelemetryStats::RecordStats& recordStatsPerPriority = m_telemetryTenantStats[tenanttoken]->recordStatsPerLatency[latency];
            recordStatsPerPriority.receivedCount++;
            recordStatsPerPriority.totalRecordsSizeInBytes += size;
        }
    }

    //overall stats for all tennat tokens together
    TelemetryStats::RecordStats&  recordStats = m_telemetryStats->recordStats;
    recordStats.receivedCount++;
    if (metastats) {
        recordStats.receivedMetastatsCount++;
    }

    updateMap(recordStats.sizeInKBytesDistribution, size / 1024);

    recordStats.maxOfRecordSizeInBytes = std::max<unsigned>(recordStats.maxOfRecordSizeInBytes, size);
    recordStats.minOfRecordSizeInBytes = std::min<unsigned>(recordStats.minOfRecordSizeInBytes, size);
    recordStats.totalRecordsSizeInBytes += size;

    if (latency >= 0) {
        TelemetryStats::RecordStats& recordStatsPerPriority = m_telemetryStats->recordStatsPerLatency[latency];
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

void MetaStats::updateOnPackageSentSucceeded(std::map<std::string, std::string> const& recordIdsAndTenantids, EventLatency eventLatency, unsigned retryFailedTimes, unsigned durationMs, std::vector<unsigned> const& latencyToSendMs, bool metastatsOnly)
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


    //for overall stats
    {
        TelemetryStats::LatencyStats& logToSuccessfulSendLatency = m_telemetryStats->logToSuccessfulSendLatencyPerLatency[eventLatency];
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
        }
        else {
            recordStats.sentCountFromPreviousSession += recordsSentCount;
        }

        //update per-priority record stats
        if (eventLatency >= 0) {
            TelemetryStats::RecordStats& recordStatsPerPriority = m_telemetryStats->recordStatsPerLatency[eventLatency];
            recordStatsPerPriority.sentCount += recordsSentCount;
            //TODO: fix it after ongoing stats implemented or discarded
            if (1) {
                recordStatsPerPriority.sentCountFromCurrentSession += recordsSentCount;
            }
            else {
                recordStatsPerPriority.sentCountFromPreviousSession += recordsSentCount;
            }
        }
    }


    //per tenant stats
    std::string stattenantToken = m_runtimeConfig.GetMetaStatsTenantToken();
    for( auto entry : recordIdsAndTenantids)
    {
        std::string tenantToken = entry.second;
        
        if (m_telemetryTenantStats.end() == m_telemetryTenantStats.find(tenantToken))
        {
            continue;
        }
        TelemetryStats* telemetryStats = m_telemetryTenantStats[tenantToken];
        TelemetryStats::LatencyStats& logToSuccessfulSendLatency = telemetryStats->logToSuccessfulSendLatencyPerLatency[eventLatency];
        for (unsigned latencyMs : latencyToSendMs) {
            updateMap(logToSuccessfulSendLatency.latencyDistribution, latencyMs);
            logToSuccessfulSendLatency.maxOfLatencyInMilliSecs = std::max(logToSuccessfulSendLatency.maxOfLatencyInMilliSecs, latencyMs);
            logToSuccessfulSendLatency.minOfLatencyInMilliSecs = std::min(logToSuccessfulSendLatency.minOfLatencyInMilliSecs, latencyMs);
        }

        TelemetryStats::RecordStats& recordStats = telemetryStats->recordStats;
        recordStats.sentCount += 1;

        //TODO: fix it after ongoing stats implemented or discarded
        if (1) {
            recordStats.sentCountFromCurrentSession += 1;
        }
        else {
            recordStats.sentCountFromPreviousSession += 1;
        }

        //update per-priority record stats
        if (eventLatency >= 0) {
            TelemetryStats::RecordStats& recordStatsPerPriority = telemetryStats->recordStatsPerLatency[eventLatency];
            recordStatsPerPriority.sentCount += 1;
            //TODO: fix it after ongoing stats implemented or discarded
            if (1) {
                recordStatsPerPriority.sentCountFromCurrentSession += 1;
            }
            else {
                recordStatsPerPriority.sentCountFromPreviousSession += 1;
            }
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

void MetaStats::updateOnRecordsDropped(EventDroppedReason reason, std::map<std::string, size_t> const& droppedCount)
{
    int overallCount = 0;
    for (auto dropcouttenant : droppedCount)
    {
        if (m_telemetryTenantStats.end() == m_telemetryTenantStats.find(dropcouttenant.first))
        {//add a new telemetry
            m_telemetryTenantStats[dropcouttenant.first] = new TelemetryStats();
        }

        TelemetryStats* temp = m_telemetryTenantStats[dropcouttenant.first];
        temp->recordStats.droppedCountReasonDistribution[reason] += dropcouttenant.second;
        temp->recordStats.droppedCount += dropcouttenant.second;
        overallCount += dropcouttenant.second;
    }
    m_telemetryStats->recordStats.droppedCountReasonDistribution[reason] += overallCount;
    m_telemetryStats->recordStats.droppedCount += overallCount;
}

void MetaStats::updateOnRecordsOverFlown(std::map<std::string, size_t> const& overflownCount)
{
    int overallCount = 0;
    for (auto overflowntenant : overflownCount)
    {
        if (m_telemetryTenantStats.end() == m_telemetryTenantStats.find(overflowntenant.first))
        {//add a new telemetry
            m_telemetryTenantStats[overflowntenant.first] = new TelemetryStats();
        }

        TelemetryStats* temp = m_telemetryTenantStats[overflowntenant.first];
        temp->recordStats.overflownCount += overflowntenant.second;
        overallCount += overflowntenant.second;
    }
    m_telemetryStats->recordStats.overflownCount += overallCount;
}

void MetaStats::updateOnRecordsRejected(EventRejectedReason reason, std::map<std::string, size_t> const& rejectedCount)
{
    int overallCount = 0;
    for (auto rejecttenant : rejectedCount)
    {
        if (m_telemetryTenantStats.end() == m_telemetryTenantStats.find(rejecttenant.first))
        {//add a new telemetry
            m_telemetryTenantStats[rejecttenant.first] = new TelemetryStats();
        }

        TelemetryStats* temp = m_telemetryTenantStats[rejecttenant.first];
        temp->recordStats.droppedCountReasonDistribution[reason] += rejecttenant.second;
        temp->recordStats.rejectedCount += rejecttenant.second;
        overallCount += rejecttenant.second;
    }
    m_telemetryStats->recordStats.droppedCountReasonDistribution[reason] += overallCount;
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
