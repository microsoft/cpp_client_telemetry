//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "OfflineStorageHandler.hpp"
#include "OfflineStorageFactory.hpp"

#include "offline/MemoryStorage.hpp"

#include "ILogManager.hpp"
#include <algorithm>
#include <exception>
#include <numeric>
#include <set>
#include <utility>

namespace MAT_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(OfflineStorageHandler, "EventsSDK.StorageHandler", "Events telemetry client - OfflineStorageHandler class")

    namespace
    {
        class ActivityGuard
        {
        public:
            explicit ActivityGuard(ILogManager& logManager) :
                m_logManager(logManager),
                m_active(logManager.StartActivity())
            {
            }

            ~ActivityGuard() noexcept
            {
                if (m_active)
                {
                    m_logManager.EndActivity();
                }
            }

            bool IsActive() const noexcept
            {
                return m_active;
            }

        private:
            ILogManager& m_logManager;
            bool m_active;
        };

        template<typename TFunc>
        class ScopeExit
        {
        public:
            explicit ScopeExit(TFunc&& func) noexcept :
                m_func(std::move(func)),
                m_active(true)
            {
            }

            ScopeExit(ScopeExit&& other) noexcept :
                m_func(std::move(other.m_func)),
                m_active(other.m_active)
            {
                other.m_active = false;
            }

            ScopeExit(const ScopeExit&) = delete;
            ScopeExit& operator=(const ScopeExit&) = delete;
            ScopeExit& operator=(ScopeExit&&) = delete;

            ~ScopeExit() noexcept
            {
                if (m_active)
                {
                    m_func();
                }
            }

        private:
            TFunc m_func;
            bool m_active;
        };

        template<typename TFunc>
        ScopeExit<TFunc> MakeScopeExit(TFunc&& func)
        {
            return ScopeExit<TFunc>(std::forward<TFunc>(func));
        }
    }

    class OfflineStorageHandler::OfflineStorageFlushTask final : public Task
    {
    public:
        explicit OfflineStorageFlushTask(OfflineStorageHandler& handler) :
            Task(),
            m_handler(handler)
        {
            Type = Task::Call;
            TargetTime = 0;
            TypeName = "OfflineStorageFlushTask";
        }

        ~OfflineStorageFlushTask() noexcept override
        {
            if (!m_started)
            {
                m_handler.DropScheduledFlush();
            }
        }

        void operator()() override
        {
            m_started = true;
            m_handler.RunScheduledFlush();
        }

    private:
        OfflineStorageHandler& m_handler;
        bool m_started = false;
    };

    OfflineStorageHandler::OfflineStorageHandler(ILogManager& logManager, IRuntimeConfig& runtimeConfig, ITaskDispatcher& taskDispatcher) :
        m_observer(nullptr),
        m_logManager(logManager),
        m_config(runtimeConfig),
        m_taskDispatcher(taskDispatcher),
        m_killSwitchManager(),
        m_clockSkewManager(),
        m_phase(StoragePhase::Stopped),
        m_inFlight(0),
        m_scheduled(false),
        m_offlineStorageMemory(nullptr),
        m_offlineStorageDisk(nullptr),
        m_readFromMemory(false),
        m_lastReadCount(0),
        m_memoryDbSize(0),
        m_queryDbSize(0),
        m_cacheMemorySizeLimitInBytes(0),
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

    bool OfflineStorageHandler::BeginOperation()
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        if (m_phase != StoragePhase::Accepting)
        {
            return false;
        }
        ++m_inFlight;
        return true;
    }

    void OfflineStorageHandler::EndOperation()
    {
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            --m_inFlight;
        }
        m_stateCV.notify_all();
    }

    void OfflineStorageHandler::DropScheduledFlush()
    {
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            if (!m_scheduled)
            {
                return;
            }
            m_scheduled = false;
            --m_inFlight;
        }
        m_stateCV.notify_all();
    }

    bool OfflineStorageHandler::BeginTeardown()
    {
        std::unique_lock<std::mutex> lock(m_stateMutex);
        if (m_phase != StoragePhase::Accepting)
        {
            m_stateCV.wait(lock, [this] { return m_phase == StoragePhase::Stopped; });
            return false;
        }
        m_phase = StoragePhase::Draining;
        m_stateCV.wait(lock, [this] { return m_inFlight == 0; });
        m_phase = StoragePhase::TearingDown;
        return true;
    }

    void OfflineStorageHandler::FinishTeardown()
    {
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_phase = StoragePhase::Stopped;
        }
        m_stateCV.notify_all();
    }

    OfflineStorageHandler::~OfflineStorageHandler()
    {
        if (BeginTeardown())
        {
            {
                std::lock_guard<std::mutex> lock(m_ioMutex);
                m_offlineStorageMemory.reset();
                m_offlineStorageDisk.reset();
            }
            FinishTeardown();
        }
    }

    void OfflineStorageHandler::Initialize(IOfflineStorageObserver& observer)
    {
        m_observer = &observer;
        m_cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];

        m_offlineStorageDisk = OfflineStorageFactory::Create(m_logManager, m_config);
        if (m_offlineStorageDisk)
        {
            m_offlineStorageDisk->Initialize(*this);
        }

        // TODO: [MG] - consider passing m_offlineStorageDisk to m_offlineStorageMemory,
        // so that the Flush() op on memory storage leads to saving unflushed events to
        // disk.
        if (m_cacheMemorySizeLimitInBytes > 0)
        {
            m_offlineStorageMemory.reset(new MemoryStorage(m_logManager, m_config));
            m_offlineStorageMemory->Initialize(*this);
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        if (m_phase == StoragePhase::Stopped)
        {
            m_phase = StoragePhase::Accepting;
        }
        LOG_TRACE("Initializing offline storage handler");
    }

    void OfflineStorageHandler::Shutdown()
    {
        LOG_TRACE("Shutting down offline storage handler");
        if (!BeginTeardown())
        {
            return;
        }

        auto finishTeardown = MakeScopeExit([this] { FinishTeardown(); });
        size_t savedRecords = 0;
        bool notifySaved = false;
        {
            std::lock_guard<std::mutex> lock(m_ioMutex);
            if (m_offlineStorageMemory != nullptr)
            {
                m_offlineStorageMemory->ReleaseAllRecords();
#if HAVE_EXCEPTIONS
                try
                {
                    notifySaved = FlushImpl(savedRecords);
                }
                catch (const std::exception& ex)
                {
                    LOG_ERROR("Offline storage shutdown flush failed: %s", ex.what());
                }
                catch (...)
                {
                    LOG_ERROR("Offline storage shutdown flush failed");
                }
#else
                notifySaved = FlushImpl(savedRecords);
#endif
                m_offlineStorageMemory->Shutdown();
            }
            if (m_offlineStorageDisk != nullptr)
            {
                m_offlineStorageDisk->Shutdown();
            }
        }
        if (notifySaved)
        {
            OnStorageRecordsSaved(savedRecords);
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
        if (!BeginOperation())
        {
            return;
        }
        auto completion = MakeScopeExit([this] { EndOperation(); });
        ActivityGuard activity(m_logManager);
        if (activity.IsActive())
        {
            size_t savedRecords = 0;
            bool notifySaved;
            {
                std::lock_guard<std::mutex> lock(m_ioMutex);
                notifySaved = FlushImpl(savedRecords);
            }
            if (notifySaved)
            {
                OnStorageRecordsSaved(savedRecords);
            }
        }
    }

    void OfflineStorageHandler::RunScheduledFlush()
    {
        {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            m_scheduled = false;
        }
        auto completion = MakeScopeExit([this] { EndOperation(); });
        ActivityGuard activity(m_logManager);
        if (activity.IsActive())
        {
            size_t savedRecords = 0;
            bool notifySaved;
            {
                std::lock_guard<std::mutex> lock(m_ioMutex);
                notifySaved = FlushImpl(savedRecords);
            }
            if (notifySaved)
            {
                OnStorageRecordsSaved(savedRecords);
            }
        }
    }

    bool OfflineStorageHandler::FlushImpl(size_t& savedRecords)
    {
        bool notifySaved = false;
        size_t dbSizeBeforeFlush = (m_offlineStorageMemory != nullptr) ? m_offlineStorageMemory->GetSize() : 0;
        if ((m_offlineStorageMemory) && (dbSizeBeforeFlush > 0) && (m_offlineStorageDisk))
        {
            // This will block on and then take a lock for the duration of this move, and
            // StoreRecord() will then block until the move completes.
            auto memoryRecords =
                m_offlineStorageMemory->GetRecords(false, EventLatency_Unspecified);
            std::vector<StorageRecord> persistentRecords;
            std::vector<StorageRecord> memoryOnlyRecords;
            persistentRecords.reserve(memoryRecords.size());
            memoryOnlyRecords.reserve(memoryRecords.size());
            for (auto& record : memoryRecords)
            {
                if (record.persistence != EventPersistence_DoNotStoreOnDisk)
                {
                    persistentRecords.push_back(std::move(record));
                }
                else
                {
                    memoryOnlyRecords.push_back(std::move(record));
                }
            }
            m_offlineStorageMemory->StoreRecords(memoryOnlyRecords);

            // TODO: [MG] - consider running the batch in transaction
            //            if (sqlite)
            //                sqlite->Execute("BEGIN");

            // IOfflineStorage::StoreRecords accepts a mutable vector, so an
            // external storage module may consume or reorder its input. Keep an
            // untouched batch for exception and partial-write recovery.
            auto recordsForRetry = persistentRecords;
            size_t const recordsToSave = recordsForRetry.size();
            size_t totalSaved = 0;
#if HAVE_EXCEPTIONS
            try
            {
                totalSaved = m_offlineStorageDisk->StoreRecords(persistentRecords);
            }
            catch (...)
            {
                // GetRecords() removes records from the RAM queue. Restore them
                // before propagating so a transient disk failure cannot lose data.
                m_offlineStorageMemory->StoreRecords(recordsForRetry);
                throw;
            }
#else
            totalSaved = m_offlineStorageDisk->StoreRecords(persistentRecords);
#endif

            // TODO: [MG] - consider running the batch in transaction
            //            if (sqlite)
            //                sqlite->Execute("END");

            if (totalSaved != recordsToSave)
            {
                // StoreRecords reports only a count, not the failed record IDs.
                // Restore the complete batch to preserve at-least-once delivery.
                m_offlineStorageMemory->StoreRecords(recordsForRetry);
            }

            savedRecords = totalSaved;
            notifySaved = true;

            if (m_offlineStorageMemory->GetSize() > dbSizeBeforeFlush)
            {
                // We managed to accumulate as much data as we had before the flush,
                // means we cannot keep up flushing at the same speed as incoming
                // obviously because the disk is slower than ram.
                LOG_WARN("Data is arriving too fast!");
            }
        }

        // Checkpoint DB
        if (m_offlineStorageDisk != nullptr &&
            m_config.HasConfig(CFG_BOOL_CHECKPOINT_DB_ON_FLUSH) &&
            m_config[CFG_BOOL_CHECKPOINT_DB_ON_FLUSH])
        {
            m_offlineStorageDisk->Flush();
        }

        m_isStorageFullNotificationSend = false;
        return notifySaved;
    }

    bool OfflineStorageHandler::StoreRecord(StorageRecord const& record)
    {
        if (!BeginOperation())
        {
            return false;
        }
        auto completion = MakeScopeExit([this] { EndOperation(); });
        if (isKilled(record))
        {
            return false;
        }

        uint32_t cacheMemorySizeLimitInBytes = m_cacheMemorySizeLimitInBytes;
        if (nullptr != m_offlineStorageMemory)
        {
            auto memDbSize = m_offlineStorageMemory->GetSize();
            m_offlineStorageMemory->StoreRecord(record);
            if (memDbSize > cacheMemorySizeLimitInBytes)
            {
                bool queueFlush = false;
                {
                    std::lock_guard<std::mutex> lock(m_stateMutex);
                    if (m_phase == StoragePhase::Accepting && !m_scheduled)
                    {
                        m_scheduled = true;
                        ++m_inFlight;
                        queueFlush = true;
                    }
                }
                if (queueFlush)
                {
#if HAVE_EXCEPTIONS
                    try
                    {
                        m_taskDispatcher.Queue(new OfflineStorageFlushTask(*this));
                    }
                    catch (...)
                    {
                        DropScheduledFlush();
                        throw;
                    }
#else
                    m_taskDispatcher.Queue(new OfflineStorageFlushTask(*this));
#endif
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

        if (ids.empty())
        {
            return;
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
