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
        m_isStorageFullNotificationSend(false)
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

    OfflineStorageHandler::~OfflineStorageHandler()
    {
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
        if (nullptr != m_offlineStorageMemory)
        {  // transfer data from Memory DB to Disk DB before shutting down.
            std::vector<StorageRecord>* records = m_offlineStorageMemory->GetRecords(true, EventLatency_Normal, 0);
            std::vector<StorageRecord>::const_iterator iter;
            for (iter = records->begin(); iter != records->end(); iter++)
            {
                m_offlineStorageDisk->StoreRecord(*iter);
            }
            records->clear();
            delete records;
        }
        m_offlineStorageDisk->Shutdown();
        if (nullptr != m_offlineStorageMemory)
        {
            m_offlineStorageMemory->Shutdown();
        }
    }

    unsigned OfflineStorageHandler::GetSize()
    {
        return 0;
    }

    bool OfflineStorageHandler::StoreRecord(StorageRecord const& record)
    {
        if (nullptr != m_offlineStorageMemory && !m_shutdownStarted)
        {
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
                DebugEvent evt;
                evt.type = DebugEventType::EVT_STORAGE_FULL;
                evt.param1 = 1;
                m_logManager.DispatchEvent(evt);
                m_isStorageFullNotificationSend = true;
            }

            uint32_t cacheMemorySizeLimitInBytes = m_config[CFG_INT_RAM_QUEUE_SIZE];

            if (m_queryDbSize > cacheMemorySizeLimitInBytes)
            {
                // transfer data from Memory DB to Disk DB
                std::vector<StorageRecord>* records = m_offlineStorageMemory->GetRecords(false, EventLatency_Normal, 500);
                std::vector<StorageRecord>::const_iterator iter;
                std::vector<StorageRecordId> recordIds;
                for (iter = records->begin(); iter != records->end(); iter++)
                {
                    recordIds.push_back(iter->id);
                    m_offlineStorageDisk->StoreRecord(*iter);
                }
                HttpHeaders temp;
                bool fromMemory;
                m_offlineStorageMemory->DeleteRecords(recordIds, temp, fromMemory);

                delete records;
                //Resize the memory DB after delete
                m_offlineStorageMemory->ResizeDb();
                m_queryDbSize = 100;
                m_isStorageFullNotificationSend = false;
            }
            m_offlineStorageMemory->StoreRecord(record);
        }
        else
        {
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

    bool OfflineStorageHandler::GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventLatency minLatency, unsigned maxCount)
    {
        bool returnValue = false;
        m_lastReadCount = 0;
        // FIXME: [MG] - memory corruption here...
        if (nullptr != m_offlineStorageMemory && m_offlineStorageMemory->GetAndReserveRecords(consumer, leaseTimeMs, minLatency, maxCount))
        {
            m_readFromMemory = true;
            returnValue = true;
            m_lastReadCount = m_offlineStorageMemory->LastReadRecordCount();
        }

        if (nullptr == m_offlineStorageMemory || m_lastReadCount == 0)
        { //nothing in memory left to send. lets end from disk
            m_readFromMemory = false;
            returnValue = m_offlineStorageDisk->GetAndReserveRecords(consumer, leaseTimeMs, minLatency, maxCount);
            m_lastReadCount = m_offlineStorageDisk->LastReadRecordCount();
        }

        return returnValue;
    }

    bool OfflineStorageHandler::IsLastReadFromMemory()
    {
        return m_readFromMemory;
    }
    unsigned OfflineStorageHandler::LastReadRecordCount()
    {
        return  m_lastReadCount;
    }

    std::vector<StorageRecord>* OfflineStorageHandler::GetRecords(bool shutdown, EventLatency minLatency, unsigned maxCount)
    {
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
} ARIASDK_NS_END
