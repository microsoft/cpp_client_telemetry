//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "MetaStats.hpp"

#include <utils/StringUtils.hpp>

namespace MAT_NS_BEGIN {

    /// <summary>
    /// Converts RollUpKind enum value to string name.
    /// </summary>
    /// <param name="rollupKind">Kind of the rollup.</param>
    /// <returns></returns>
    static char const* RollUpKindToString(RollUpKind rollupKind)
    {
        switch (rollupKind) {
        case ACT_STATS_ROLLUP_KIND_START:
            return "start";
        case ACT_STATS_ROLLUP_KIND_STOP:
            return "stop";
        case ACT_STATS_ROLLUP_KIND_ONGOING:
            return "ongoing";
        default:
            return "unknown";
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
    /* TODO: consider rewriting this function */
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
        }
        else {
            for (unsigned int i = 1; i < totalSpot; ++i) {
                unsigned int key = (lastkey == 0) ? firstValue : (lastkey + increment);
                distribution[key] = 0;
                lastkey = key;
            }
        }
    }

    /// <summary>
    /// Update the occurrence within the corresponding group of Map distribution
    /// </summary>
    /// <param name="distribution">a distribution to be updated</param>
    /// <param name="value">unsigned int, sample value, must be in some group of the given distribution</param>
    /* TODO: consider rewriting this function */
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
            //LOG_WARN("value %u is less than distribution start (< %u)", value, it->first);
            it->second++;
        }
        else {
            (--it)->second++;
        }
    }

    /// <summary>
    /// A template function with typename T.
    /// The definition and implement must be in the same file.
    /// Only Clear values of each frequency distribution while keys are maintained.
    /// </summary>
    /// <param name="distribution">map<T, unsigned int></param>
    template<typename T, typename V>
    void clearMapValues(std::map<T, V>& distribution)
    {
        for (auto& item : distribution) {
            item.second = 0;
        }
    }

    template<typename T>
    static void insertNonZero(std::map<std::string, ::CsProtocol::Value>& target, std::string const& key, T const& value)
    {
        if (value != 0)
        {
            ::CsProtocol::Value temp;
            temp.stringValue = toString(value);
            target[key] = temp;
        }
    }

    /// <summary>
    /// Add count per each HTTP recode code to Record Extension Field
    /// </summary>
    /// <param name="record">telemetry::Record</param>
    /// <param name="distributionName">prefix of the key name in record extension map</param>
    /// <param name="distribution">map<unsigned int, unsigned int>, key is the http return code, value is the count</param>
    /* TODO: consider rewriting this function */
    static void addCountsPerHttpReturnCodeToRecordFields(::CsProtocol::Record& record, std::string const& prefix,
        uint_uint_dict_t const& countsPerHttpReturnCodeMap)
    {
        if (countsPerHttpReturnCodeMap.empty()) {
            return;
        }

        if (record.data.size() == 0)
        {
            ::CsProtocol::Data data;
            record.data.push_back(data);
        }
        for (auto const& item : countsPerHttpReturnCodeMap) {
            insertNonZero(record.data[0].properties, prefix + "_" + toString(item.first), item.second);
        }
    }

    MetaStats::MetaStats(IRuntimeConfig& config)
        :
        m_config(config),
        m_enableTenantStats(false)
    {
        m_telemetryStats.statsStartTimestamp = PAL::getUtcSystemTimeMs();
        resetStats(true);

        m_telemetryStats.offlineStorageEnabled = (static_cast<unsigned>(m_config[CFG_INT_CACHE_FILE_SIZE]) > 0);
        m_telemetryStats.resourceManagerEnabled = false;
        m_telemetryStats.ecsClientEnabled = false;

        m_enableTenantStats = static_cast<bool>(m_config[CFG_MAP_METASTATS_CONFIG]["split"]);
        m_sessionId = PAL::generateUuidString();
    }

    MetaStats::~MetaStats()
    {
    }

    /// <summary>
    /// Resets the stats.
    /// </summary>
    /// <param name="start">if set to <c>true</c> [start].</param>
    void MetaStats::resetStats(bool start)
    {
        LOG_TRACE("resetStats start=%u", (unsigned)start);

        auto resetTelemetryStats = [&](TelemetryStats& telemetryStats)
        {
            telemetryStats.Reset();
            telemetryStats.statsStartTimestamp = PAL::getUtcSystemTimeMs();
            telemetryStats.sessionId = m_sessionId;
            if (start) {
                telemetryStats.statsSequenceNum = 0;
                telemetryStats.sessionStartTimestamp = telemetryStats.statsStartTimestamp;
            }
            else {
                telemetryStats.statsSequenceNum++;
            }
        };

        // Cumulative
        resetTelemetryStats(m_telemetryStats);

        // Per-tenant
        if (m_enableTenantStats)
        {
            for (auto &kv : m_telemetryTenantStats)
            {
                resetTelemetryStats(kv.second);
            }
        }
    }

    /// <summary>
    /// Saves private snap stats to record.
    /// </summary>
    /// <param name="records">The records.</param>
    /// <param name="rollupKind">Kind of the rollup.</param>
    /// <param name="telemetryStats">The telemetry stats.</param>
    void MetaStats::snapStatsToRecord(std::vector< ::CsProtocol::Record>& records, RollUpKind rollupKind, TelemetryStats& telemetryStats)
    {
        ::CsProtocol::Record record;
        if (record.data.size() == 0)
        {
            ::CsProtocol::Data data;
            record.data.push_back(data);
        }
        record.baseType = "evt_stats";
        record.name = "evt_stats";

        std::map<std::string, ::CsProtocol::Value>& ext = record.data[0].properties;

        // Stats tenant ID
        std::string statTenantToken = m_config.GetMetaStatsTenantToken();
        record.iKey = "o:" + statTenantToken.substr(0, statTenantToken.find('-'));;

        // Session start time
        insertNonZero(ext, "sess_time", telemetryStats.sessionStartTimestamp);
        // Stats interval start
        insertNonZero(ext, "stat_time", telemetryStats.statsStartTimestamp);
        // Dtats interval end
        insertNonZero(ext, "snap_time", PAL::getUtcSystemTimeMs());

        // Stats kind: start|ongoing|stop
        ::CsProtocol::Value rollupKindValue;
        rollupKindValue.stringValue = RollUpKindToString(rollupKind);
        ext["kind"] = rollupKindValue;

        // Stats frequency
        insertNonZero(ext, "freq", m_config.GetMetaStatsSendIntervalSec());

        // Offline storage info
        if (telemetryStats.offlineStorageEnabled) {
            OfflineStorageStats& storageStats = telemetryStats.offlineStorageStats;
            ::CsProtocol::Value storageFormatValue;
            storageFormatValue.stringValue = storageStats.storageFormat;
            ext["off_type"] = storageFormatValue;
            if (!storageStats.lastFailureReason.empty())
            {
                ::CsProtocol::Value lastFailureReasonValue;
                lastFailureReasonValue.stringValue = storageStats.lastFailureReason;
                ext["off_fail"] = lastFailureReasonValue;
            }
            insertNonZero(ext, "off_size", storageStats.fileSizeInBytes);
        }

        // Package stats
        PackageStats& packageStats = telemetryStats.packageStats;
        insertNonZero(ext, "pkg_nak", packageStats.totalPkgsNotToBeAcked);
        insertNonZero(ext, "pkg_pnd", packageStats.totalPkgsToBeAcked);
        insertNonZero(ext, "pkg_ack", packageStats.totalPkgsAcked);
        insertNonZero(ext, "pkg_ok", packageStats.successPkgsAcked);
        insertNonZero(ext, "pkg_ret", packageStats.retryPkgsAcked);
        insertNonZero(ext, "pkg_drp", packageStats.dropPkgsAcked);
        addCountsPerHttpReturnCodeToRecordFields(record, "pkg_drop_HTTP", packageStats.dropPkgsPerHttpReturnCode);
        addCountsPerHttpReturnCodeToRecordFields(record, "pkg_retr_HTTP", packageStats.retryPkgsPerHttpReturnCode);
        insertNonZero(ext, "bytes", packageStats.totalBandwidthConsumedInBytes);

        // RTT stats
        if (packageStats.successPkgsAcked > 0) {
            LOG_TRACE("rttStats is added to record ext field");
            LatencyStats& rttStats = telemetryStats.rttStats;
            insertNonZero(ext, "rtt_max", rttStats.maxOfLatencyInMilliSecs);
            insertNonZero(ext, "rtt_min", rttStats.minOfLatencyInMilliSecs);
        }

        // Event stats
        RecordStats& recordStats = telemetryStats.recordStats;
        insertNonZero(ext, "evt_ban", recordStats.banned);
        insertNonZero(ext, "evt_rcv", recordStats.received);
        insertNonZero(ext, "evt_snt", recordStats.sent);
        insertNonZero(ext, "evt_rej", recordStats.rejected);
        insertNonZero(ext, "evt_drp", recordStats.dropped);

        // Reject reason stats
        for (const auto &kv : m_reject_reasons)
        {
            insertNonZero(ext, kv.second, recordStats.rejectedByReason[kv.first]);
        }

        // Drop reason stats
        insertNonZero(ext, "drp_ful", recordStats.overflown);
        insertNonZero(ext, "drp_io", recordStats.droppedByReason[DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED]);
        insertNonZero(ext, "drp_ret", recordStats.droppedByReason[DROPPED_REASON_RETRY_EXCEEDED]);
        addCountsPerHttpReturnCodeToRecordFields(record, "drp_HTTP", recordStats.droppedByHTTPCode);

        // Event size stats
        if (recordStats.received > 0) {
            LOG_TRACE("source stats and record size stats in recordStats"
                " are added to record ext field");
            insertNonZero(ext, "evt_bytes_max", recordStats.maxOfRecordSizeInBytes);
            insertNonZero(ext, "evt_bytes_min", recordStats.minOfRecordSizeInBytes);
            insertNonZero(ext, "evt_bytes", recordStats.totalRecordsSizeInBytes);
        }

        for (const auto &kv : m_latency_pfx)
        {
            const auto& lat = kv.first;
            const auto& pfx = kv.second;
            const RecordStats& r_stats = telemetryStats.recordStatsPerLatency[lat];
            insertNonZero(ext, pfx + "ban", r_stats.banned);
            insertNonZero(ext, pfx + "rcv", r_stats.received);
            insertNonZero(ext, pfx + "snt", r_stats.sent);
            insertNonZero(ext, pfx + "drp", r_stats.dropped);
            insertNonZero(ext, pfx + "dsk", r_stats.overflown);
            insertNonZero(ext, pfx + "rej", r_stats.rejected);
            insertNonZero(ext, pfx + "bytes", r_stats.totalRecordsSizeInBytes);
        }

        records.push_back(record);
    }

    /// <summary>
    /// Saves stats to record for given current RollUpKind
    /// </summary>
    /// <param name="records">The records.</param>
    /// <param name="rollupKind">Kind of the rollup.</param>
    void MetaStats::rollup(std::vector< ::CsProtocol::Record>& records, RollUpKind rollupKind)
    {
        LOG_TRACE("snapStatsToRecord");

        // Cumulative
        std::string statTenantToken = m_config.GetMetaStatsTenantToken();
        m_telemetryStats.tenantId = statTenantToken.substr(0, statTenantToken.find('-'));
        snapStatsToRecord(records, rollupKind, m_telemetryStats);

        // Per-tenant
        if (m_enableTenantStats)
        {
            for (auto &tenantStats : m_telemetryTenantStats)
            {
                snapStatsToRecord(records, rollupKind, tenantStats.second);
            }
        }
    }

    /// <summary>
    /// Clears the stats.
    /// </summary>
    void MetaStats::clearStats()
    {
        LOG_TRACE("clearStats");

        auto clearTelemetryStats = [&](TelemetryStats& telemetryStats)
        {
            telemetryStats.packageStats.dropPkgsPerHttpReturnCode.clear();
            telemetryStats.packageStats.retryPkgsPerHttpReturnCode.clear();
            telemetryStats.retriesCountDistribution.clear();

            RecordStats& recordStats = telemetryStats.recordStats;
            recordStats.droppedByHTTPCode.clear();

            OfflineStorageStats& storageStats = telemetryStats.offlineStorageStats;
            storageStats.saveSizeInKBytesDistribution.clear();
            storageStats.overwrittenSizeInKBytesDistribution.clear();
        };

        // Cumulative
        clearTelemetryStats(m_telemetryStats);

        // Per-tenant
        if (m_enableTenantStats)
        {
            for (auto& tenantStats : m_telemetryTenantStats)
            {
                clearTelemetryStats(tenantStats.second);
            }
        }
    }

    /// <summary>
    /// Determines whether stats data available.
    /// </summary>
    /// <returns>
    ///   <c>true</c> if [has stats data available]; otherwise, <c>false</c>.
    /// </returns>
    bool MetaStats::hasStatsDataAvailable() const
    {
        return (m_telemetryStats.recordStats.received > 0);
    }

    /// <summary>
    /// Generates the stats event.
    /// </summary>
    /// <param name="rollupKind">Kind of the rollup.</param>
    /// <returns></returns>
    std::vector< ::CsProtocol::Record> MetaStats::generateStatsEvent(RollUpKind rollupKind)
    {
        LOG_TRACE("generateStatsEvent");

        std::vector< ::CsProtocol::Record> records;

        if (hasStatsDataAvailable() || rollupKind != RollUpKind::ACT_STATS_ROLLUP_KIND_ONGOING) {
            rollup(records, rollupKind);
            resetStats(false);
        }

        if (rollupKind == ACT_STATS_ROLLUP_KIND_STOP) {
            clearStats();
        }

        return records;
    }

    /// <summary>
    /// Updates stats on incoming event.
    /// </summary>
    /// <param name="tenanttoken">The tenanttoken.</param>
    /// <param name="size">The size.</param>
    /// <param name="latency">The latency.</param>
    /// <param name="metastats">if set to <c>true</c> [metastats].</param>
    void MetaStats::updateOnEventIncoming(std::string const& tenanttoken, unsigned size, EventLatency latency, bool metastats)
    {
        auto updateRecordStats = [&](RecordStats& recordStats)
        {
            recordStats.received++;
            if (metastats)
            {
                recordStats.receivedStats++;
            }
            recordStats.maxOfRecordSizeInBytes = std::max<unsigned>(recordStats.maxOfRecordSizeInBytes, size);
            recordStats.minOfRecordSizeInBytes = std::min<unsigned>(recordStats.minOfRecordSizeInBytes, size);
            recordStats.totalRecordsSizeInBytes += size;
            if (latency >= 0) {
                RecordStats& recordStatsPerPriority = m_telemetryTenantStats[tenanttoken].recordStatsPerLatency[latency];
                recordStatsPerPriority.received++;
                recordStatsPerPriority.totalRecordsSizeInBytes += size;
            }
        };

        // Cumulative
        updateRecordStats(m_telemetryStats.recordStats);

        // Per-tenant
        if (m_enableTenantStats)
        {
            if (m_telemetryTenantStats[tenanttoken].tenantId.empty())
            {
                m_telemetryTenantStats[tenanttoken].tenantId = tenanttoken.substr(0, tenanttoken.find('-'));
            }
            updateRecordStats(m_telemetryTenantStats[tenanttoken].recordStats);
        }
    }

    /// <summary>
    /// Updates stats on post data success.
    /// </summary>
    /// <param name="postDataLength">Length of the post data.</param>
    /// <param name="metastatsOnly">if set to <c>true</c> [metastats only].</param>
    void MetaStats::updateOnPostData(unsigned postDataLength, bool metastatsOnly)
    {
        // Cumulative only
        m_telemetryStats.packageStats.totalBandwidthConsumedInBytes += postDataLength;
        m_telemetryStats.packageStats.totalPkgsToBeAcked++;
        if (metastatsOnly) {
            m_telemetryStats.packageStats.totalMetastatsOnlyPkgsToBeAcked++;
        }
    }

    /// <summary>
    /// Updates stats on successful package send.
    /// </summary>
    /// <param name="recordIdsAndTenantids">The record ids and tenantids.</param>
    /// <param name="eventLatency">The event latency.</param>
    /// <param name="retryFailedTimes">The retry failed times.</param>
    /// <param name="durationMs">The duration ms.</param>
    /// <param name="latencyToSendMs">The latency to send ms.</param>
    /// <param name="metastatsOnly">if set to <c>true</c> [metastats only].</param>
    void MetaStats::updateOnPackageSentSucceeded(std::map<std::string, std::string> const& recordIdsAndTenantids, EventLatency eventLatency, unsigned retryFailedTimes, unsigned durationMs, std::vector<unsigned> const& /*latencyToSendMs*/, bool metastatsOnly)
    {
        // Package summary stats
        PackageStats& packageStats = m_telemetryStats.packageStats;
        packageStats.totalPkgsAcked++;
        packageStats.successPkgsAcked++;
        if (metastatsOnly)
        {
            packageStats.totalMetastatsOnlyPkgsAcked++;
        }
        m_telemetryStats.retriesCountDistribution[retryFailedTimes]++;

        // RTT stats: record min and max HTTP post latency
        LatencyStats& rttStats = m_telemetryStats.rttStats;
        rttStats.maxOfLatencyInMilliSecs = std::max<unsigned>(rttStats.maxOfLatencyInMilliSecs, durationMs);
        rttStats.minOfLatencyInMilliSecs = std::min<unsigned>(rttStats.minOfLatencyInMilliSecs, durationMs);

        auto updatePackageSent = [&](TelemetryStats& stats)
        {
            RecordStats& recordStats = stats.recordStats;
            recordStats.sent++;
            // Update per-priority record stats
            if (eventLatency >= 0) {
                RecordStats& recordStatsPerPriority = stats.recordStatsPerLatency[eventLatency];
                recordStatsPerPriority.sent++;
            }
        };

        // Cumulative
        updatePackageSent(m_telemetryStats);

        // Per-tenant
        if (m_enableTenantStats)
        {
            for (const auto& entry : recordIdsAndTenantids)
            {
                updatePackageSent(m_telemetryTenantStats[entry.second]);
            }
        }

    }

    /// <summary>
    /// Update stats on package failure.
    /// </summary>
    /// <param name="statusCode">The status code.</param>
    void MetaStats::updateOnPackageFailed(int statusCode)
    {
        // Cumulative only
        PackageStats& packageStats = m_telemetryStats.packageStats;
        packageStats.totalPkgsAcked++;
        packageStats.dropPkgsAcked++;
        packageStats.dropPkgsPerHttpReturnCode[statusCode]++;
    }

    /// <summary>
    /// Update stats on package retry.
    /// </summary>
    /// <param name="statusCode">The status code.</param>
    /// <param name="retryFailedTimes">The retry failed times.</param>
    void MetaStats::updateOnPackageRetry(int statusCode, unsigned retryFailedTimes)
    {
        // Cumulative only
        PackageStats& packageStats = m_telemetryStats.packageStats;
        packageStats.totalPkgsAcked++;
        packageStats.retryPkgsAcked++;
        packageStats.retryPkgsPerHttpReturnCode[statusCode]++;
        m_telemetryStats.retriesCountDistribution[retryFailedTimes]++;
    }

    /// <summary>
    /// Update stats on records dropped.
    /// </summary>
    /// <param name="reason">The reason.</param>
    /// <param name="droppedCount">The dropped count.</param>
    void MetaStats::updateOnRecordsDropped(EventDroppedReason reason, std::map<std::string, size_t> const& droppedCount)
    {
        unsigned int overallCount = 0;
        for (const auto& dropcouttenant : droppedCount)
        {
            // Per-tenant
            if (m_enableTenantStats)
            {
                auto& temp = m_telemetryTenantStats[dropcouttenant.first];
                temp.recordStats.droppedByReason[reason] += static_cast<unsigned int>(dropcouttenant.second);
                temp.recordStats.dropped += static_cast<unsigned int>(dropcouttenant.second);
            }
            overallCount += static_cast<unsigned int>(dropcouttenant.second);
        }
        // Cumulative
        m_telemetryStats.recordStats.droppedByReason[reason] += overallCount;
        m_telemetryStats.recordStats.dropped += overallCount;
    }

    /// <summary>
    /// Update stats on records storage overflow.
    /// </summary>
    /// <param name="overflownCount">The overflown count.</param>
    void MetaStats::updateOnRecordsOverFlown(std::map<std::string, size_t> const& overflown)
    {
        unsigned int overallCount = 0;
        for (const auto& overflowntenant : overflown)
        {
            // Per-tenant
            if (m_enableTenantStats)
            {
                auto& temp = m_telemetryTenantStats[overflowntenant.first];
                temp.recordStats.overflown += static_cast<unsigned int>(overflowntenant.second);
            }
            overallCount += static_cast<unsigned int>(overflowntenant.second);
        }
        // Cumulative
        m_telemetryStats.recordStats.overflown += overallCount;
    }

    /// <summary>
    /// Update stats on records rejected.
    /// </summary>
    /// <param name="reason">The reason.</param>
    /// <param name="rejectedCount">The rejected count.</param>
    void MetaStats::updateOnRecordsRejected(EventRejectedReason reason, std::map<std::string, size_t> const& rejectedCount)
    {
        unsigned int overallCount = 0;
        for (const auto& rejecttenant : rejectedCount)
        {
            // Per-tenant
            if (m_enableTenantStats)
            {
                TelemetryStats& temp = m_telemetryTenantStats[rejecttenant.first];
                temp.recordStats.rejectedByReason[reason] += static_cast<unsigned int>(rejecttenant.second);
                temp.recordStats.rejected += static_cast<unsigned int>(rejecttenant.second);
            }
            overallCount += static_cast<unsigned int>(rejecttenant.second);
        }
        // Cumulative
        m_telemetryStats.recordStats.rejectedByReason[reason] += overallCount;
    }

    /// <summary>
    /// Update on storage open.
    /// </summary>
    /// <param name="type">The type.</param>
    void MetaStats::updateOnStorageOpened(std::string const& type)
    {
        m_telemetryStats.offlineStorageStats.storageFormat = type;
    }

    /// <summary>
    /// Update on storage open failed.
    /// </summary>
    /// <param name="reason">The reason.</param>
    void MetaStats::updateOnStorageFailed(std::string const& reason)
    {
        m_telemetryStats.offlineStorageStats.lastFailureReason = reason;
    }

    MATSDK_LOG_INST_COMPONENT_CLASS(RecordStats, "EventsSDK.RecordStats", "RecordStats");

} MAT_NS_END

