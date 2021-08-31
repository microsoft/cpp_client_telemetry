//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "OfflineStorageHandler.hpp"
#include "OfflineStorageFactory.hpp"

#include "offline/MemoryStorage.hpp"

#include "ILogManager.hpp"
#include <algorithm>
#include <numeric>
#include <set>

namespace MAT_NS_BEGIN {


    MATSDK_LOG_INST_COMPONENT_CLASS(OfflineStorageHandler, "EventsSDK.StorageHandler", "Events telemetry client - OfflineStorageHandler class");

    OfflineStorageHandler::OfflineStorageHandler(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher) :
        m_observer(nullptr),
        m_logManager(logManager),
        m_config(runtimeConfig),
        m_taskDispatcher(taskDispatcher),
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
        m_isStorageFullNotificationSend(false)
    {
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
        LOG_INFO("Waiting for pending Flush (%p) to complete...", m_flushHandle.m_task);
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
        uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];

        m_offlineStorageDisk = OfflineStorageFactory::Create(m_logManager, m_config);
        if (m_offlineStorageDisk)
        {
            m_offlineStorageDisk->Initialize(*this);
        }

        // TODO: [MG] - consider passing m_offlineStorageDisk to m_offlineStorageMemory,
        // so that the Flush() op on memory storage leads to saving unflushed events to
        // disk.
        if (cacheMemorySizeLimitInBytes > 0)
        {
            m_offlineStorageMemory.reset(new MemoryStorage(m_logManager, m_config));
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

    void OfflineStorageHandler::Flush()
    {
        // Flush could be executed from context of worker thread, as well as from TPM and
        // after HTTP callback. Make sure it is atomic / thread-safe.
        LOCKGUARD(m_flushLock);

        // If item isn't scheduled yet, it gets canceled, so that we don't do two flushes.
        // If we are running that item right now (our thread), then nothing happens other
        // than the handle gets replaced by nullptr in this DeferredCallbackHandle obj.
        m_flushHandle.Cancel();

        size_t dbSizeBeforeFlush = m_offlineStorageMemory->GetSize();
        if ((m_offlineStorageMemory) && (dbSizeBeforeFlush > 0) && (m_offlineStorageDisk))
        {
            // This will block on and then take a lock for the duration of this move, and
            // StoreRecord() will then block until the move completes.
            auto records = m_offlineStorageMemory->GetRecords(false, EventLatency_Unspecified);
            std::vector<StorageRecordId> ids;

            // TODO: [MG] - consider running the batch in transaction
            //            if (sqlite)
            //                sqlite->Execute("BEGIN");

            size_t totalSaved = m_offlineStorageDisk->StoreRecords(records);

            // TODO: [MG] - consider running the batch in transaction
            //            if (sqlite)
            //                sqlite->Execute("END");

            // Delete records from reserved on flush
            HttpHeaders dummy;
            bool fromMemory = true;
            m_offlineStorageMemory->DeleteRecords(ids, dummy, fromMemory);

            // Notify event listener about the records cached
            OnStorageRecordsSaved(totalSaved);

            if (m_offlineStorageMemory->GetSize() > dbSizeBeforeFlush)
            {
                // We managed to accumulate as much data as we had before the flush,
                // means we cannot keep up flushing at the same speed as incoming
                // obviously because the disk is slower than ram.
                LOG_WARN("Data is arriving too fast!");
            }
        }

        m_isStorageFullNotificationSend = false;

        // Flush is done, notify the waiters
        m_flushComplete.post();
        m_flushPending = false;
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

        // Check cache size only once at start
        static uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];

        if (nullptr != m_offlineStorageMemory && !m_shutdownStarted)
        {
            auto memDbSize = m_offlineStorageMemory->GetSize();
            {
                // During flush, this will block on a mutex while records
                // are selected and removed from the cache (but will
                // not block for the subsequent handoff to persistent
                // storage)
                m_offlineStorageMemory->StoreRecord(record);
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
                        LOG_INFO("Requested Flush (%p)", m_flushHandle.m_task);
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
                    m_offlineStorageDisk->StoreRecord(record);
                }
            }
        }

        return true;
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

