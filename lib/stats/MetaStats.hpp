#ifndef METASTATS_HPP
#define METASTATS_HPP

#include "pal/PAL.hpp"

#include "decorators/BaseDecorator.hpp"
#include "decorators/SemanticContextDecorator.hpp"

#include "api/IRuntimeConfig.hpp"

#include <Enums.hpp>
#include "bond/generated/AriaProtocol_types.hpp"
#include <memory>

namespace ARIASDK_NS_BEGIN {

    struct StatsConfig;
    struct TelemetryStats;

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
        // ARIASDK_LOG_DECL_COMPONENT_CLASS();

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
        std::unique_ptr<StatsConfig>    m_statsConfig;
        std::unique_ptr<TelemetryStats> m_telemetryStats;
        std::map<std::string, TelemetryStats*>  m_telemetryTenantStats;

        // FIXME
        // BaseDecorator                   m_baseDecorator;

    private:
        void privateSnapStatsToRecord(std::vector< ::AriaProtocol::Record>& records, RollUpKind rollupKind, TelemetryStats* telemetryStats);
        void privateClearStats(TelemetryStats* telemetryStats);
    };


} ARIASDK_NS_END

#endif