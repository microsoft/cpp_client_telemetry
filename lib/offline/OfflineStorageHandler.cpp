// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorageHandler.hpp"

#include "offline/OfflineStorage_SQLite.hpp"
#include "offline/MemoryStorage.hpp"

#include "ILogManager.hpp"
#include <algorithm>
#include <numeric>
#include <set>

namespace ARIASDK_NS_BEGIN {


    ARIASDK_LOG_INST_COMPONENT_CLASS(OfflineStorageHandler, "EventsSDK.StorageHandler", "Events telemetry client - OfflineStorageHandler class");

    OfflineStorageHandler::OfflineStorageHandler(ILogManager & logManager, IRuntimeConfig& runtimeConfig)
        : m_logManager(logManager),
        m_config(runtimeConfig),
        m_offlineStorageMemory(nullptr),
        m_offlineStorageDisk(nullptr),
        m_readFromMemory(false),
        m_lastReadCount(0),
        m_shutdownStarted(false),
        m_memoryDbSize(0),
        m_queryDbSize(0),
        m_isStorageFullNotificationSend(false),
        m_flushPending(false)
    {
        // FIXME: [MG] - this code seems redundant / suspicious because OfflineStorage_SQLite.cpp is doing the same thing...
        uint32_t percentage = m_config[CFG_INT_RAMCACHE_FULL_PCT];
        uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];
        if (percentage > 0 && percentage <= 100)
        {
            m_memoryDbSizeNotificationLimit = (percentage * cacheMemorySizeLimitInBytes) / 100;
        }
        else
        {// incase user has specified bad percentage, we stck to 75%
            m_memoryDbSizeNotificationLimit = (DB_FULL_NOTIFICATION_DEFAULT_PERCENTAGE * cacheMemorySizeLimitInBytes) / 100;
        }
    }

    void OfflineStorageHandler::WaitForFlush()
    {
        {
            LOCKGUARD(m_flushLock);
            if (!m_flushPending)
                return;
        }
        LOG_INFO("Waiting for pending Flush (%p) to complete...", m_flushHandle.m_item);
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

        m_offlineStorageDisk.reset(new OfflineStorage_SQLite(m_logManager, m_config));
        m_offlineStorageDisk->Initialize(*this);

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
        m_flushHandle.cancel();

        size_t dbSizeBeforeFlush = m_offlineStorageMemory->GetSize();
        if ((m_offlineStorageMemory) && (dbSizeBeforeFlush > 0) && (m_offlineStorageDisk))
        {
            
            std::vector<StorageRecord>* records = m_offlineStorageMemory->GetRecords(false, EventLatency_Unspecified);
            std::vector<StorageRecordId> ids;
            size_t totalSaved = 0;

////        OfflineStorage_SQLite *sqlite = dynamic_cast<OfflineStorage_SQLite *>(m_offlineStorageDisk.get());
// TODO: [MG] - consider running the batch in transaction
//            if (sqlite)
//                sqlite->Execute("BEGIN");

            while (records->size())
            {
                ids.push_back(records->back().id);
                if (m_offlineStorageDisk->StoreRecord(std::move(records->back())))
                    totalSaved++;
                records->pop_back();
            }

// TODO: [MG] - consider running the batch in transaction
//            if (sqlite)
//                sqlite->Execute("END");

            // Delete records from reserved on flush
            HttpHeaders dummy;
            bool fromMemory = true;
            m_offlineStorageMemory->DeleteRecords(ids, dummy, fromMemory);

            delete records;

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

    // TODO: [MG] - investigate if StoreRecord is thread-safe if executed simultaneously with Flush
    bool OfflineStorageHandler::StoreRecord(StorageRecord const& record)
    {
        // Check cache size only once at start
        static uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];

        if (nullptr != m_offlineStorageMemory && !m_shutdownStarted)
        {
            auto memDbSize = m_offlineStorageMemory->GetSize();
            {
                //check if Application needs to be notified
                if ( (memDbSize > m_memoryDbSizeNotificationLimit) && !m_isStorageFullNotificationSend)
                {
                    // TODO: [MG] - do we really need in-memory DB size limit notifications here?
                    DebugEvent evt;
                    evt.type = DebugEventType::EVT_STORAGE_FULL;
                    evt.param1 = 1;
                    m_logManager.DispatchEvent(evt);
                    m_isStorageFullNotificationSend = true;
                }

                // TODO: [MG] - investigate what happens if Flush from memory to disk
                // is happening concurrently with adding a new in-memory record
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
                        m_flushHandle = PAL::scheduleOnWorkerThread(0, this, &OfflineStorageHandler::Flush);
                        LOG_INFO("Requested Flush (%p)", m_flushHandle.m_item);
                    }
                    m_flushLock.unlock();
                }
            }
        }
        else
        {
            if (m_offlineStorageDisk != nullptr)
            {
                m_offlineStorageDisk->StoreRecord(record);
            }
        }

        return true;
    }

    bool OfflineStorageHandler::ResizeDb()
    {
        if (nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->ResizeDb();
        }

        m_offlineStorageDisk->ResizeDb();

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

        return returnValue;
    }

    std::vector<StorageRecord>* OfflineStorageHandler::GetRecords(bool shutdown, EventLatency minLatency, unsigned maxCount)
    {
        // This method should not be called directly because it's a no-op
        assert(false);

        UNREFERENCED_PARAMETER(shutdown);
        UNREFERENCED_PARAMETER(minLatency);
        UNREFERENCED_PARAMETER(maxCount);
        std::vector<StorageRecord>* records = new std::vector<StorageRecord>();
        return records;
    }

    void OfflineStorageHandler::DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory)
    {

        LOG_TRACE(" OfflineStorageHandler Deleting %u sent event(s) {%s%s}...",
            static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");
        if (fromMemory && nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->DeleteRecords(ids, headers, fromMemory);
        }
        else
        {
            m_offlineStorageDisk->DeleteRecords(ids, headers, fromMemory);
        }
    }

    void OfflineStorageHandler::ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory)
    {

        if (fromMemory && nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->ReleaseRecords(ids, incrementRetryCount, headers, fromMemory);
        }
        else
        {
            m_offlineStorageDisk->ReleaseRecords(ids, incrementRetryCount, headers, fromMemory);
        }
    }

    bool OfflineStorageHandler::StoreSetting(std::string const& name, std::string const& value)
    {
        m_offlineStorageDisk->StoreSetting(name, value);
        return true;
    }

    std::string OfflineStorageHandler::GetSetting(std::string const& name)
    {
        return m_offlineStorageDisk->GetSetting(name);
    }


    void OfflineStorageHandler::OnStorageOpened(std::string const& type)
    {
        m_observer->OnStorageOpened(type);
    }

    void OfflineStorageHandler::OnStorageFailed(std::string const& reason)
    {
        m_observer->OnStorageFailed(reason);
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

} ARIASDK_NS_END
