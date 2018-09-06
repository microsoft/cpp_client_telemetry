// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorage_SQLite.hpp"
#include "ILogManager.hpp"
#include "SQLiteWrapper.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
#include <numeric>
#include <set>

namespace ARIASDK_NS_BEGIN {

    class DbTransaction {
        SqliteDB* m_db;
    public:
        bool locked;

        DbTransaction(SqliteDB* db) : m_db(db), locked(false)
        {
            locked = m_db->trylock();
        };

        ~DbTransaction()
        {
            if (locked)
                m_db->unlock();
        }
    };

    ARIASDK_LOG_INST_COMPONENT_CLASS(OfflineStorage_SQLite, "EventsSDK.Storage", "Events telemetry client - OfflineStorage_SQLite class");

    static int const CURRENT_SCHEMA_VERSION = 1;
#define TABLE_NAME_EVENTS   "events"
#define TABLE_NAME_SETTINGS "settings"

    OfflineStorage_SQLite::OfflineStorage_SQLite(ILogManager & logManager, IRuntimeConfig& runtimeConfig, bool inMemory)
        : m_logManager(logManager),
        m_config(runtimeConfig),
        m_skipInitAndShutdown(false),
        m_killSwitchManager(),
        m_clockSkewManager(),
        m_lastReadCount(0),
        m_isStorageFullNotificationSend(false),
        m_isOpened(false),
        m_DbSizeHeapLimit(0)
    {
        uint32_t percentage = (inMemory) ? m_config[CFG_INT_RAMCACHE_FULL_PCT] : m_config[CFG_INT_STORAGE_FULL_PCT];
        uint32_t fileSize = (inMemory) ? m_config[CFG_INT_RAM_QUEUE_SIZE] : m_config[CFG_INT_CACHE_FILE_SIZE];

        m_offlineStorageFileName = (inMemory) ? ":memory:" : (const char *)m_config[CFG_STR_CACHE_FILE_PATH];

        if (percentage > 0 && percentage <= 100)
        {
            m_DbSizeNotificationLimit = (percentage * fileSize) / 100;
        }
        else
        {// incase user has specified bad percentage, we stck to 75%
            m_DbSizeNotificationLimit = (DB_FULL_NOTIFICATION_DEFAULT_PERCENTAGE * fileSize) / 100;
        }

#if 0
        // TODO: [MG] - impose optional RAM size constraint on sqlite3 heap
        uint32_t ramSizeLimit = m_config[CFG_INT_RAM_QUEUE_SIZE];
        m_DbSizeHeapLimit = ramSizeLimit;
#endif

        // TODO: [MG] - this needs to be moved into constant
        const char* skipSqliteInit = m_config["skipSqliteInitAndShutdown"];
        if (skipSqliteInit != nullptr)
        {
            if (std::string(skipSqliteInit) == "true")
                m_skipInitAndShutdown = true;
        }
    }

    OfflineStorage_SQLite::~OfflineStorage_SQLite()
    {
        assert(!m_db);
    }

    void OfflineStorage_SQLite::Initialize(IOfflineStorageObserver& observer)
    {
        m_observer = &observer;

        assert(!m_db);
        m_db.reset(new SqliteDB(m_skipInitAndShutdown));

        LOG_TRACE("Initializing offline storage: %s", m_offlineStorageFileName.c_str());
        auto sqlStartTime = GetUptimeMs();
        if (m_db->initialize(m_offlineStorageFileName, false, m_DbSizeHeapLimit) && initializeDatabase()) {
            LOG_INFO("Using configured on-disk database");
            m_observer->OnStorageOpened("SQLite/Default");
            sqlStartTime = GetUptimeMs() - sqlStartTime;
            LOG_INFO("Storage opened in %lld ms", sqlStartTime);
            m_isOpened = true;
            return;
        }

        // TODO: implement DB recreation
        // recreate(1);

        // DB is not opened
        m_db.reset();
        m_isOpened = false;
    }

    void OfflineStorage_SQLite::Shutdown()
    {
        LOG_TRACE("Shutting down offline storage %s", m_offlineStorageFileName.c_str());
        LOCKGUARD(m_lock);
        if (m_db) {
            if (m_isOpened) {
                m_db->shutdown();
                m_db.reset();
            }
            m_isOpened = false;
        }
    }

    void OfflineStorage_SQLite::Execute(std::string command)
    {
        if (m_db)
            m_db->execute(command.c_str());
    }

    bool OfflineStorage_SQLite::StoreRecord(StorageRecord const& record)
    {
        // TODO: [MG] - this works, but may not play nicely with several LogManager instances
        // static SqliteStatement sql_insert(*m_db, m_stmtInsertEvent_id_tenant_prio_ts_data);

        // TODO: [MG] - verify this codepath
        if (record.id.empty() || record.tenantToken.empty() || static_cast<int>(record.latency) < 0 || record.timestamp <= 0) {
            LOG_ERROR("Failed to store event %s:%s: Invalid parameters",
                tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
            m_observer->OnStorageFailed("Invalid parameters");
            return false;
        }

        if (!m_db) {
            LOG_ERROR("Failed to store event %s:%s: Database is not open",
                tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
            m_observer->OnStorageFailed("Database is not open");
            return false;
        }

        {
#ifdef ENABLE_LOCKING
            LOCKGUARD(m_lock);
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to store event %s:%s: Database error", tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
                m_observer->OnStorageFailed("Database error");
                return false;
            }
#endif
            SqliteStatement(*m_db, m_stmtInsertEvent_id_tenant_prio_ts_data).execute(record.id, record.tenantToken, static_cast<int>(record.latency), static_cast<int>(record.persistence), record.timestamp, record.blob);
            // m_observer->OnStorageRecordsSaved(1); // [MG]: don't issue notification for individual record, only for batches at higher-level
        }
        return true;

    }

    /// <summary>
    /// Gets the and reserve records.
    /// </summary>
    /// <param name="consumer">The consumer.</param>
    /// <param name="leaseTimeMs">The lease time ms.</param>
    /// <param name="minLatency">The minimum latency.</param>
    /// <param name="maxCount">The maximum count.</param>
    /// <returns></returns>
    bool OfflineStorage_SQLite::GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventLatency minLatency, unsigned maxCount)
    {
        m_lastReadCount = 0;

        if (!m_db) {
            LOG_ERROR("Failed to retrieve events to send: Database is not open");
            return false;
        }

        LOG_TRACE("Retrieving max. %u%s events of latency at least %d (%s)",
            maxCount, (maxCount > 0) ? "" : " (unlimited)", minLatency, latencyToStr(static_cast<EventLatency>(minLatency)));

        /* ============================================================================================================= */
        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to lock");
                return false;
            }
#endif
            SqliteStatement releaseStmt(*m_db, m_stmtReleaseExpiredEvents);

            // FIXME: [MG] - add error checking here
            if (!releaseStmt.execute(PAL::getUtcSystemTimeMs()))
                LOG_ERROR("Failed to release expired reserved events: Database error occurred");
            else {
                if (releaseStmt.changes() > 0) {
                    LOG_TRACE("Released %u expired reserved events", static_cast<unsigned>(releaseStmt.changes()));
                }
            }

            if (m_config.IsClockSkewEnabled() && m_clockSkewManager.isWaitingForClockSkew())
            {
                LOG_INFO("Not sending - waiting for clock-skew...");
                return false;
            }

            SqliteStatement selectStmt(*m_db, m_stmtSelectEvents);
            if (!selectStmt.select(static_cast<int>(minLatency), maxCount > 0 ? maxCount : -1)) {
                LOG_ERROR("Failed to retrieve events to send: Database error occurred, recreating database");
                recreate(204);
                return false;
            }

            std::vector<StorageRecordId> consumedIds;
            std::vector<StorageRecordId> killedConsumedIds;
            std::map<std::string, size_t> deletedData;

            StorageRecord record;
            int latency;

            while (selectStmt.getRow(record.id, record.tenantToken, latency, record.timestamp, record.retryCount, record.reservedUntil, record.blob))
            {
                if (latency < EventLatency_Off || latency > EventLatency_Max) {
                    record.latency = EventLatency_Normal;
                }
                else {
                    record.latency = static_cast<EventLatency>(latency);
                }
                // The record ID needs to be saved before std::move() below.
                if (!m_killSwitchManager.isTokenBlocked(record.tenantToken))
                {
                    consumedIds.push_back(record.id);
                    if (!consumer(std::move(record)))
                    {
                        consumedIds.pop_back();
                        break;
                    }
                }
                else
                {
                    if (!m_killSwitchManager.isRetryAfterActive())
                    {
                        killedConsumedIds.push_back(record.id);
                        deletedData[record.tenantToken]++;
                    }
                }
            }

            selectStmt.reset();

            if (selectStmt.error()) {
                LOG_ERROR("Failed to search for events to send: Database error has occurred, recreating database");
                recreate(205);
                return false;
            }

            if (consumedIds.empty()) {
                return false;
            }

            LOG_TRACE("Reserving %u event(s) {%s%s} for %u milliseconds",
                static_cast<unsigned>(consumedIds.size()), consumedIds.front().c_str(), (consumedIds.size() > 1) ? ", ..." : "", leaseTimeMs);

            std::vector<uint8_t> idList = packageIdList(consumedIds);
            if (!SqliteStatement(*m_db, m_stmtReserveEvents).execute(idList, PAL::getUtcSystemTimeMs() + leaseTimeMs)) {
                LOG_ERROR("Failed to reserve events to send: Database error occurred, recreating database");
                recreate(207);
                return false;
            }

            if (m_config.IsClockSkewEnabled() &&
                !m_clockSkewManager.GetResumeTransmissionAfterClockSkew() &&
                !consumedIds.empty())
            {
                m_clockSkewManager.GetDelta();
            }

            m_lastReadCount = static_cast<unsigned>(consumedIds.size());

            if (killedConsumedIds.size() > 0)
            {
                HttpHeaders temp;
                bool fromMemory = false;
                DeleteRecords(killedConsumedIds, temp, fromMemory);
                m_observer->OnStorageRecordsRejected(deletedData);
            }
        }
        return true;
    }

    bool OfflineStorage_SQLite::IsLastReadFromMemory()
    {
        return false;
    }
    unsigned OfflineStorage_SQLite::LastReadRecordCount()
    {
        return  m_lastReadCount;
    }

    std::vector<StorageRecord>* OfflineStorage_SQLite::GetRecords(bool shutdown, EventLatency minLatency, unsigned maxCount)
    {
        std::vector<StorageRecord>* records = new std::vector<StorageRecord>();
        StorageRecord record;

        if (shutdown)
        {
            SqliteStatement selectStmt(*m_db, m_stmtSelectEventAtShutdown);
            if (selectStmt.select(static_cast<int>(minLatency), maxCount > 0 ? maxCount : -1))
            {
                int latency;
                while (selectStmt.getRow(record.id, record.tenantToken, latency, record.timestamp, record.retryCount, record.reservedUntil, record.blob))
                {
                    record.latency = static_cast<EventLatency>(latency);
                    records->push_back(record);
                }
                selectStmt.reset();
            }
        }
        else
        {
            SqliteStatement selectStmt(*m_db, m_stmtSelectEventsMinlatency);
            if (selectStmt.select(static_cast<int>(minLatency), maxCount > 0 ? maxCount : -1))
            {
                int latency;
                while (selectStmt.getRow(record.id, record.tenantToken, latency, record.timestamp, record.retryCount, record.reservedUntil, record.blob))
                {
                    record.latency = static_cast<EventLatency>(latency);
                    records->push_back(record);
                }
                selectStmt.reset();
            }
        }
        return records;
    }


    void OfflineStorage_SQLite::DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers, bool& fromMemory)
    {
        UNREFERENCED_PARAMETER(fromMemory);
        m_killSwitchManager.handleResponse(headers);
        if (m_clockSkewManager.isWaitingForClockSkew())
        {
            m_clockSkewManager.handleResponse(headers);
        }

        if (ids.empty()) {
            return;
        }

        if (!m_db) {
            LOG_ERROR("Failed to delete %u sent event(s) {%s%s}: Database is not open",
                static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");
            return;
        }

        /* ============================================================================================================= */
        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_ERROR("Failed to DeleteRecords");
                return;
            }
#endif
            LOG_TRACE("Deleting %u sent event(s) {%s%s}...", static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");

            std::vector<uint8_t> idList = packageIdList(ids);
            if (!SqliteStatement(*m_db, m_stmtDeleteEvents_ids).execute(idList)) {
                LOG_ERROR("Failed to delete %u sent event(s) {%s%s}: Database error occurred, recreating database",
                    static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");
                recreate(302);
                return;
            }
        }
    }

    void OfflineStorage_SQLite::ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers, bool& fromMemory)
    {
        UNREFERENCED_PARAMETER(fromMemory);
        m_killSwitchManager.handleResponse(headers);
        if (m_clockSkewManager.isWaitingForClockSkew())
        {
            m_clockSkewManager.handleResponse(headers);
        }

        if (ids.empty()) {
            return;
        }
        if (!m_db) {
            LOG_ERROR("Failed to release %u event(s) {%s%s}, retry count %s: Database is not open",
                static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");
            return;
        }

        LOCKGUARD(m_lock);
        if (m_db->trylock())
        {
            LOG_TRACE("Releasing %u event(s) {%s%s}, retry count %s...",
                static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");

            std::vector<uint8_t> idList = packageIdList(ids);
            SqliteStatement releaseStmt(*m_db, m_stmtReleaseEvents_ids_retryCountDelta);
            if (!releaseStmt.execute(idList, incrementRetryCount ? 1 : 0)) {
                LOG_ERROR("Failed to release %u event(s) {%s%s}, retry count %s: Database error occurred, recreating database",
                    static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");
                recreate(403);
                return;
            }
            LOG_TRACE("Successfully released %u requested event(s), %u were not found anymore",
                releaseStmt.changes(), static_cast<unsigned>(ids.size()) - releaseStmt.changes());

            if (incrementRetryCount)
            {
                unsigned maxRetryCount = m_config.GetMaximumRetryCount();

                SqliteStatement getRowstobedeleteStmt(*m_db, m_stmtSelectEventsRetried_maxRetryCount);
                if (!getRowstobedeleteStmt.select(maxRetryCount)) {
                    LOG_ERROR("Failed to get events with exceeded retry count: Database error occurred, recreating database");
                    recreate(404);
                    return;
                }

                std::map<std::string, size_t> deletedData;
                std::string tenantToken;
                while (getRowstobedeleteStmt.getRow(tenantToken))
                {
                    deletedData[tenantToken]++;
                }

                getRowstobedeleteStmt.reset();

                SqliteStatement deleteStmt(*m_db, m_stmtDeleteEventsRetried_maxRetryCount);
                if (!deleteStmt.execute(maxRetryCount)) {
                    LOG_ERROR("Failed to delete events with exceeded retry count: Database error occurred, recreating database");
                    recreate(404);
                    return;
                }

                unsigned droppedCount = deleteStmt.changes();
                if (droppedCount > 0)
                {
                    LOG_ERROR("Deleted %u events over maximum retry count %u",
                        droppedCount, maxRetryCount);
                    m_observer->OnStorageRecordsDropped(deletedData);
                }
            }
            m_db->unlock();
        }

    }

    bool OfflineStorage_SQLite::StoreSetting(std::string const& name, std::string const& value)
    {
        if (name.empty()) {
            LOG_ERROR("Failed to set setting \"%s\": Name cannot be empty", name.c_str());
            return false;
        }

        if (!m_db) {
            LOG_ERROR("Failed to set setting \"%s\": Database is not open", name.c_str());
            return false;
        }

        if (!value.empty()) {
            if (!SqliteStatement(*m_db, m_stmtInsertSetting_name_value).execute(name, value)) {
                LOG_ERROR("Failed to set setting \"%s\": Database error occurred, recreating database", name.c_str());
                recreate(502);
                return false;
            }
        }
        else {
            if (!SqliteStatement(*m_db, m_stmtDeleteSetting_name).execute(name)) {
                LOG_ERROR("Failed to set setting \"%s\": Database error occurred, recreating database", name.c_str());
                recreate(503);
                return false;
            }
        }

        return true;
    }

    std::string OfflineStorage_SQLite::GetSetting(std::string const& name)
    {
        std::string result;

        if (name.empty()) {
            LOG_ERROR("Failed to get setting \"%s\": Name cannot be empty", name.c_str());
            return result;
        }

        if (!m_db) {
            LOG_ERROR("Failed to get setting \"%s\": Database is not open", name.c_str());
            return result;
        }

        if (m_db->trylock()) {
            SqliteStatement stmt(*m_db, m_stmtSelectSetting_name);
            if (!stmt.select(name)) {
                LOG_ERROR("Failed to get setting \"%s\": Database error occurred, recreating database", name.c_str());
                recreate(505);
            }
            stmt.getOneValue(result);
            m_db->unlock();
        }

        return result;
    }


    bool OfflineStorage_SQLite::recreate(unsigned failureCode)
    {
        LOG_WARN("DB failure %u: recreate not implemented!", failureCode);
        return false;
    }

    /*
            m_scheduledAutoCommit.cancel();
            m_isInTransaction = false;
            m_db->shutdown();

            m_observer->OnStorageFailed(toString(failureCode));

            // Try again with deletePrevious = true
            if (m_db->initialize(m_offlineStorageFileName, true)) {
                if (initializeDatabase()) {
                    m_observer->OnStorageOpened("SQLite/Clean");
                    LOG_INFO("Using configured on-disk database after deleting the existing one");
                    return true;
                }
                m_db->shutdown();
            }

            // Try again with "a private, temporary on-disk database" which "will be
            // automatically deleted as soon as the database connection is closed".
            if (m_db->initialize("", false)) {
                if (initializeDatabase()) {
                    m_observer->OnStorageOpened("SQLite/Temp");
                    LOG_INFO("Using private temporary on-disk database");
                    return true;
                }
                m_db->shutdown();
            }

            // Try again with "a private, temporary in-memory database".
            if (m_db->initialize(":memory:", false)) {
                if (initializeDatabase()) {
                    m_observer->OnStorageOpened("SQLite/Memory");
                    LOG_INFO("Using private temporary in-memory database");
                    return true;
                }
                m_db->shutdown();
            }

            m_db.reset();
            LOG_ERROR("No database could be opened");
            m_observer->OnStorageOpened("SQLite/None");
            return false;
        }
    */

    bool OfflineStorage_SQLite::initializeDatabase()
    {
        if (!SqliteStatement(*m_db,
            "PRAGMA auto_vacuum=INCREMENTAL"
        ).execute()) {
            return false;
        }

#if 1
        SqliteStatement(*m_db, "PRAGMA journal_mode=WAL").select();
        SqliteStatement(*m_db, "PRAGMA synchronous=NORMAL").select();
#else
        SqliteStatement(*m_db, "PRAGMA journal_mode=off").select();
        SqliteStatement(*m_db, "PRAGMA synchronous=NORMAL").select();
#endif

#if 0
        if (!SqliteStatement(*m_db,
            ("PRAGMA journal_size_limit=" + toString(m_config.GetOfflineStorageMaximumSizeBytes())).c_str()
        ).select()) {
            return false;
        }
#endif

        int openedDbVersion;
        {
            SqliteStatement stmt(*m_db, "PRAGMA user_version");
            if (!stmt.select() || !stmt.getRow(openedDbVersion)) { return false; }
        }

        if (openedDbVersion != CURRENT_SCHEMA_VERSION) {
            if (openedDbVersion == 0) {
                LOG_TRACE("No stored version found, assuming fresh database");
            }
            else if (openedDbVersion < CURRENT_SCHEMA_VERSION) {
                LOG_INFO("Database has older version %d, upgrading to %d",
                    openedDbVersion, CURRENT_SCHEMA_VERSION);
            }
            else {
                LOG_WARN("Database version %d is newer than current %d, erasing and replacing with new",
                    openedDbVersion, CURRENT_SCHEMA_VERSION);
                return false;
            }
            if (!SqliteStatement(*m_db,
                ("PRAGMA user_version=" + toString(CURRENT_SCHEMA_VERSION)).c_str()
            ).execute()) {
                return false;
            }
        }

        // FIXME: [MG] - migration code is missing for this scenario since we renamed property to latency!!!
        if (!SqliteStatement(*m_db,
            "CREATE TABLE IF NOT EXISTS " TABLE_NAME_EVENTS " ("
            "record_id"      " TEXT,"
            "tenant_token"   " TEXT NOT NULL,"
            "latency"        " INTEGER,"
            "persistence"    " INTEGER,"
            "timestamp"      " INTEGER,"
            "retry_count"    " INTEGER DEFAULT 0,"
            "reserved_until" " INTEGER DEFAULT 0,"
            "payload"        " BLOB,"
            "PRIMARY KEY (record_id))"
        ).execute()) {
            return false;
        }

        if (!SqliteStatement(*m_db,
            "CREATE INDEX IF NOT EXISTS k_latency_timestamp ON " TABLE_NAME_EVENTS
            " (latency DESC, persistence DESC, timestamp ASC)"
        ).execute()) {
            return false;
        }

        if (!SqliteStatement(*m_db,
            "CREATE TABLE IF NOT EXISTS " TABLE_NAME_SETTINGS " ("
            "name"  " TEXT,"
            "value" " TEXT,"
            " PRIMARY KEY (name))"
        ).execute()) {
            return false;
        }

        {
            SqliteStatement stmt(*m_db, "PRAGMA page_size");
            if (!stmt.select() || !stmt.getRow(m_pageSize)) { return false; }
        }

        // *INDENT-OFF* Uncrustify insists on adding a namespace closing comment after the closing curly brace
#pragma warning(push)
#pragma warning(disable:4296) // expression always false.
#define PREPARE_SQL(var_, stmt_) \
    if ((var_ = m_db->prepare(stmt_)) < 0) { return false; }
// *INDENT-ON*

#ifdef ENABLE_LOCKING
        PREPARE_SQL(m_stmtBeginTransaction,
            "BEGIN IMMEDIATE");
        PREPARE_SQL(m_stmtCommitTransaction,
            "COMMIT");
        PREPARE_SQL(m_stmtRollbackTransaction,
            "ROLLBACK");
#endif

        PREPARE_SQL(m_stmtGetPageCount,
            "PRAGMA page_count");
        PREPARE_SQL(m_stmtIncrementalVacuum0,
            "PRAGMA incremental_vacuum(0)");
        PREPARE_SQL(m_stmtPerTenantTrimCount,
            "SELECT tenant_token FROM " TABLE_NAME_EVENTS " ORDER BY persistence ASC, timestamp ASC LIMIT MAX(1,"
            "(SELECT COUNT(record_id) FROM " TABLE_NAME_EVENTS ")"
            "* ? / 100)");
        PREPARE_SQL(m_stmtTrimEvents_percent,
            "DELETE FROM " TABLE_NAME_EVENTS " WHERE record_id IN ("
            "SELECT record_id FROM " TABLE_NAME_EVENTS " ORDER BY persistence ASC, timestamp ASC LIMIT MAX(1,"
            "(SELECT COUNT(record_id) FROM " TABLE_NAME_EVENTS ")"
            "* ? / 100)"
            ")");
        PREPARE_SQL(m_stmtDeleteEvents_ids,
            SQL_SUPPLY_PACKAGED_IDS
            "DELETE FROM " TABLE_NAME_EVENTS " WHERE record_id IN ids");
        PREPARE_SQL(m_stmtReleaseExpiredEvents,
            "UPDATE " TABLE_NAME_EVENTS
            " SET reserved_until=0, retry_count=retry_count+1"
            " WHERE reserved_until<>0 AND reserved_until<=?");
        PREPARE_SQL(m_stmtSelectEvents,
            "SELECT record_id,tenant_token,latency,timestamp,retry_count,reserved_until,payload"
            " FROM " TABLE_NAME_EVENTS
            " WHERE latency>=? AND reserved_until=0"
            " ORDER BY latency DESC,persistence DESC, timestamp ASC LIMIT ?");
        PREPARE_SQL(m_stmtSelectEventAtShutdown,
            "SELECT record_id,tenant_token,latency,timestamp,retry_count,reserved_until,payload"
            " FROM " TABLE_NAME_EVENTS
            " WHERE latency>=?"
            " ORDER BY latency DESC,persistence DESC, timestamp ASC LIMIT ?");
        PREPARE_SQL(m_stmtSelectEventsMinlatency,
            "SELECT record_id,tenant_token,latency,timestamp,retry_count,reserved_until,payload"
            " FROM " TABLE_NAME_EVENTS
            " WHERE latency=(SELECT MIN(latency) FROM " TABLE_NAME_EVENTS " WHERE reserved_until=0 AND latency>=?) AND reserved_until=0"
            " ORDER BY timestamp ASC LIMIT ?");

        PREPARE_SQL(m_stmtReserveEvents,
            SQL_SUPPLY_PACKAGED_IDS
            "UPDATE " TABLE_NAME_EVENTS
            " SET reserved_until=?"
            " WHERE record_id IN ids");
        PREPARE_SQL(m_stmtReleaseEvents_ids_retryCountDelta,
            SQL_SUPPLY_PACKAGED_IDS
            "UPDATE " TABLE_NAME_EVENTS
            " SET reserved_until=0, retry_count=retry_count+?"
            " WHERE record_id IN ids AND reserved_until>0");
        PREPARE_SQL(m_stmtSelectEventsRetried_maxRetryCount,
            "SELECT tenant_token FROM " TABLE_NAME_EVENTS
            " WHERE retry_count>?");
        PREPARE_SQL(m_stmtDeleteEventsRetried_maxRetryCount,
            "DELETE FROM " TABLE_NAME_EVENTS
            " WHERE retry_count>?");
        PREPARE_SQL(m_stmtInsertEvent_id_tenant_prio_ts_data,
            "REPLACE INTO " TABLE_NAME_EVENTS " (record_id,tenant_token,latency,persistence,timestamp,payload) VALUES (?,?,?,?,?,?)");
        PREPARE_SQL(m_stmtInsertSetting_name_value,
            "REPLACE INTO " TABLE_NAME_SETTINGS " (name,value) VALUES (?,?)");
        PREPARE_SQL(m_stmtDeleteSetting_name,
            "DELETE FROM " TABLE_NAME_SETTINGS " WHERE name=?");
        PREPARE_SQL(m_stmtSelectSetting_name,
            "SELECT value FROM " TABLE_NAME_SETTINGS " WHERE name=?");

#undef PREPARE_SQL
#pragma warning(pop)

        m_currentlyAddedBytes = 0;
        m_isInTransaction = false;
        return true;
    }

#if 0
    bool OfflineStorage_SQLite::beginIfNotInTransaction()
    {
        LOCKGUARD(m_lock);
        if (!m_isInTransaction)
        {
            m_isInTransaction = true;
            SqliteStatement stmt(*m_db, m_stmtBeginTransaction);
            if (stmt.execute()) {
                LOG_INFO("=== [%p] Transaction started", stmt.handle());
                return true;
            }
            LOG_ERROR("=== [%p] Transaction failed", stmt.handle());
            m_isInTransaction = false;
            return false;
        }
        return true;
    }

    bool OfflineStorage_SQLite::commitIfInTransaction()
    {
        LOCKGUARD(m_lock);
        if (m_isInTransaction) {
            m_scheduledAutoCommit.cancel();
            SqliteStatement stmt(*m_db, m_stmtCommitTransaction);
            if (stmt.execute()) {
                LOG_INFO("=== [%p] Transaction committed in %u ms", stmt.handle(), stmt.duration());
                m_isInTransaction = false;
                return true;
            }
            else {
                LOG_ERROR("=== [%p] Failed to commit database transaction", stmt.handle());
                return false;
            }
        }
        return true;
    }

    bool OfflineStorage_SQLite::rollbackIfInTransaction()
    {
        if (!m_isInTransaction) {
            return true;
        }

        SqliteStatement stmt(*m_db, m_stmtRollbackTransaction);
        if (stmt.execute()) {
            LOG_INFO("=== [%p] Transaction rolled back in %u ms", stmt.handle(), stmt.duration());
            m_isInTransaction = false;
            return true;
        }
        else {
            LOG_ERROR("=== [%p] Failed to rollback database transaction", stmt.handle());
            return false;
        }
    }

    void OfflineStorage_SQLite::scheduleAutoCommitTransaction()
    {
        autoCommitTransaction();
        /*
                if (!m_scheduledAutoCommit) {
                    unsigned delayMs = 2500;
                    LOG_TRACE("Scheduling auto commit in %u msec", delayMs);
                    m_scheduledAutoCommit = PAL::scheduleOnWorkerThread(delayMs, this, &OfflineStorage_SQLite::autoCommitTransaction);
                }
         */
    }

    void OfflineStorage_SQLite::autoCommitTransaction()
    {
        m_scheduledAutoCommit.reset();
        if (!commitIfInTransaction()) {
            LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
            recreate(601);
        }
    }
#endif

    // TODO: [MG] - for on-disk database this has to be replaced by filesize check
    size_t OfflineStorage_SQLite::GetSize()
    {
        if (!m_db) {
            LOG_ERROR("Failed to get DB size: database is not open");
            return 0;
        }

        LOCKGUARD(m_lock);
        unsigned pageCount;
        SqliteStatement pageCountStmt(*m_db, m_stmtGetPageCount);
        while (!pageCountStmt.select())
        {
            PAL::sleep(100);
        }
        pageCountStmt.getRow(pageCount);
        pageCountStmt.reset();
        return pageCount * m_pageSize;
    }

    bool OfflineStorage_SQLite::trimDbIfNeeded(size_t justAddedBytes)
    {
        if (!m_db) {
            LOG_ERROR("Failed to trim: database is not open");
            return 0;
        }

        m_currentlyAddedBytes += justAddedBytes;
        if (m_currentlyAddedBytes < 10240) {
            return true;
        }
        m_currentlyAddedBytes = 0;

        LOCKGUARD(m_lock);
        {
#ifdef ENABLE_LOCKING
            DbTransaction transaction(m_db.get());
            if (!transaction.locked)
            {
                LOG_WARN("Failed to trim database");
                return false;
            }
#endif
            unsigned pageCount;
            SqliteStatement pageCountStmt(*m_db, m_stmtGetPageCount);
            if (!pageCountStmt.select() || !pageCountStmt.getRow(pageCount)) {
                LOG_ERROR("Failed to query database size: Database error has occurred, recreating database");
                return false;
            }
            pageCountStmt.reset();

            unsigned previousDbSize = pageCount * m_pageSize;

            //check if Application needs to be notified
            if (previousDbSize > m_DbSizeNotificationLimit && !m_isStorageFullNotificationSend)
            {
                DebugEvent evt;
                evt.type = DebugEventType::EVT_STORAGE_FULL;
                evt.param1 = 2;
                m_logManager.DispatchEvent(evt);
                m_isStorageFullNotificationSend = true;
            }

            unsigned maxSize = m_config.GetOfflineStorageMaximumSizeBytes();
            if (previousDbSize <= maxSize) {
                LOG_TRACE("Database size (%u bytes) is below the maximum allowed size (%u bytes), no trimming necessary",
                    previousDbSize, maxSize);
                return true;
            }

            m_isStorageFullNotificationSend = false; // to be notified again
            unsigned pct = m_config.GetOfflineStorageResizeThresholdPct();
            LOG_ERROR("Database size (%u bytes) exceeds the maximum allowed size (%u bytes), trimming %u%% off...",
                previousDbSize, maxSize, pct);

            SqliteStatement getRowstobedeleteStmt(*m_db, m_stmtPerTenantTrimCount);
            if (!getRowstobedeleteStmt.select(pct)) {
                LOG_ERROR("Failed to get infor on events to be deleted to reduce size: Database error has occurred, recreating database");
                return false;
            }

            std::map<std::string, size_t> deletedData;
            std::string tenantToken;
            while (getRowstobedeleteStmt.getRow(tenantToken))
            {
                deletedData[tenantToken]++;
            }

            getRowstobedeleteStmt.reset();

            SqliteStatement deleteStmt(*m_db, m_stmtTrimEvents_percent);
            if (!deleteStmt.execute(pct)) {
                LOG_ERROR("Failed to delete events to reduce size: Database error has occurred, recreating database");
                return false;
            }

            unsigned droppedCount = deleteStmt.changes();
            LOG_ERROR("Deleted %u event(s) in %u ms",
                droppedCount, deleteStmt.duration());
            m_observer->OnStorageTrimmed(deletedData);

            SqliteStatement trimStmt(*m_db, m_stmtIncrementalVacuum0);
            if (!trimStmt.select()) {
                LOG_ERROR("Failed to trim free pages to reduce size: Database error has occurred, recreating database");
                return false;
            }
            while (trimStmt.getRow()) {
            }
            if (trimStmt.error()) {
                LOG_ERROR("Failed to trim free pages to reduce size: Database error has occurred, recreating database");
                return false;
            }

            unsigned pageCount2;
            if (!pageCountStmt.select() || !pageCountStmt.getRow(pageCount2)) {
                LOG_ERROR("Failed to query database size: Database error has occurred, recreating database");
                return false;
            }

            unsigned newDbSize = pageCount2 * m_pageSize;
            LOG_TRACE("Trimmed %d bytes in %u ms",
                previousDbSize - newDbSize, trimStmt.duration());

            if (newDbSize > maxSize) {
                LOG_ERROR("Failed to trim database: previous size %u, after trimming %u, still more than limit %u bytes, recreating database",
                    previousDbSize, newDbSize, maxSize);
                return false;
            }
        }
        return true;
    }

    bool OfflineStorage_SQLite::ResizeDb()
    {
        if (!m_db) {
            LOG_ERROR("Failed to resize: database is not open");
            return 0;
        }

        SqliteStatement trimStmt(*m_db, m_stmtIncrementalVacuum0);
        if (!trimStmt.select()) {
            LOG_ERROR("Failed to trim free pages to reduce size: Database error has occurred, recreating database");
            return false;
        }
        while (trimStmt.getRow()) {
        }
        if (trimStmt.error()) {
            LOG_ERROR("Failed to trim free pages to reduce size: Database error has occurred, recreating database");
            return false;
        }
        return true;
    }

    std::vector<uint8_t> OfflineStorage_SQLite::packageIdList(std::vector<std::string> const& ids)
    {
        size_t size = std::accumulate(ids.cbegin(), ids.cend(), size_t(0), [](size_t sum, std::string const& id) -> size_t {
            return sum + id.length() + 1;
        });

        std::vector<uint8_t> result;
        result.reserve(size);

        for (std::string const& id : ids) {
            uint8_t const* ptr = reinterpret_cast<uint8_t const*>(id.c_str());
            result.insert(result.end(), ptr, ptr + id.size() + 1);
        }

        return result;
    }


} ARIASDK_NS_END
