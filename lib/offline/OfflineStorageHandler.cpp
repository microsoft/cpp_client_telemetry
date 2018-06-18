// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorageHandler.hpp"
#include "offline/OfflineStorage_SQLite.hpp"
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
        m_offlineStorageDisk.reset();
    }

    void OfflineStorageHandler::Initialize(IOfflineStorageObserver& observer)
    {
        m_observer = &observer;
        uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];

        if (cacheMemorySizeLimitInBytes > 0)
        {
            m_offlineStorageMemory.reset(new OfflineStorage_SQLite(m_logManager, m_config, true));
            m_offlineStorageMemory->Initialize(*this);
        }

        m_offlineStorageDisk.reset(new OfflineStorage_SQLite(m_logManager, m_config));
        m_offlineStorageDisk->Initialize(*this);

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
            Flush();
            m_offlineStorageMemory->Shutdown();
        }
        m_offlineStorageDisk->Shutdown();
    }

    unsigned OfflineStorageHandler::GetSize()
    {
        // TODO: [MG] - add sum of memory + offline
        return 0;
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

        for (auto latency : { EventLatency_Normal , EventLatency_CostDeferred , EventLatency_RealTime , EventLatency_Max })
        {
            std::vector<StorageRecord>* records = m_offlineStorageMemory->GetRecords(false, latency);
            if (records->size())
            {
                std::vector<StorageRecord>::const_iterator iter;
                std::vector<StorageRecordId> recordIds;
                for (iter = records->begin(); iter != records->end(); iter++)
                {
                    recordIds.push_back(iter->id);
                    m_offlineStorageDisk->StoreRecord(*iter);
                }
                OnStorageRecordsSaved(records->size());
                HttpHeaders temp;
                bool fromMemory = true;
                m_offlineStorageMemory->DeleteRecords(recordIds, temp, fromMemory);
            }
            delete records;
        }

        //Resize the memory DB after delete
        m_offlineStorageMemory->ResizeDb();
        m_queryDbSize = 100;
        m_isStorageFullNotificationSend = false;

        // Flush is done, notify the waiters
        m_flushComplete.post();
        m_flushPending = false;
    }

    // TODO: [MG] - investigate if StoreRecord is thread-safe if executed simultaneously with Flush
    bool OfflineStorageHandler::StoreRecord(StorageRecord const& record)
    {
        if (nullptr != m_offlineStorageMemory && !m_shutdownStarted)
        {
            {
                LOCKGUARD(m_flushLock);
                ++m_queryDbSize;

                //query DB size from DB only every 100 Events, use size calculation done before that
                if (m_queryDbSize >= 100)
                {
                    m_queryDbSize = 0;
                    m_memoryDbSize = m_offlineStorageMemory->GetSize();
                }

                m_queryDbSize = m_queryDbSize + static_cast<unsigned>(/* empiric estimate */ 32 + 2 * record.id.size() + record.tenantToken.size() + record.blob.size());

                //check if Application needs to be notified
                if (m_memoryDbSize > m_memoryDbSizeNotificationLimit && !m_isStorageFullNotificationSend)
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
            uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];
            if (m_queryDbSize > cacheMemorySizeLimitInBytes)
            {
                // Schedule flush if there isn't one already pending
                LOCKGUARD(m_flushLock);
                if (!m_flushPending)
                {
                    m_flushPending = true;
                    m_flushComplete.Reset();
                    m_flushHandle = PAL::scheduleOnWorkerThread(0, this, &OfflineStorageHandler::Flush);
                    LOG_INFO("Requested Flush (%p)", m_flushHandle.m_item);
                }
            }
        }
        else
        {
            OnStorageRecordsSaved(1);
            m_offlineStorageDisk->StoreRecord(record);
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
        LOCKGUARD(m_flushLock);

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
        if (m_shutdownStarted)
        {
            return;
        }

        LOCKGUARD(m_flushLock);

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
        if (m_shutdownStarted)
        {
            return;
        }

        LOCKGUARD(m_flushLock);

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
