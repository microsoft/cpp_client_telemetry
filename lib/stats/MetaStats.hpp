// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "decorators/BaseDecorator.hpp"
#include "decorators/SemanticContextDecorator.hpp"
#include <Enums.hpp>
#include <IRuntimeConfig.hpp>
#include "bond/generated/AriaProtocol_types.hpp"
#include <memory>

namespace ARIASDK_NS_BEGIN {


struct StatsConfig;
struct TelemetryStats;

enum ActRollUpKind {
    ACT_STATS_ROLLUP_KIND_START,
    ACT_STATS_ROLLUP_KIND_STOP,
    ACT_STATS_ROLLUP_KIND_ONGOING
};

// Event dropped due to system limitation or failure
enum EventDroppedReason
{
    DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED,
    DROPPED_REASON_OFFLINE_STORAGE_OVERFLOW,
    DROPPED_REASON_RETRY_EXCEEDED,
    DROPPED_REASON_COUNT
};
static unsigned const gc_NumDroppedReasons = DROPPED_REASON_COUNT;


class MetaStats
{
  public:
    MetaStats(IRuntimeConfig const& runtimeConfig, ContextFieldsProvider const& parentContext);
    ~MetaStats();

    std::vector< ::AriaProtocol::CsEvent> generateStatsEvent(ActRollUpKind rollupKind);

    void updateOnEventIncoming(std::string const& tenanttoken, unsigned size, EventLatency latency, bool metastats);
    void updateOnPostData(unsigned postDataLength, bool metastatsOnly);
    void updateOnPackageSentSucceeded( std::map<std::string, std::string> const& recordIdsAndTenantids, EventLatency eventLatency, unsigned retryFailedTimes, unsigned durationMs, std::vector<unsigned> const& latencyToSendMs, bool metastatsOnly);
    void updateOnPackageFailed(int statusCode);
    void updateOnPackageRetry(int statusCode, unsigned retryFailedTimes);
    void updateOnRecordsDropped(EventDroppedReason reason, std::map<std::string, size_t> const& droppedCount);
    void updateOnRecordsOverFlown(std::map<std::string, size_t> const& overflownCount);
    void updateOnRecordsRejected(EventRejectedReason reason, std::map<std::string, size_t> const& rejectedCount);
    void updateOnStorageOpened(std::string const& type);
    void updateOnStorageFailed(std::string const& reason);

  protected:
    ARIASDK_LOG_DECL_COMPONENT_CLASS();

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
    void snapStatsToRecord(std::vector< ::AriaProtocol::CsEvent>& records, ActRollUpKind rollupKind);

  protected:
    IRuntimeConfig const&           m_runtimeConfig;
    std::unique_ptr<StatsConfig>    m_statsConfig;
    std::unique_ptr<TelemetryStats> m_telemetryStats;
    std::map<std::string,TelemetryStats*>  m_telemetryTenantStats;
    BaseDecorator                   m_baseDecorator;
    SemanticContextDecorator        m_semanticContextDecorator;
private:
    void privateSnapStatsToRecord(std::vector< ::AriaProtocol::CsEvent>& records, ActRollUpKind rollupKind, TelemetryStats* telemetryStats);
    void privateClearStats(TelemetryStats* telemetryStats);
};


} ARIASDK_NS_END
