// Copyright (c) Microsoft. All rights reserved.

#include "pal/PAL.hpp"

#include "Statistics.hpp"
#include "ILogManager.hpp"
#include "utils/Utils.hpp"
#include <oacr.h>

namespace ARIASDK_NS_BEGIN {

    Statistics::Statistics(ITelemetrySystem& telemetrySystem) :
        m_iTelemetrySystem(telemetrySystem),
        m_logManager(telemetrySystem.getLogManager()),
        m_metaStats(telemetrySystem.getConfig()),
        m_config(telemetrySystem.getConfig()),
        m_isStarted(false),
        m_baseDecorator(m_logManager),
        m_semanticContextDecorator(m_logManager)
    {
    }

    Statistics::~Statistics()
    {
    }

    void Statistics::scheduleSend()
    {
        if (!m_isStarted) {
            return;
        }

        unsigned intervalMs = m_config.GetMetaStatsSendIntervalSec() * 1000;
        // FIXME: [MG] - review if this code works properly
#if 1
        if (!m_isScheduled && intervalMs != 0)
        {
            m_isScheduled = true;
            m_scheduledSend = PAL::scheduleOnWorkerThread(intervalMs, this, &Statistics::send, ACT_STATS_ROLLUP_KIND_ONGOING);
            LOG_TRACE("Ongoing stats event generation scheduled in %u msec", intervalMs);
        }
#else
        std::int64_t timedelta = PAL::getMonotonicTimeMs() - m_statEventSentTime;
        if (!m_isScheduled && intervalMs != 0 && timedelta > intervalMs)
        {
            m_isScheduled = true;
            send(ACT_STATS_ROLLUP_KIND_ONGOING);
            LOG_TRACE("Ongoing stats event generation scheduled in %u msec", intervalMs);
        }
#endif

    }

    void Statistics::send(RollUpKind rollupKind)
    {
        // TODO: [MG] - verify this codepath
        m_isScheduled = false;

        std::vector< ::AriaProtocol::Record> records = m_metaStats.generateStatsEvent(rollupKind);
        std::string tenantToken = m_config.GetMetaStatsTenantToken();

        for (auto& record : records)
        {
            bool result = true;
            result &= m_baseDecorator.decorate(record);
            result &= m_semanticContextDecorator.decorate(record);
            if (result)
            {
                IncomingEventContext evt(PAL::generateUuidString(), tenantToken, EventLatency_RealTime, EventPersistence_Critical, &record);
                eventGenerated(&evt);
                m_iTelemetrySystem.sendEvent(&evt);
            }
            else
            {
                LOG_WARN("Failed to decorate stats event rollupKind=%u", rollupKind);
            }
        }
        m_statEventSentTime = PAL::getMonotonicTimeMs();
    }

    bool Statistics::handleOnStart()
    {
        // TODO: [MG] - verify this codepath
        send(ACT_STATS_ROLLUP_KIND_START);

        m_isStarted = true;
        return true;
    }

    bool Statistics::handleOnStop()
    {
        m_isStarted = false;

        if (m_isScheduled) {
            m_scheduledSend.cancel();
            m_isScheduled = false;
        }

        // TODO: [MG] - verify this codepath
        send(ACT_STATS_ROLLUP_KIND_STOP);
        return true;
    }

    void Statistics::OnDebugEvent(DebugEvent &evt)
    {
        // TODO: refactor the rest of code here to go thru this method
        m_logManager.DispatchEvent(evt);
    }

    bool Statistics::handleOnIncomingEventAccepted(IncomingEventContextPtr const& ctx)
    {
        bool metastats = (ctx->record.tenantToken == m_config.GetMetaStatsTenantToken());
        m_metaStats.updateOnEventIncoming(ctx->record.tenantToken, static_cast<unsigned>(ctx->record.blob.size()), ctx->record.latency, metastats);
        scheduleSend();

        DebugEvent evt;
        evt.type = DebugEventType::EVT_ADDED;
        evt.param1 = 1;
        OnDebugEvent(evt);

        return true;
    }

    bool Statistics::handleOnIncomingEventFailed(IncomingEventContextPtr const& ctx)
    {
        UNREFERENCED_PARAMETER(ctx);
        std::map<std::string, size_t> failedData;
        failedData[ctx->record.tenantToken] = 1;
        m_metaStats.updateOnRecordsDropped(DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED, failedData);
        scheduleSend();

        DebugEvent evt;
        evt.type = DebugEventType::EVT_DROPPED;
        evt.param1 = 1;
        OnDebugEvent(evt);

        return true;
    }

    bool Statistics::handleOnUploadStarted(EventsUploadContextPtr const& ctx)
    {
        bool metastatsOnly = (ctx->packageIds.count(m_config.GetMetaStatsTenantToken()) == ctx->packageIds.size());
        m_metaStats.updateOnPostData(static_cast<unsigned>(ctx->httpRequest->GetSizeEstimate()), metastatsOnly);
        scheduleSend();

        DebugEvent evt;
        evt.type = DebugEventType::EVT_SENDING;
        evt.param1 = ctx->recordIdsAndTenantIds.size();
        OnDebugEvent(evt);

        return true;
    }

    bool Statistics::handleOnPackagingFailed(EventsUploadContextPtr const& ctx)
    {
        UNREFERENCED_PARAMETER(ctx);
        OACR_USE_PTR(this);
        return true;
    }

    bool Statistics::handleOnUploadSuccessful(EventsUploadContextPtr const& ctx)
    {
        int64_t now = PAL::getUtcSystemTimeMs();
        std::vector<unsigned> latencyToSendMs;
        latencyToSendMs.reserve(ctx->recordTimestamps.size());
        for (int64_t ts : ctx->recordTimestamps)
        {
            latencyToSendMs.push_back(static_cast<unsigned>(std::max<int64_t>(0, std::min<int64_t>(0xFFFFFFFFu, now - ts))));
        }

        bool metastatsOnly = (ctx->packageIds.count(m_config.GetMetaStatsTenantToken()) == ctx->packageIds.size());
        m_metaStats.updateOnPackageSentSucceeded(ctx->recordIdsAndTenantIds, ctx->latency, ctx->maxRetryCountSeen, ctx->durationMs, latencyToSendMs, metastatsOnly);
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnUploadRejected(EventsUploadContextPtr const& ctx)
    {
        unsigned status = (ctx->httpResponse)?ctx->httpResponse->GetStatusCode():0;
        m_metaStats.updateOnPackageFailed(status);

        std::map<std::string, size_t> countOnTenant;
        for (auto recordAndTenant : ctx->recordIdsAndTenantIds)
        {
            countOnTenant[recordAndTenant.second]++;
        }

        m_metaStats.updateOnRecordsRejected(REJECTED_REASON_SERVER_DECLINED, countOnTenant);

        scheduleSend();
        return true;
    }

    bool Statistics::handleOnUploadFailed(EventsUploadContextPtr const& ctx)
    {
        // TODO: [MG] - identify a special status code other than 0 for "request aborted" condition
        unsigned status = (ctx->httpResponse)?ctx->httpResponse->GetStatusCode():0;
        m_metaStats.updateOnPackageRetry(status, ctx->maxRetryCountSeen);
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnStorageOpened(StorageNotificationContext const* ctx)
    {
        m_metaStats.updateOnStorageOpened(ctx->str);
        return true;
    }

    bool Statistics::handleOnStorageFailed(StorageNotificationContext const* ctx)
    {
        m_metaStats.updateOnStorageFailed(ctx->str);
        return true;
    }

    bool Statistics::handleOnStorageTrimmed(StorageNotificationContext const* ctx)
    {
        m_metaStats.updateOnRecordsOverFlown(ctx->countonTenant);
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnStorageRecordsDropped(StorageNotificationContext const* ctx)
    {
        m_metaStats.updateOnRecordsDropped(DROPPED_REASON_RETRY_EXCEEDED, ctx->countonTenant);
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnStorageRecordsRejected(StorageNotificationContext const* ctx)
    {
        m_metaStats.updateOnRecordsRejected(REJECTED_REASON_TENANT_KILLED, ctx->countonTenant);
        scheduleSend();
        return true;
    }

#if 0
    void Statistics::createNewTenantIfNotFound(std::string token)
    {
        if (m_metaStats.find(token) == m_metaStats.end()) {
            MetaStats *newTenant = new MetaStats(m_runtimeConfig, m_globalContext);
            LOG_TRACE("!!! tenantToken      obj=%p", newTenant);
            m_metaStats[token] = newTenant;
            m_metaStats[token]->setTenantId(token);
        }
    }
#endif

} ARIASDK_NS_END
