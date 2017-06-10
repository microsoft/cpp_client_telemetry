// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorageHandler.hpp"
#include "offline/OfflineStorage_SQLite.hpp"
#include "LogManager.hpp"
#include <algorithm>
#include <numeric>
#include <set>

namespace ARIASDK_NS_BEGIN {


ARIASDK_LOG_INST_COMPONENT_CLASS(OfflineStorageHandler, "AriaSDK.StorageHandler", "Aria telemetry client - OfflineStorageHandler class");

OfflineStorageHandler::OfflineStorageHandler(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig)
    : m_logConfiguration(configuration),
    m_runtimeConfig(runtimeConfig),
    m_offlineStorageMemory(nullptr),
    m_offlineStorageDisk(nullptr),
    m_readFromMemory(false),
    m_lastReadCount(0)
{
}

OfflineStorageHandler::~OfflineStorageHandler()
{
    if (nullptr != m_offlineStorageMemory)
    {
        m_offlineStorageMemory->Shutdown();
        m_offlineStorageMemory.reset();
    }	
    m_offlineStorageDisk.reset();
}

void OfflineStorageHandler::Initialize(IOfflineStorageObserver& observer)
{
    m_observer = &observer;

    if (m_logConfiguration.cacheMemorySizeLimitInBytes > 0)
    {
        LogConfiguration inMemoryConfig = m_logConfiguration;
        inMemoryConfig.cacheFilePath = ":memory:";

        m_offlineStorageMemory.reset(new OfflineStorage_SQLite(inMemoryConfig, m_runtimeConfig));
        m_offlineStorageMemory->Initialize(*this);
    }

    m_offlineStorageDisk.reset(new OfflineStorage_SQLite(m_logConfiguration, m_runtimeConfig));
    m_offlineStorageDisk->Initialize(*this);

    ARIASDK_LOG_DETAIL("Initializing offline storage");
}

void OfflineStorageHandler::Shutdown()
{
    ARIASDK_LOG_DETAIL("Shutting down offline storage");
    if (nullptr != m_offlineStorageMemory)
    {
        // transfer data from Memory DB to Disk DB before shutting down.
        std::vector<StorageRecord>* records = m_offlineStorageMemory->GetRecords(true, EventPriority_Low, 0);
        std::vector<StorageRecord>::const_iterator iter;
        for (iter = records->begin(); iter != records->end(); iter++)
        {
            m_offlineStorageDisk->StoreRecord(*iter);
        }		
        delete records;
            
        //PAL::sleep(1000);
    }
    m_offlineStorageDisk->Shutdown();	
}

unsigned OfflineStorageHandler::GetSize() 
{
    return 0;
}

bool OfflineStorageHandler::StoreRecord(StorageRecord const& record)
{
    if (nullptr != m_offlineStorageMemory)
    {
        //check if the momory is full
        unsigned dbsize = m_offlineStorageMemory->GetSize();
        dbsize = dbsize + (/* empiric estimate */ 32 + 2 * record.id.size() + record.tenantToken.size() + record.blob.size());

        if (dbsize > m_logConfiguration.cacheMemorySizeLimitInBytes)
        {
            LogManager::DispatchEvent(DebugEventType::EVT_STORAGE_FULL);
            // transfer data from Memory DB to Disk DB
            std::vector<StorageRecord>* records = m_offlineStorageMemory->GetRecords(false, EventPriority_Low, 500);
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
            //transfer data from memory to disk;
        }
        m_offlineStorageMemory->StoreRecord(record);
    }
    else
    {
        m_offlineStorageDisk->StoreRecord(record);
    }

    return true;
}

bool OfflineStorageHandler::GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventPriority minPriority, unsigned maxCount)
{
    bool returnValue = false;
    m_lastReadCount = 0;
    if (nullptr != m_offlineStorageMemory && m_offlineStorageMemory->GetAndReserveRecords(consumer, leaseTimeMs, minPriority, maxCount))
    {
        m_readFromMemory = true;
        returnValue =  true;
        m_lastReadCount = m_offlineStorageMemory->LastReadRecordCount();
    }

    if(nullptr == m_offlineStorageMemory || m_lastReadCount == 0)
    { //nothing in memory left to send. lets end from disk
        m_readFromMemory = false;
        returnValue =  m_offlineStorageDisk->GetAndReserveRecords(consumer, leaseTimeMs, minPriority, maxCount);
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

std::vector<StorageRecord>* OfflineStorageHandler::GetRecords(bool shutdown, EventPriority minPriority, unsigned maxCount) 
{
    UNREFERENCED_PARAMETER(shutdown);
    UNREFERENCED_PARAMETER(minPriority);
    UNREFERENCED_PARAMETER(maxCount);
    std::vector<StorageRecord>* records = new std::vector<StorageRecord>();
    return records;
}

void OfflineStorageHandler::DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory)
{
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

void OfflineStorageHandler::OnStorageTrimmed(unsigned numRecords)
{
    m_observer->OnStorageTrimmed(numRecords);
}

void OfflineStorageHandler::OnStorageRecordsDropped(unsigned numRecords)
{
    m_observer->OnStorageRecordsDropped(numRecords);
}

} ARIASDK_NS_END
