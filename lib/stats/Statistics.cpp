//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "pal/PAL.hpp"

#include "Statistics.hpp"
#include "ILogManager.hpp"
#include "utils/Utils.hpp"
#include <oacr.h>

namespace MAT_NS_BEGIN {

    Statistics::Statistics(ITelemetrySystem& telemetrySystem, ITaskDispatcher& taskDispatcher) :
        m_metaStats(telemetrySystem.getConfig()),
        m_iTelemetrySystem(telemetrySystem),
        m_taskDispatcher(taskDispatcher),
        m_config(telemetrySystem.getConfig()),
        m_logManager(telemetrySystem.getLogManager()),
        m_baseDecorator(m_logManager),
        m_semanticContextDecorator(m_logManager),
        m_isStarted(false)
    {
    }

    Statistics::~Statistics()
    {
    }

    inline void Statistics::scheduleSend()
    {
        if (!m_isStarted) {
            return;
        }

        unsigned int m_intervalMs = m_config.GetMetaStatsSendIntervalSec() * 1000;
        if (m_intervalMs != 0)
        {
            if (!m_isScheduled.exchange(true))
            {
                m_scheduledSend = PAL::scheduleTask(&m_taskDispatcher, m_intervalMs, this, &Statistics::send, ACT_STATS_ROLLUP_KIND_ONGOING);
                LOG_TRACE("Ongoing stats event generation scheduled in %u msec", m_intervalMs);
            }
        }
    }

     /// <summary>
    /// Sends stats event of a specified rollup kind.
    /// </summary>
    /// <param name="rollupKind">Kind of the rollup.</param>
    void Statistics::send(RollUpKind rollupKind)
    {
        m_isScheduled = false;

        unsigned int m_intervalMs = m_config.GetMetaStatsSendIntervalSec() * 1000;
        if (m_intervalMs == 0)
        {
            // cancel pending stats event if timer changed at runtime
            return;
        }

        std::vector< ::CsProtocol::Record> records;
        {
            LOCKGUARD(m_metaStats_mtx);
            records = m_metaStats.generateStatsEvent(rollupKind);
        }
        std::string tenantToken = m_config.GetMetaStatsTenantToken();

        for (auto& record : records)
        {
            bool result = true;
            result &= m_baseDecorator.decorate(record);
            // Allow stats to capture Part A common properties, but not the custom
            result &= m_semanticContextDecorator.decorate(record, true);
            if (result)
            {
                IncomingEventContext evt(PAL::generateUuidString(), tenantToken, EventLatency_Normal, EventPersistence_Normal, &record);
                m_iTelemetrySystem.sendEvent(&evt);
            }
            else
            {
                LOG_WARN("Failed to decorate stats event rollupKind=%u", rollupKind);
            }
        }
        m_statEventSentTime = PAL::getUtcSystemTimeMs();
    }

    bool Statistics::handleOnStart()
    {
        // synchronously send stats event on SDK start, but only if stats are enabled
        if (m_config.GetMetaStatsSendIntervalSec() * 1000 != 0)
        {
            send(ACT_STATS_ROLLUP_KIND_START);
        }

        m_isStarted = true;
        return true;
    }

    bool Statistics::handleOnStop()
    {
        m_isStarted = false;

        if (m_isScheduled.exchange(false)) {
            m_scheduledSend.Cancel();
        }

        // synchronously send stats event on SDK stop, but only if stats are enabled
        if (m_config.GetMetaStatsSendIntervalSec() * 1000 != 0)
        {
            send(ACT_STATS_ROLLUP_KIND_STOP);
        }
        return true;
    }

    void Statistics::OnDebugEvent(DebugEvent &evt)
    {
        m_logManager.DispatchEvent(evt);
    }

    bool Statistics::handleOnIncomingEventAccepted(IncomingEventContextPtr const& ctx)
    {
        bool metastats = (ctx->record.tenantToken == m_config.GetMetaStatsTenantToken());
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnEventIncoming(ctx->record.tenantToken, static_cast<unsigned>(ctx->record.blob.size()), ctx->record.latency, metastats);
        }
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
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnRecordsDropped(DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED, failedData);
        }
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
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnPostData(static_cast<unsigned>(ctx->httpRequest->GetSizeEstimate()), metastatsOnly);
        }
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
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnPackageSentSucceeded(ctx->recordIdsAndTenantIds, ctx->latency, ctx->maxRetryCountSeen, ctx->durationMs, latencyToSendMs, metastatsOnly);
        }
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnUploadRejected(EventsUploadContextPtr const& ctx)
    {
        unsigned status = (ctx->httpResponse)?ctx->httpResponse->GetStatusCode():0;
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnPackageFailed(status);
            std::map<std::string, size_t> countOnTenant;
            for (const auto& recordAndTenant : ctx->recordIdsAndTenantIds)
            {
                countOnTenant[recordAndTenant.second]++;
            }
            m_metaStats.updateOnRecordsRejected(REJECTED_REASON_SERVER_DECLINED, countOnTenant);
        }
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnUploadFailed(EventsUploadContextPtr const& ctx)
    {
        // TODO: [MG] - identify a special status code other than 0 for "request aborted" condition
        unsigned status = (ctx->httpResponse)?ctx->httpResponse->GetStatusCode():0;
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnPackageRetry(status, ctx->maxRetryCountSeen);
        }
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnStorageOpened(StorageNotificationContext const* ctx)
    {
        LOCKGUARD(m_metaStats_mtx);
        m_metaStats.updateOnStorageOpened(ctx->str);
        return true;
    }

    bool Statistics::handleOnStorageFailed(StorageNotificationContext const* ctx)
    {
        LOCKGUARD(m_metaStats_mtx);
        m_metaStats.updateOnStorageFailed(ctx->str);
        return true;
    }

    bool Statistics::handleOnStorageTrimmed(StorageNotificationContext const* ctx)
    {
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnRecordsOverFlown(ctx->countonTenant);
        }
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnStorageRecordsDropped(StorageNotificationContext const* ctx)
    {
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnRecordsDropped(DROPPED_REASON_RETRY_EXCEEDED, ctx->countonTenant);
        }
        scheduleSend();
        return true;
    }

    bool Statistics::handleOnStorageRecordsRejected(StorageNotificationContext const* ctx)
    {
        {
            LOCKGUARD(m_metaStats_mtx);
            m_metaStats.updateOnRecordsRejected(REJECTED_REASON_TENANT_KILLED, ctx->countonTenant);
        }
        scheduleSend();
        return true;
    }

} MAT_NS_END

