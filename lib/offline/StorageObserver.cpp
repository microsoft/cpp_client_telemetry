//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "StorageObserver.hpp"

namespace MAT_NS_BEGIN {

    StorageObserver::StorageObserver(ITelemetrySystem& system, IOfflineStorage& offlineStorage)
        :
        m_system(system),
        m_offlineStorage(offlineStorage)
    {
    }

    StorageObserver::~StorageObserver()
    {
    }

    bool StorageObserver::handleStart()
    {
        m_offlineStorage.Initialize(*this);
        return true;
    }

    bool StorageObserver::handleStop()
    {
        m_offlineStorage.Shutdown();
        return true;
    }

    bool StorageObserver::handleStoreRecord(IncomingEventContextPtr const& ctx)
    {
        ctx->record.timestamp = PAL::getUtcSystemTimeMs();
        if (!m_offlineStorage.StoreRecord(ctx->record)) {
            // stats implementation must trigger a failure notification
            storeRecordFailed(ctx);
            return false;
        }
        return true;
    }

    void StorageObserver::handleRetrieveEvents(EventsUploadContextPtr const& ctx)
    {
        auto consumer = [&ctx, this](StorageRecord&& record) -> bool {
            bool wantMore = true;
            retrievedEvent(ctx, std::move(record), wantMore);
            return wantMore;
        };

        // TODO: [MG] - expose 120000 as a configuration parameter
        if (!m_offlineStorage.GetAndReserveRecords(consumer, 120000, ctx->requestedMinLatency, ctx->requestedMaxCount))
        {
            ctx->fromMemory = m_offlineStorage.IsLastReadFromMemory();
            retrievalFailed(ctx);
        }
        else
        {
            ctx->fromMemory = m_offlineStorage.IsLastReadFromMemory();
            retrievalFinished(ctx);
        }
    }

    bool StorageObserver::handleDeleteRecords(EventsUploadContextPtr const& ctx)
    {
        HttpHeaders headers;
        if (ctx->httpResponse)
        {
            headers = ctx->httpResponse->GetHeaders();
        }
        std::vector<StorageRecordId> recordIds;
        for (const auto& item : ctx->recordIdsAndTenantIds)
        {
            recordIds.push_back(item.first);
        }
        m_offlineStorage.DeleteRecords(recordIds, headers, ctx->fromMemory);
        return true;
    }

    bool StorageObserver::handleReleaseRecords(EventsUploadContextPtr const& ctx)
    {
        if (!ctx)
            return false;

        HttpHeaders headers;
        if (ctx->httpResponse)  // Error #2: UNADDRESSABLE ACCESS beyond heap bounds: reading 8 byte(s)
        {
            headers = ctx->httpResponse->GetHeaders();
        }
        std::vector<StorageRecordId> recordIds;
        for (const auto& item : ctx->recordIdsAndTenantIds)
        {
            recordIds.push_back(item.first);
        }
        m_offlineStorage.ReleaseRecords(recordIds, false, headers, ctx->fromMemory);
        return true;
    }

    bool StorageObserver::handleReleaseRecordsIncRetryCount(EventsUploadContextPtr const& ctx)
    {
        DispatchEvent(DebugEventType::EVT_SEND_RETRY);
        HttpHeaders headers;
        if (ctx->httpResponse)
        {
            headers = ctx->httpResponse->GetHeaders();
        }
        std::vector<StorageRecordId> recordIds;
        for (const auto& item : ctx->recordIdsAndTenantIds)
        {
            recordIds.push_back(item.first);
        }

        m_offlineStorage.ReleaseRecords(recordIds, true, headers, ctx->fromMemory);
        return true;
    }

    void StorageObserver::OnStorageOpened(std::string const& type)
    {
        StorageNotificationContext ctx;
        ctx.str = type;
        opened(&ctx);
    }

    void StorageObserver::OnStorageFailed(std::string const& reason)
    {
        StorageNotificationContext ctx;
        ctx.str = reason;
        failed(&ctx);
    }

    void StorageObserver::OnStorageOpenFailed(std::string const& reason)
    {
        StorageNotificationContext ctx;
        ctx.str = reason;
        failed(&ctx);
        {
            DebugEvent evt;
            evt.type = EVT_STORAGE_FAILED;
            evt.data = static_cast<void *>(const_cast<char *>(reason.c_str()));
            evt.size = reason.length();
            DispatchEvent(evt);
        }
    }


    void StorageObserver::OnStorageTrimmed(std::map<std::string, size_t> const& numRecords)
    {
        StorageNotificationContext ctx;

        size_t overallCount = 0;
        for (const auto& records : numRecords)
        {
            ctx.countonTenant[records.first] = records.second;
            overallCount += records.second;
        }
        trimmed(&ctx);

        {
            DebugEvent evt;
            evt.type = EVT_DROPPED;
            evt.param1 = overallCount;
            evt.size = overallCount;
            DispatchEvent(evt);
        }
    }

    void StorageObserver::OnStorageRecordsDropped(std::map<std::string, size_t> const& numRecords)
    {
        StorageNotificationContext ctx;
        size_t overallCount = 0;
        for (const auto& records : numRecords)
        {
            ctx.countonTenant[records.first] = records.second;
            overallCount += records.second;
        }
        recordsDropped(&ctx);

        {
            DebugEvent evt;
            evt.type = EVT_DROPPED;
            evt.param1 = overallCount;
            evt.size = overallCount;
            DispatchEvent(evt);
        }
    }

    void StorageObserver::OnStorageRecordsRejected(std::map<std::string, size_t> const& numRecords)
    {
        StorageNotificationContext ctx;
        size_t overallCount = 0;
        for (const auto& records : numRecords)
        {
            ctx.countonTenant[records.first] = records.second;
            overallCount += records.second;
        }
        recordsRejected(&ctx);

        {
            DebugEvent evt;
            evt.type = EVT_REJECTED;
            evt.param1 = overallCount;
            evt.size = overallCount;
            DispatchEvent(evt);
        }
    }

    void StorageObserver::OnStorageRecordsSaved(size_t numRecords)
    {
        DebugEvent evt;
        evt.type = EVT_CACHED;
        evt.param1 = numRecords;
        evt.size = 0; // We don't know the records size here
        DispatchEvent(evt);
    }

} MAT_NS_END

