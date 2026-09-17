//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "OfflineStorageHandler.hpp"
#include "OfflineStorageFactory.hpp"

#include "offline/MemoryStorage.hpp"
#include "offline/StorageRecordValidation.hpp"

#include "ILogManager.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
#include <cstdio>
#include <exception>
#include <limits>
#include <numeric>
#include <set>
#include <stdexcept>

namespace MAT_NS_BEGIN {

    namespace
    {
        // Keep each persistence transaction bounded so a large in-memory backlog
        // cannot monopolize memory or database locks.
        constexpr unsigned MAX_RECORDS_PER_STORAGE_BATCH = 2000;
    }


    MATSDK_LOG_INST_COMPONENT_CLASS(OfflineStorageHandler, "EventsSDK.StorageHandler", "Events telemetry client - OfflineStorageHandler class")

    OfflineStorageHandler::OfflineStorageHandler(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher) :
        OfflineStorageHandler(logManager, runtimeConfig, taskDispatcher, OfflineStorageFactory::GetDefaultProvider())
    {
    }

    OfflineStorageHandler::OfflineStorageHandler(ILogManager& logManager, IRuntimeConfig& runtimeConfig,
        ITaskDispatcher& taskDispatcher, std::shared_ptr<IOfflineStorageProvider> storageProvider) :
        m_observer(nullptr),
        m_logManager(logManager),
        m_config(runtimeConfig),
        m_taskDispatcher(taskDispatcher),
        m_storageProvider(std::move(storageProvider)),
        m_killSwitchManager(),
        m_clockSkewManager(),
        m_flushPending(false),
        m_offlineStorageMemory(nullptr),
        m_offlineStorageDisk(nullptr),
        m_readFromMemory(false),
        m_lastReadCount(0),
        m_shutdownStarted(false),
        m_memoryDbSize(0),
        m_queryDbSize(0),
        m_cacheMemorySizeLimitInBytes(0),
        m_isStorageFullNotificationSend(false)
    {
        if (!m_storageProvider)
        {
            throw std::invalid_argument("OfflineStorageHandler requires a storage provider");
        }

        // TODO: [MG] - OfflineStorage_SQLite.cpp is performing similar checks
        uint32_t percentage = m_config[CFG_INT_RAMCACHE_FULL_PCT];
        uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];
        if (percentage > 0 && percentage <= 100)
        {
            m_memoryDbSizeNotificationLimit = (percentage * cacheMemorySizeLimitInBytes) / 100;
        }
        else
        {
            // In case if user has specified bad percentage, we stick to 75%
            m_memoryDbSizeNotificationLimit = (DB_FULL_NOTIFICATION_DEFAULT_PERCENTAGE * cacheMemorySizeLimitInBytes) / 100;
        }
    }

    /// <summary>
    /// RAII guard around ILogManager::StartActivity()/EndActivity(). Flush()
    /// used to pair these manually (StartActivity() at the top, EndActivity()
    /// on the last line), so an exception thrown by disk I/O or by
    /// IOfflineStorageObserver::OnStorageRecordsSaved() partway through would
    /// skip EndActivity() and permanently leak the pause-activity count --
    /// deadlocking every later FlushAndTeardown()'s WaitPause(). This guard
    /// guarantees EndActivity() runs on every exit path, matching the existing
    /// safe pattern used by PauseGuard (TransmissionPolicyManager.cpp) and
    /// ActiveLoggerCall (Logger.cpp).
    /// </summary>
    class ActivityGuard
    {
       public:
        explicit ActivityGuard(ILogManager& logManager) :
            m_logManager(logManager),
            m_active(logManager.StartActivity()),
            m_allowInactive(false)
        {
        }

        ActivityGuard(ILogManager& logManager, bool allowInactive) :
            m_logManager(logManager),
            m_active(logManager.StartActivity()),
            m_allowInactive(allowInactive)
        {
        }

        ~ActivityGuard() noexcept
        {
            if (m_active)
            {
                try
                {
                    m_logManager.EndActivity();
                }
                catch (const std::exception& e)
                {
                    std::fprintf(stderr, "Failed to end telemetry activity: %s\n", e.what());
                }
                catch (...)
                {
                    std::fputs("Failed to end telemetry activity\n", stderr);
                }
            }
        }

        ActivityGuard(ActivityGuard const&) = delete;
        ActivityGuard& operator=(ActivityGuard const&) = delete;

        bool IsActive() const noexcept { return m_active || m_allowInactive; }

       private:
        ILogManager& m_logManager;
        bool m_active;
        bool m_allowInactive;
    };

    bool OfflineStorageHandler::isKilled(StorageRecord const& record)
    {
        return (
            /* fast   */ m_killSwitchManager.isActive() &&
            /* slower */ m_killSwitchManager.isTokenBlocked(record.tenantToken));
    }

    void OfflineStorageHandler::WaitForFlush()
    {
        {
            LOCKGUARD(m_flushLock);
            if (!m_flushPending)
                return;
        }
        LOG_INFO("Waiting for pending Flush (%p) to complete...",
            static_cast<void*>(m_flushHandle.GetTask()));
        m_flushComplete.wait();
    }

    OfflineStorageHandler::~OfflineStorageHandler()
    {
        WaitForFlush();
        if (nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory.reset();
        }
        if (nullptr != m_offlineStorageDisk)
        {
            m_offlineStorageDisk.reset();
        }
    }

    void OfflineStorageHandler::Initialize(IOfflineStorageObserver& observer)
    {
        m_observer = &observer;
        m_cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];

        m_offlineStorageDisk = m_storageProvider->CreateDiskStorage(m_logManager, m_config);
        if (m_offlineStorageDisk)
        {
            m_offlineStorageDisk->Initialize(*this);
        }

        // TODO: [MG] - consider passing m_offlineStorageDisk to m_offlineStorageMemory,
        // so that the Flush() op on memory storage leads to saving unflushed events to
        // disk.
        if (m_cacheMemorySizeLimitInBytes > 0)
        {
            m_offlineStorageMemory = m_storageProvider->CreateMemoryStorage(m_logManager, m_config);
            m_offlineStorageMemory->Initialize(*this);
        }

        m_shutdownStarted = false;
        LOG_TRACE("Initializing offline storage handler");
    }

    void OfflineStorageHandler::Shutdown()
    {
        LOG_TRACE("Shutting down offline storage handler");
        m_shutdownStarted = true;
        WaitForFlush();
        if (nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->ReleaseAllRecords();
            Flush();
            m_offlineStorageMemory->Shutdown();
        }
        if (nullptr != m_offlineStorageDisk)
        {
            m_offlineStorageDisk->Shutdown();
        }
    }

    /// <summary>
    /// Get estimated DB size
    /// </summary>
    /// <returns>
    /// Size of memory + disk storage
    /// </returns>
    /// <remarks>
    /// Value may change at runtime, so it's only approximate value.
    /// </remarks>
    size_t OfflineStorageHandler::GetSize()
    {
        size_t size = 0;
        if (m_offlineStorageMemory != nullptr)
            size += m_offlineStorageMemory->GetSize();
        if (m_offlineStorageDisk != nullptr)
            size += m_offlineStorageDisk->GetSize();
        return size;
    }

    size_t OfflineStorageHandler::GetRecordCount(EventLatency latency) const
    {
        size_t count = 0;
        if (m_offlineStorageMemory != nullptr)
            count += m_offlineStorageMemory->GetRecordCount(latency);
        if (m_offlineStorageDisk != nullptr)
            count += m_offlineStorageDisk->GetRecordCount(latency);
        return count;
    }

    size_t OfflineStorageHandler::GetRemainingRecordCountForShutdown() const
    {
        size_t count = 0;
        if (m_offlineStorageMemory != nullptr)
            count += m_offlineStorageMemory->GetRemainingRecordCountForShutdown();
        if (m_offlineStorageDisk != nullptr)
            count += m_offlineStorageDisk->GetRemainingRecordCountForShutdown();
        return count;
    }

    void OfflineStorageHandler::Flush()
    {
        // Shutdown has already paused normal logging, but its synchronous final
        // flush must still persist the in-memory records before storage closes.
        ActivityGuard activityGuard(m_logManager, m_shutdownStarted);
        if (!activityGuard.IsActive()) {
            // The LogManager is shutting down, so the flush cannot run. Still
            // signal completion and clear the pending flag so a concurrent
            // WaitForFlush() (e.g. during teardown) does not block forever
            // waiting for m_flushComplete.
            LOCKGUARD(m_flushLock);
            m_flushHandle.Cancel();
            m_flushComplete.post();
            m_flushPending = false;
            return;
        }
        std::vector<StorageRecord> recordsToRecover;
        try
        {
            // Flush could be executed from context of worker thread, as well as from TPM and
            // after HTTP callback. Make sure it is atomic / thread-safe.
            LOCKGUARD(m_flushLock);

            // If item isn't scheduled yet, it gets canceled, so that we don't do two flushes.
            // If we are running that item right now (our thread), then nothing happens other
            // than the handle reporting nullptr once that task finishes.
            m_flushHandle.Cancel();

            size_t dbSizeBeforeFlush = (m_offlineStorageMemory != nullptr) ? m_offlineStorageMemory->GetSize() : 0;
            if ((m_offlineStorageMemory) && (dbSizeBeforeFlush > 0) && (m_offlineStorageDisk))
            {
                size_t totalSaved = 0;
                if (IsBatchedStorageFlushEnabled())
                {
                    // Drain only the records present when this flush started so
                    // producers cannot keep the flush alive indefinitely.
                    size_t recordsRemaining = m_offlineStorageMemory->GetRecordCount();
                    while (recordsRemaining > 0)
                    {
                        recordsToRecover = m_offlineStorageMemory->GetRecords(
                            false, EventLatency_Unspecified, MAX_RECORDS_PER_STORAGE_BATCH);
                        if (recordsToRecover.empty())
                        {
                            break;
                        }

                        const size_t drainedBatchSize = recordsToRecover.size();
                        recordsRemaining -= std::min(recordsRemaining, drainedBatchSize);
                        const size_t batchSaved = m_offlineStorageDisk->StoreRecords(recordsToRecover);
                        // StoreRecords() removes permanently-invalid records before
                        // returning, so compare against the remaining valid records.
                        const size_t validBatchSize = recordsToRecover.size();
                        if (batchSaved != validBatchSize)
                        {
                            LOG_WARN("Flush: disk store failed for the batch of %zu records; returning it to the queue for retry",
                                validBatchSize);
                            ReturnRecordsToMemory(recordsToRecover);
                            recordsToRecover.clear();
                            break;
                        }

                        totalSaved += batchSaved;
                        recordsToRecover.clear();
                    }
                }
                else
                {
                    // Preserve the legacy per-record path and its unlimited drain.
                    recordsToRecover = m_offlineStorageMemory->GetRecords(
                        false, EventLatency_Unspecified);
                    totalSaved = StoreRecordsIndividually(recordsToRecover);
                }

                // Persistence and retry handling are complete; a later exception
                // must not requeue records that were already committed.
                recordsToRecover.clear();

                if (m_offlineStorageMemory->GetSize() > dbSizeBeforeFlush)
                {
                    // We managed to accumulate as much data as we had before the flush,
                    // means we cannot keep up flushing at the same speed as incoming
                    // obviously because the disk is slower than ram.
                    LOG_WARN("Data is arriving too fast!");
                }
                OnStorageRecordsSaved(totalSaved);
            }

            // Checkpoint DB
            if (m_offlineStorageDisk && m_config.HasConfig(CFG_BOOL_CHECKPOINT_DB_ON_FLUSH) && m_config[CFG_BOOL_CHECKPOINT_DB_ON_FLUSH])
            {
                m_offlineStorageDisk->Flush();
            }

            m_isStorageFullNotificationSend = false;
            m_flushComplete.post();
            m_flushPending = false;
        }
        catch (...)
        {
            std::exception_ptr failure = std::current_exception();
            try
            {
                if (m_offlineStorageMemory && !recordsToRecover.empty())
                {
                    ReturnRecordsToMemory(recordsToRecover);
                }
            }
            catch (const std::exception& e)
            {
                std::fprintf(stderr, "Failed to recover records after flush failure: %s\n", e.what());
            }
            catch (...)
            {
                std::fputs("Failed to recover records after flush failure\n", stderr);
            }
            LOCKGUARD(m_flushLock);
            m_flushComplete.post();
            m_flushPending = false;
            std::rethrow_exception(failure);
        }
    }

    bool OfflineStorageHandler::StoreRecord(StorageRecord const& record)
    {
        // Don't discard on shutdown because the kill-switch may be temporary.
        // Attempt to upload after restart.
        if ((!m_shutdownStarted) && isKilled(record))
        {
            // Discard unwanted records associated with killed tenant, reporting events as dropped
            return false;
        }

        // Cache size limit is per-instance config computed once in Initialize();
        // it must NOT be a function-local static, which would share the first
        // LogManager's value with every other LogManager instance.
        uint32_t cacheMemorySizeLimitInBytes = m_cacheMemorySizeLimitInBytes;

        if (nullptr != m_offlineStorageMemory && !m_shutdownStarted)
        {
            auto memDbSize = m_offlineStorageMemory->GetSize();
            {
                // During flush, this will block on a mutex while records
                // are selected and removed from the cache (but will
                // not block for the subsequent handoff to persistent
                // storage)
                if (!m_offlineStorageMemory->StoreRecord(record))
                {
                    if (record.latency == EventLatency_Off)
                    {
                        // MemoryStorage intentionally returns false for latency-off
                        // records to mean "drop without storing", not "storage
                        // failed". Keep the handler's false return reserved for
                        // genuine storage failures so StorageObserver does not
                        // misclassify this normal drop as a persistence error.
                        return true;
                    }
                    LOG_ERROR("Failed to store event %s:%s in memory queue",
                        tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
                    return false;
                }
            }

            // Perform periodic flush to disk
            if (memDbSize > cacheMemorySizeLimitInBytes)
            {
                if (m_flushLock.try_lock())
                {
                    if (!m_flushPending)
                    {
                        m_flushPending = true;
                        m_flushComplete.Reset();
                        m_flushHandle = PAL::scheduleTask(&m_taskDispatcher, 0, this, &OfflineStorageHandler::Flush);
                        if (m_flushHandle.GetTask() == nullptr)
                        {
                            // The dispatcher may drop a task synchronously during
                            // shutdown. Do not leave WaitForFlush blocked forever.
                            m_flushPending = false;
                            m_flushComplete.post();
                        }
                        LOG_INFO("Requested Flush (%p)",
                            static_cast<void*>(m_flushHandle.GetTask()));
                    }
                    m_flushLock.unlock();
                }
            }
        }
        else
        {
            if (m_offlineStorageDisk != nullptr)
            {
                if (record.persistence != EventPersistence::EventPersistence_DoNotStoreOnDisk)
                {
                    // Propagate a synchronous disk write failure to the caller so a
                    // failed store is not counted as successfully persisted.
                    return m_offlineStorageDisk->StoreRecord(record);
                }
            }
        }

        return true;
    }

    bool OfflineStorageHandler::IsBatchedStorageFlushEnabled()
    {
        const bool batchingConfigured =
            !m_config.HasConfig(CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH) ||
            m_config[CFG_BOOL_ENABLE_BATCHED_STORAGE_FLUSH];
        const bool usingCustomStorage =
            m_logManager.GetLogConfiguration().GetModule(CFG_MODULE_OFFLINE_STORAGE) != nullptr;
        return batchingConfigured && !usingCustomStorage;
    }

    void OfflineStorageHandler::ReportInvalidDiskRecord(StorageRecord const& record)
    {
        (void)record;
        LOG_ERROR("Flush: dropping event %s:%s: Invalid parameters",
            tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
        OnStorageFailed("Invalid parameters");
    }

    size_t OfflineStorageHandler::StoreRecordsIndividually(std::vector<StorageRecord>& records)
    {
        size_t totalSaved = 0;
        std::vector<StorageRecord> recordsToRetry;
        size_t nextRecord = 0;

        try
        {
            for (; nextRecord < records.size(); ++nextRecord)
            {
                auto const& record = records[nextRecord];
                if (!IsValidDiskStorageRecord(record))
                {
                    ReportInvalidDiskRecord(record);
                    continue;
                }

                if (m_offlineStorageDisk->StoreRecord(record))
                {
                    ++totalSaved;
                    continue;
                }

                for (size_t retryIndex = nextRecord; retryIndex < records.size(); ++retryIndex)
                {
                    auto const& retryRecord = records[retryIndex];
                    if (IsValidDiskStorageRecord(retryRecord))
                    {
                        recordsToRetry.push_back(retryRecord);
                    }
                    else
                    {
                        ReportInvalidDiskRecord(retryRecord);
                    }
                }
                break;
            }
        }
        catch (...)
        {
            recordsToRetry.clear();
            for (size_t retryIndex = nextRecord; retryIndex < records.size(); ++retryIndex)
            {
                if (IsValidDiskStorageRecord(records[retryIndex]))
                {
                    recordsToRetry.push_back(records[retryIndex]);
                }
            }
            records.clear();
            ReturnRecordsToMemory(recordsToRetry);
            throw;
        }

        if (!recordsToRetry.empty())
        {
            LOG_WARN("Flush: per-record disk store failed after saving %zu of %zu records; returning %zu records to the queue for retry",
                totalSaved, records.size(), recordsToRetry.size());
            ReturnRecordsToMemory(recordsToRetry);
        }

        return totalSaved;
    }

    size_t OfflineStorageHandler::ReturnRecordsToMemory(std::vector<StorageRecord> const& records)
    {
        size_t returned = 0;
        DroppedMap dropped;

        for (auto const& record : records)
        {
            try
            {
                if (m_offlineStorageMemory && m_offlineStorageMemory->StoreRecord(record))
                {
                    ++returned;
                    continue;
                }
                LOG_ERROR("Flush: failed to return event %s:%s to memory queue after disk store failure; dropping record",
                    tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
                dropped[record.tenantToken]++;
            }
            catch (const std::exception& e)
            {
                std::fprintf(stderr, "Failed to recover a record after flush failure: %s\n", e.what());
            }
            catch (...)
            {
                std::fputs("Failed to recover a record after flush failure\n", stderr);
            }
        }

        if (!dropped.empty())
        {
            try
            {
                OnStorageRecordsDropped(dropped);
            }
            catch (const std::exception& e)
            {
                std::fprintf(stderr, "Failed to report dropped records after flush failure: %s\n", e.what());
            }
            catch (...)
            {
                std::fputs("Failed to report dropped records after flush failure\n", stderr);
            }
        }

        return returned;
    }

    size_t OfflineStorageHandler::StoreRecords(std::vector<StorageRecord>& records)
    {
        size_t stored = 0;
        for (auto& i : records)
        {
            if (StoreRecord(i))
            {
                ++stored;
            }
        }
        return stored;
    }

    bool OfflineStorageHandler::ResizeDb()
    {
        if (nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->ResizeDb();
        }

        if (nullptr != m_offlineStorageDisk)
        {
            m_offlineStorageDisk->ResizeDb();
        }

        return true;
    }

    bool OfflineStorageHandler::IsLastReadFromMemory()
    {
        return m_readFromMemory;
    }

    unsigned OfflineStorageHandler::LastReadRecordCount()
    {
        return m_lastReadCount;
    }

    bool OfflineStorageHandler::GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventLatency minLatency, unsigned maxCount)
    {
        bool returnValue = false;

        m_lastReadCount = 0;
        m_readFromMemory = false;

        if (m_offlineStorageMemory)
        {
            returnValue |= m_offlineStorageMemory->GetAndReserveRecords(consumer, leaseTimeMs, minLatency, maxCount);
            m_lastReadCount += m_offlineStorageMemory->LastReadRecordCount();
            if (m_lastReadCount <= maxCount)
                maxCount -= m_lastReadCount;
            m_readFromMemory = true;
            // Prefer to send all of in-memory first before going to disk. This also helps in case if in-ram queue
            // is larger than request size (2MB), we'd exit the function because the consumer no longer wants more
            // records.
            if (m_lastReadCount)
                return returnValue;
        }

        if (m_offlineStorageDisk)
        {
            returnValue |= m_offlineStorageDisk->GetAndReserveRecords(consumer, leaseTimeMs, minLatency, maxCount);
            auto lastOfflineReadCount = m_offlineStorageDisk->LastReadRecordCount();
            if (lastOfflineReadCount)
            {
                m_lastReadCount += lastOfflineReadCount;
                m_readFromMemory = false;
            }
        }

        if (m_config.IsClockSkewEnabled() && !m_clockSkewManager.GetResumeTransmissionAfterClockSkew()
            /* && !consumedIds.empty() */
        )
        {
            m_clockSkewManager.GetDelta();
        }

        return returnValue;
    }

    std::vector<StorageRecord> OfflineStorageHandler::GetRecords(bool shutdown, EventLatency minLatency, unsigned maxCount)
    {
        // This method should not be called directly because it's a no-op
        assert(false);

        UNREFERENCED_PARAMETER(shutdown);
        UNREFERENCED_PARAMETER(minLatency);
        UNREFERENCED_PARAMETER(maxCount);
        return std::vector<StorageRecord>{};
    }

    /**
     * Delete records by API ingestion key aka "Tenant Token".
     * Internal method used by DeleteRecords and ReleaseRecords
     * invoked by HTTP callback thread. The scrubbing is done
     * async in context where the HTTP callback is running.
     */
    void OfflineStorageHandler::DeleteRecordsByKeys(const std::list<std::string>& keys)
    {
        for (const auto& key : keys)
        {
            /* DELETE * FROM events WHERE tenant_token=${key} */
            DeleteRecords({{"tenant_token", key}});
        }
    }

    /**
     * Delete all records locally".
     */

    void OfflineStorageHandler::DeleteAllRecords()
    {
        for (const auto storagePtr : { m_offlineStorageMemory.get() , m_offlineStorageDisk.get() })
        {
            if (storagePtr != nullptr)
            {
                storagePtr->DeleteAllRecords();
            }
        }

    }

    /**
     * Perform scrub of both memory queue and offline storage.
     */
    /// <summary>
    /// Perform scrub of underlying storage systems using 'where' clause
    /// </summary>
    /// <param name="whereFilter">The where filter.</param>
    /// <remarks>
    /// whereFilter contains the key-value pairs for the
    /// WHERE [key0==value0 .. keyN==valueN] clause.
    /// </remarks>
    void OfflineStorageHandler::DeleteRecords(const std::map<std::string, std::string>& whereFilter)
    {
        for (const auto storagePtr : {m_offlineStorageMemory.get(), m_offlineStorageDisk.get()})
        {
            if (storagePtr != nullptr)
            {
                storagePtr->DeleteRecords(whereFilter);
            }
        }
    }

    /// <summary>
    /// Delete records that would match the set of ids or based on kill-switch header
    /// </summary>
    /// <param name="ids">Identifiers of records to delete</param>
    /// <param name="headers">Headers may indicate "Kill-Token" several times</param>
    /// <param name="fromMemory">Flag that indicates where to delete from by IDs</param>
    /// <remarks>
    /// IDs of records that are no longer found in the storage are silently ignored.
    /// Called from the internal worker thread.
    /// Killed tokens deleted from both - memory storage and offline storage if available.
    /// </remarks>
    void OfflineStorageHandler::DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory)
    {
        if (m_clockSkewManager.isWaitingForClockSkew())
        {
            m_clockSkewManager.handleResponse(headers);
        }

        /* Handle delete of killed tokens on 200 OK or non-retryable status code */
        if ((!headers.empty()) && m_killSwitchManager.handleResponse(headers))
        {
            /* Since we got the ask for a new token kill, means we sent something we should now stop sending */
            LOG_TRACE("Scrub all pending events associated with killed token(s)");
            DeleteRecordsByKeys(m_killSwitchManager.getTokensList());
        }

        LOG_TRACE(" OfflineStorageHandler Deleting %u sent event(s) {%s%s}...",
                  static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");
        if (fromMemory && nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->DeleteRecords(ids, headers, fromMemory);
        }
        else
        {
            if (nullptr != m_offlineStorageDisk)
            {
                m_offlineStorageDisk->DeleteRecords(ids, headers, fromMemory);
            }
        }
    }

    void OfflineStorageHandler::ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory)
    {
        if (m_clockSkewManager.isWaitingForClockSkew())
        {
            m_clockSkewManager.handleResponse(headers);
        }

        /* Handle delete of kills tokens on 503 or other retryable status code */
        if ((!headers.empty()) && m_killSwitchManager.handleResponse(headers))
        {
            /* Since we got the ask for a new token kill, means we sent something we should now stop sending */
            LOG_TRACE("Scrub all pending events associated with killed token(s)");
            DeleteRecordsByKeys(m_killSwitchManager.getTokensList());
        }

        if (fromMemory && nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->ReleaseRecords(ids, incrementRetryCount, headers, fromMemory);
        }
        else
        {
            if (nullptr != m_offlineStorageDisk)
            {
                m_offlineStorageDisk->ReleaseRecords(ids, incrementRetryCount, headers, fromMemory);
            }
        }
    }

    bool OfflineStorageHandler::StoreSetting(std::string const& name, std::string const& value)
    {
        if (nullptr != m_offlineStorageDisk)
        {
            m_offlineStorageDisk->StoreSetting(name, value);
            return true;
        }
        return false;
    }

    std::string OfflineStorageHandler::GetSetting(std::string const& name)
    {
        if (nullptr != m_offlineStorageDisk)
        {
            return m_offlineStorageDisk->GetSetting(name);
        }
        return "";
    }

    bool OfflineStorageHandler::DeleteSetting(std::string const& name)
    {
        if (nullptr != m_offlineStorageDisk)
        {
            return m_offlineStorageDisk->DeleteSetting(name);
        }
        return false;
    }

    void OfflineStorageHandler::OnStorageOpened(std::string const& type)
    {
        m_observer->OnStorageOpened(type);
    }

    void OfflineStorageHandler::OnStorageFailed(std::string const& reason)
    {
        m_observer->OnStorageOpenFailed(reason);
    }

    void OfflineStorageHandler::OnStorageOpenFailed(std::string const& reason)
    {
        m_observer->OnStorageOpenFailed(reason);
    }

    void OfflineStorageHandler::OnStorageTrimmed(std::map<std::string, size_t> const& numRecords)
    {
        m_observer->OnStorageTrimmed(numRecords);
    }

    void OfflineStorageHandler::OnStorageRecordsDropped(std::map<std::string, size_t> const& numRecords)
    {
        m_observer->OnStorageRecordsDropped(numRecords);
    }

    void OfflineStorageHandler::OnStorageRecordsRejected(std::map<std::string, size_t> const& numRecords)
    {
        m_observer->OnStorageRecordsRejected(numRecords);
    }

    void OfflineStorageHandler::OnStorageRecordsSaved(size_t numRecords)
    {
        m_observer->OnStorageRecordsSaved(numRecords);
    }

} MAT_NS_END
