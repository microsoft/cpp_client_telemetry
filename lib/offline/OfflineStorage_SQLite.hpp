// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "pal/PAL.hpp"
#include <IOfflineStorage.hpp>
#include <IRuntimeConfig.hpp>
#include <LogConfiguration.hpp>
#include "KillSwitchManager.hpp"
#include "ClockSkewManager.hpp"
#include <memory>

namespace ARIASDK_NS_BEGIN {


class SqliteDB;


class OfflineStorage_SQLite : public IOfflineStorage,
                              public PAL::RefCountedImpl<OfflineStorage_SQLite>
{
  public:
    OfflineStorage_SQLite(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig);
    virtual ~OfflineStorage_SQLite() override;
    virtual void Initialize(IOfflineStorageObserver& observer) override;
    virtual void Shutdown() override;
    virtual bool StoreRecord(StorageRecord const& record) override;
    virtual bool GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventPriority minPriority = EventPriority_Unspecified, unsigned maxCount = 0) override;
    virtual bool IsLastReadFromMemory() override;
    virtual unsigned LastReadRecordCount() override; 
	virtual void DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory) override;
    virtual void ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory) override;
    virtual bool StoreSetting(std::string const& name, std::string const& value) override;
    virtual std::string GetSetting(std::string const& name) override;
	virtual unsigned GetSize() override;
	virtual std::vector<StorageRecord>* GetRecords(bool shutdown, EventPriority minPriority = EventPriority_Unspecified, unsigned maxCount = 0) override;

  protected:
    bool initializeDatabase();
    bool recreate(unsigned failureCode);
    bool beginIfNotInTransaction();
    bool commitIfInTransaction();
    bool rollbackIfInTransaction();
    virtual void scheduleAutoCommitTransaction();
    void autoCommitTransaction();
    bool trimDbIfNeeded(size_t justAddedBytes);
    std::vector<uint8_t> packageIdList(std::vector<std::string> const& ids);

  protected:
    IOfflineStorageObserver*    m_observer;
    std::string                 m_databasePath;
    IRuntimeConfig&             m_runtimeConfig;
    std::unique_ptr<SqliteDB>   m_db;
    PAL::DeferredCallbackHandle m_scheduledAutoCommit;
    int                         m_pageSize;
    size_t                      m_currentlyAddedBytes;
    bool                        m_skipInitAndShutdown;
    bool                        m_isInTransaction;
    int                         m_stmtBeginTransaction;
    int                         m_stmtCommitTransaction;
    int                         m_stmtRollbackTransaction;
    int                         m_stmtGetPageCount;
    int                         m_stmtIncrementalVacuum0;
    int                         m_stmtTrimEvents_percent;
    int                         m_stmtDeleteEvents_ids;
    int                         m_stmtReleaseExpiredEvents;
    int                         m_stmtSelectEvents;
	int                         m_stmtSelectEventsMinPriority;
    int                         m_stmtReserveEvents;
    int                         m_stmtReleaseEvents_ids_retryCountDelta;
    int                         m_stmtDeleteEventsRetried_maxRetryCount;
    int                         m_stmtInsertEvent_id_tenant_prio_ts_data;
    int                         m_stmtInsertSetting_name_value;
    int                         m_stmtDeleteSetting_name;
    int                         m_stmtSelectSetting_name;
	KillSwitchManager           m_killSwitchManager;
	ClockSkewManager            m_clockSkewManager;
	unsigned                    m_lastReadCount;

  protected:
    ARIASDK_LOG_DECL_COMPONENT_CLASS();
};


} ARIASDK_NS_END
