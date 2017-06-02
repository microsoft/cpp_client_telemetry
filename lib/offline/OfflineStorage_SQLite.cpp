// Copyright (c) Microsoft. All rights reserved.

#include "OfflineStorage_SQLite.hpp"
#include "LogManager.hpp"
#include "SQLiteWrapper.hpp"
#include "utils/Utils.hpp"
#include <algorithm>
#include <numeric>
#include <set>

namespace ARIASDK_NS_BEGIN {


ARIASDK_LOG_INST_COMPONENT_CLASS(OfflineStorage_SQLite, "AriaSDK.Storage", "Aria telemetry client - OfflineStorage_SQLite class");

static int const CURRENT_SCHEMA_VERSION = 1;
#define TABLE_NAME_EVENTS   "events"
#define TABLE_NAME_SETTINGS "settings"

OfflineStorage_SQLite::OfflineStorage_SQLite(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig)
    : m_databasePath(configuration.cacheFilePath),
    m_runtimeConfig(runtimeConfig),
    m_skipInitAndShutdown(configuration.skipSqliteInitAndShutdown),
    m_killSwitchManager(),
    m_clockSkewManager()
{
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

    ARIASDK_LOG_DETAIL("Initializing offline storage");

    if (m_db->initialize(m_databasePath, false) && initializeDatabase()) {
        ARIASDK_LOG_INFO("Using configured on-disk database");
        m_observer->OnStorageOpened("SQLite/Default");
        return;
    }

    recreate(1);
}

void OfflineStorage_SQLite::Shutdown()
{
    ARIASDK_LOG_DETAIL("Shutting down offline storage");

    if (m_db) {
        if (!commitIfInTransaction()) {
            ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, but ignored during shutdown");
        }
        m_db->shutdown();
        m_db.reset();
    }
}

bool OfflineStorage_SQLite::StoreRecord(StorageRecord const& record)
{
    if (record.id.empty() || record.tenantToken.empty() || static_cast<int>(record.priority) < 0 || record.timestamp <= 0) {
        ARIASDK_LOG_ERROR("Failed to store event %s:%s: Invalid parameters",
            tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
        return false;
    }
    if (!m_db) {
        ARIASDK_LOG_ERROR("Failed to store event %s:%s: Database is not open",
            tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
        return false;
    }

    for (int retry = 0; retry < 2; retry++) {
        ARIASDK_LOG_DETAIL("Storing event %s:%s to offline storage",
            tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());

        if (beginIfNotInTransaction() &&
            SqliteStatement(*m_db, m_stmtInsertEvent_id_tenant_prio_ts_data).execute(record.id, record.tenantToken, static_cast<int>(record.priority), record.timestamp, record.blob))
        {
            scheduleAutoCommitTransaction();
            if (trimDbIfNeeded(/* empiric estimate */ 32 + 2 * record.id.size() + record.tenantToken.size() + record.blob.size())) {
                return true;
            }
        }

        ARIASDK_LOG_ERROR("Failed to store event %s:%s: Database error occurred, recreating database",
            tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
        if (!recreate(101)) {
            return false;
        }
    }

    ARIASDK_LOG_ERROR("Failed to store event %s:%s: Database error even after retries",
        tenantTokenToId(record.tenantToken).c_str(), record.id.c_str());
    return false;
}

bool OfflineStorage_SQLite::GetAndReserveRecords(std::function<bool(StorageRecord&&)> const& consumer, unsigned leaseTimeMs, EventPriority minPriority /* =EventPriority_Unspecified */, unsigned maxCount /* =0 */)
{
    if (!m_db) {
        ARIASDK_LOG_ERROR("Failed to retrieve events to send: Database is not open");
        return false;
    }

    ARIASDK_LOG_DETAIL("Retrieving max. %u%s events of priority at least %d (%s)",
        maxCount, (maxCount > 0) ? "" : " (unlimited)", minPriority, priorityToStr(static_cast<EventPriority>(minPriority)));

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
        recreate(201);
        return false;
    }

    SqliteStatement releaseStmt(*m_db, m_stmtReleaseExpiredEvents);
    if (!releaseStmt.execute(PAL::getUtcSystemTimeMs())) {
        ARIASDK_LOG_ERROR("Failed to release expired reserved events: Database error occurred, recreating database");
        recreate(202);
        return false;
    }
    if (releaseStmt.changes() > 0) {
        ARIASDK_LOG_DETAIL("Released %u expired reserved events",
            static_cast<unsigned>(releaseStmt.changes()));
    }

    if (!beginIfNotInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to retrieve events to send: Database error occurred, recreating database");
        recreate(203);
        return false;
    }
    
    if (m_runtimeConfig.IsClockSkewEnabled() && m_clockSkewManager.isWaitingForClockSkew())
    {
        return false;
    }
    SqliteStatement selectStmt(*m_db, m_stmtSelectEvents);
    if (!selectStmt.select(static_cast<int>(minPriority), maxCount > 0 ? maxCount : -1)) {
        ARIASDK_LOG_ERROR("Failed to retrieve events to send: Database error occurred, recreating database");
        recreate(204);
        return false;
    }

    std::vector<StorageRecordId> consumedIds;
    StorageRecord record;
    int priority;
    while (selectStmt.getRow(record.id, record.tenantToken, priority, record.timestamp, record.retryCount, record.reservedUntil, record.blob)) {
        if (priority < EventPriority_Off || priority > EventPriority_Immediate) {
            record.priority = EventPriority_Normal;
        } else {
            record.priority = static_cast<EventPriority>(priority);
        }
        // The record ID needs to be saved before std::move() below.
        if (!m_killSwitchManager.isTokenBlocked(record.tenantToken))
        {			
            consumedIds.push_back(record.id);
        }
        
        if (!consumer(std::move(record))) {
            consumedIds.pop_back();
            selectStmt.reset();
            break;
        }
    }
    if (selectStmt.error()) {
        ARIASDK_LOG_ERROR("Failed to search for events to send: Database error has occurred, recreating database");
        recreate(205);
        return false;
    }

    if (consumedIds.empty()) {
        if (!rollbackIfInTransaction()) {
            ARIASDK_LOG_ERROR("Failed to rollback event search: Database error has occurred, recreating database");
            recreate(206);
            return false;
        }
        return true;
    }

    ARIASDK_LOG_DETAIL("Reserving %u event(s) {%s%s} for %u milliseconds",
        static_cast<unsigned>(consumedIds.size()), consumedIds.front().c_str(), (consumedIds.size() > 1) ? ", ..." : "", leaseTimeMs);
    std::vector<uint8_t> idList = OfflineStorage_SQLite::packageIdList(consumedIds);
    if (!SqliteStatement(*m_db, m_stmtReserveEvents).execute(idList, PAL::getUtcSystemTimeMs() + leaseTimeMs)) {
        ARIASDK_LOG_ERROR("Failed to reserve events to send: Database error occurred, recreating database");
        recreate(207);
        return false;
    }

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to reserve events to send: Database error occurred, recreating database");
        recreate(208);
        return false;
    }

    if (m_runtimeConfig.IsClockSkewEnabled() &&
        !m_clockSkewManager.GetResumeTransmissionAfterClockSkew() &&
        !consumedIds.empty())
    {
        m_clockSkewManager.GetDelta();
    }

    return true;
}

void OfflineStorage_SQLite::DeleteRecords(std::vector<StorageRecordId> const& ids, HttpHeaders headers)
{
    m_killSwitchManager.handleResponse(headers);
    if (m_clockSkewManager.isWaitingForClockSkew())
    {
        m_clockSkewManager.handleResponse(headers);
    }

    if (ids.empty()) {
        return;
    }
    if (!m_db) {
        ARIASDK_LOG_ERROR("Failed to delete %u sent event(s) {%s%s}: Database is not open",
            static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");
        return;
    }

    ARIASDK_LOG_DETAIL("Deleting %u sent event(s) {%s%s}...",
        static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
        recreate(301);
        return;
    }

    std::vector<uint8_t> idList = OfflineStorage_SQLite::packageIdList(ids);
    if (!SqliteStatement(*m_db, m_stmtDeleteEvents_ids).execute(idList)) {
        ARIASDK_LOG_ERROR("Failed to delete %u sent event(s) {%s%s}: Database error occurred, recreating database",
            static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "");
        recreate(302);
        return;
    }
}

void OfflineStorage_SQLite::ReleaseRecords(std::vector<StorageRecordId> const& ids, bool incrementRetryCount, HttpHeaders headers)
{
    m_killSwitchManager.handleResponse(headers);
    if (m_clockSkewManager.isWaitingForClockSkew())
    {
        m_clockSkewManager.handleResponse(headers);
    }

    if (ids.empty()) {
        return;
    }
    if (!m_db) {
        ARIASDK_LOG_ERROR("Failed to release %u event(s) {%s%s}, retry count %s: Database is not open",
            static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");
        return;
    }

    ARIASDK_LOG_DETAIL("Releasing %u event(s) {%s%s}, retry count %s...",
        static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
        recreate(401);
        return;
    }

    if (!beginIfNotInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to release %u event(s) {%s%s}, retry count %s: Database error occurred, recreating database",
            static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");
        recreate(402);
        return;
    }

    std::vector<uint8_t> idList = OfflineStorage_SQLite::packageIdList(ids);
    SqliteStatement releaseStmt(*m_db, m_stmtReleaseEvents_ids_retryCountDelta);
    if (!releaseStmt.execute(idList, incrementRetryCount ? 1 : 0)) {
        ARIASDK_LOG_ERROR("Failed to release %u event(s) {%s%s}, retry count %s: Database error occurred, recreating database",
            static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");
        recreate(403);
        return;
    }
    ARIASDK_LOG_DETAIL("Successfully released %u requested event(s), %u were not found anymore",
        releaseStmt.changes(), static_cast<unsigned>(ids.size()) - releaseStmt.changes());

    if (incrementRetryCount) {
        unsigned maxRetryCount = m_runtimeConfig.GetMaximumRetryCount();

        SqliteStatement deleteStmt(*m_db, m_stmtDeleteEventsRetried_maxRetryCount);
        if (!deleteStmt.execute(maxRetryCount)) {
            ARIASDK_LOG_ERROR("Failed to delete events with exceeded retry count: Database error occurred, recreating database");
            recreate(404);
            return;
        }

        unsigned droppedCount = deleteStmt.changes();
        if (droppedCount > 0)
        {
            ARIASDK_LOG_ERROR("Deleted %u events over maximum retry count %u",
                droppedCount, maxRetryCount);
            m_observer->OnStorageRecordsDropped(droppedCount);
        }
    }

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to release %u event(s) {%s%s}, retry count %s: Database error occurred, recreating database",
            static_cast<unsigned>(ids.size()), ids.front().c_str(), (ids.size() > 1) ? ", ..." : "", incrementRetryCount ? "+1" : "not changed");
        recreate(405);
    }
}

bool OfflineStorage_SQLite::StoreSetting(std::string const& name, std::string const& value)
{
    if (name.empty()) {
        ARIASDK_LOG_ERROR("Failed to set setting \"%s\": Name cannot be empty", name.c_str());
        return false;
    }
    if (!m_db) {
        ARIASDK_LOG_ERROR("Failed to set setting \"%s\": Database is not open", name.c_str());
        return false;
    }

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
        recreate(501);
        return false;
    }

    if (!value.empty()) {
        if (!SqliteStatement(*m_db, m_stmtInsertSetting_name_value).execute(name, value)) {
            ARIASDK_LOG_ERROR("Failed to set setting \"%s\": Database error occurred, recreating database", name.c_str());
            recreate(502);
            return false;
        }
    } else {
        if (!SqliteStatement(*m_db, m_stmtDeleteSetting_name).execute(name)) {
            ARIASDK_LOG_ERROR("Failed to set setting \"%s\": Database error occurred, recreating database", name.c_str());
            recreate(503);
            return false;
        }
    }

    return true;
}

std::string OfflineStorage_SQLite::GetSetting(std::string const& name)
{
    if (name.empty()) {
        ARIASDK_LOG_ERROR("Failed to get setting \"%s\": Name cannot be empty", name.c_str());
        return std::string();
    }
    if (!m_db) {
        ARIASDK_LOG_ERROR("Failed to get setting \"%s\": Database is not open", name.c_str());
        return std::string();
    }

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
        recreate(504);
        return std::string();
    }

    SqliteStatement stmt(*m_db, m_stmtSelectSetting_name);
    if (!stmt.select(name)) {
        ARIASDK_LOG_ERROR("Failed to get setting \"%s\": Database error occurred, recreating database", name.c_str());
        recreate(505);
        return std::string();
    }

    std::string result;
    stmt.getOneValue(result);
    return result;
}

bool OfflineStorage_SQLite::recreate(unsigned failureCode)
{
    m_scheduledAutoCommit.cancel();
    m_isInTransaction = false;
    m_db->shutdown();

    m_observer->OnStorageFailed(toString(failureCode));

    // Try again with deletePrevious = true
    if (m_db->initialize(m_databasePath, true)) {
        if (initializeDatabase()) {
            m_observer->OnStorageOpened("SQLite/Clean");
            ARIASDK_LOG_INFO("Using configured on-disk database after deleting the existing one");
            return true;
        }
        m_db->shutdown();
    }

    // Try again with "a private, temporary on-disk database" which "will be
    // automatically deleted as soon as the database connection is closed".
    if (m_db->initialize("", false)) {
        if (initializeDatabase()) {
            m_observer->OnStorageOpened("SQLite/Temp");
            ARIASDK_LOG_INFO("Using private temporary on-disk database");
            return true;
        }
        m_db->shutdown();
    }

    // Try again with "a private, temporary in-memory database".
    if (m_db->initialize(":memory:", false)) {
        if (initializeDatabase()) {
            m_observer->OnStorageOpened("SQLite/Memory");
            ARIASDK_LOG_INFO("Using private temporary in-memory database");
            return true;
        }
        m_db->shutdown();
    }

    m_db.reset();
    ARIASDK_LOG_ERROR("No database could be opened");
    m_observer->OnStorageOpened("SQLite/None");
    return false;
}

bool OfflineStorage_SQLite::initializeDatabase()
{
    if (!SqliteStatement(*m_db,
        "PRAGMA auto_vacuum=INCREMENTAL"
        ).execute()) { return false; }

    if (!SqliteStatement(*m_db,
        "PRAGMA journal_mode=WAL"
        ).select()) { return false; }

    if (!SqliteStatement(*m_db,
        ("PRAGMA journal_size_limit=" + toString(m_runtimeConfig.GetOfflineStorageMaximumSizeBytes())).c_str()
        ).select()) { return false; }

    int openedDbVersion;
    {
        SqliteStatement stmt(*m_db, "PRAGMA user_version");
        if (!stmt.select() || !stmt.getRow(openedDbVersion)) { return false; }
    }

    if (openedDbVersion != CURRENT_SCHEMA_VERSION) {
        if (openedDbVersion == 0) {
            ARIASDK_LOG_DETAIL("No stored version found, assuming fresh database");
        } else if (openedDbVersion < CURRENT_SCHEMA_VERSION) {
            ARIASDK_LOG_INFO("Database has older version %d, upgrading to %d",
                openedDbVersion, CURRENT_SCHEMA_VERSION);
        } else {
            ARIASDK_LOG_WARNING("Database version %d is newer than current %d, erasing and replacing with new",
                openedDbVersion, CURRENT_SCHEMA_VERSION);
            return false;
        }
        if (!SqliteStatement(*m_db,
            ("PRAGMA user_version=" + toString(CURRENT_SCHEMA_VERSION)).c_str()
            ).execute()) { return false; }
    }

    if (!SqliteStatement(*m_db,
        "CREATE TABLE IF NOT EXISTS " TABLE_NAME_EVENTS " ("
        "record_id"      " TEXT,"
        "tenant_token"   " TEXT NOT NULL,"
        "priority"       " INTEGER,"
        "timestamp"      " INTEGER,"
        "retry_count"    " INTEGER DEFAULT 0,"
        "reserved_until" " INTEGER DEFAULT 0,"
        "payload"        " BLOB,"
        " PRIMARY KEY (record_id))"
        ).execute()) { return false; }

    if (!SqliteStatement(*m_db,
        "CREATE INDEX IF NOT EXISTS k_priority_timestamp ON " TABLE_NAME_EVENTS
        " (priority DESC, timestamp ASC)"
        ).execute()) { return false; }

    if (!SqliteStatement(*m_db,
        "CREATE TABLE IF NOT EXISTS " TABLE_NAME_SETTINGS " ("
        "name"  " TEXT,"
        "value" " TEXT,"
        " PRIMARY KEY (name))"
        ).execute()) { return false; }

    {
        SqliteStatement stmt(*m_db, "PRAGMA page_size");
        if (!stmt.select() || !stmt.getRow(m_pageSize)) { return false; }
    }

// *INDENT-OFF* Uncrustify insists on adding a namespace closing comment after the closing curly brace
#define PREPARE_SQL(var_, stmt_) \
    if ((var_ = m_db->prepare(stmt_)) < 0) { return false; }
// *INDENT-ON*

    PREPARE_SQL(m_stmtBeginTransaction,
        "BEGIN IMMEDIATE");
    PREPARE_SQL(m_stmtCommitTransaction,
        "COMMIT");
    PREPARE_SQL(m_stmtRollbackTransaction,
        "ROLLBACK");
    PREPARE_SQL(m_stmtGetPageCount,
        "PRAGMA page_count");
    PREPARE_SQL(m_stmtIncrementalVacuum0,
        "PRAGMA incremental_vacuum(0)");
    PREPARE_SQL(m_stmtTrimEvents_percent,
        "DELETE FROM " TABLE_NAME_EVENTS " WHERE record_id IN ("
        "SELECT record_id FROM " TABLE_NAME_EVENTS " ORDER BY priority ASC, timestamp ASC LIMIT MAX(1,"
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
        "SELECT record_id,tenant_token,priority,timestamp,retry_count,reserved_until,payload"
        " FROM " TABLE_NAME_EVENTS
        " WHERE priority=(SELECT MAX(priority) FROM " TABLE_NAME_EVENTS " WHERE reserved_until=0 AND priority>=?) AND reserved_until=0"
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
    PREPARE_SQL(m_stmtDeleteEventsRetried_maxRetryCount,
        "DELETE FROM " TABLE_NAME_EVENTS
        " WHERE retry_count>?");
    PREPARE_SQL(m_stmtInsertEvent_id_tenant_prio_ts_data,
        "REPLACE INTO " TABLE_NAME_EVENTS " (record_id,tenant_token,priority,timestamp,payload) VALUES (?,?,?,?,?)");
    PREPARE_SQL(m_stmtInsertSetting_name_value,
        "REPLACE INTO " TABLE_NAME_SETTINGS " (name,value) VALUES (?,?)");
    PREPARE_SQL(m_stmtDeleteSetting_name,
        "DELETE FROM " TABLE_NAME_SETTINGS " WHERE name=?");
    PREPARE_SQL(m_stmtSelectSetting_name,
        "SELECT value FROM " TABLE_NAME_SETTINGS " WHERE name=?");

#undef PREPARE_SQL

    m_currentlyAddedBytes = 0;
    m_isInTransaction = false;
    return true;
}

bool OfflineStorage_SQLite::beginIfNotInTransaction()
{
    if (m_isInTransaction) {
        return true;
    }

    SqliteStatement stmt(*m_db, m_stmtBeginTransaction);
    if (stmt.execute()) {
        ARIASDK_LOG_DETAIL("Transaction started");
        m_isInTransaction = true;
        return true;
    } else {
        ARIASDK_LOG_ERROR("Failed to start transaction");
        return false;
    }
}

bool OfflineStorage_SQLite::commitIfInTransaction()
{
    if (!m_isInTransaction) {
        return true;
    }

    m_scheduledAutoCommit.cancel();

    SqliteStatement stmt(*m_db, m_stmtCommitTransaction);
    if (stmt.execute()) {
        ARIASDK_LOG_DETAIL("Transaction committed in %u ms", stmt.duration());
        m_isInTransaction = false;
        return true;
    } else {
        ARIASDK_LOG_ERROR("Failed to commit database transaction");
        return false;
    }
}

bool OfflineStorage_SQLite::rollbackIfInTransaction()
{
    if (!m_isInTransaction) {
        return true;
    }

    SqliteStatement stmt(*m_db, m_stmtRollbackTransaction);
    if (stmt.execute()) {
        ARIASDK_LOG_DETAIL("Transaction rolled back in %u ms", stmt.duration());
        m_isInTransaction = false;
        return true;
    } else {
        ARIASDK_LOG_ERROR("Failed to rollback database transaction");
        return false;
    }
}

void OfflineStorage_SQLite::scheduleAutoCommitTransaction()
{
    if (!m_scheduledAutoCommit) {
        unsigned delayMs = 2500;
        ARIASDK_LOG_DETAIL("Scheduling auto commit in %u msec", delayMs);
        m_scheduledAutoCommit = PAL::scheduleOnWorkerThread(delayMs, this, &OfflineStorage_SQLite::autoCommitTransaction);
    }
}

void OfflineStorage_SQLite::autoCommitTransaction()
{
    m_scheduledAutoCommit.reset();
    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
        recreate(601);
    }
}

bool OfflineStorage_SQLite::trimDbIfNeeded(size_t justAddedBytes)
{
    m_currentlyAddedBytes += justAddedBytes;
    if (m_currentlyAddedBytes < 10240) {
        return true;
    }
    m_currentlyAddedBytes = 0;

    unsigned pageCount;
    SqliteStatement pageCountStmt(*m_db, m_stmtGetPageCount);
    if (!pageCountStmt.select() || !pageCountStmt.getRow(pageCount)) {
        ARIASDK_LOG_ERROR("Failed to query database size: Database error has occurred, recreating database");
        return false;
    }
    pageCountStmt.reset();

    unsigned previousDbSize = pageCount * m_pageSize;
    unsigned maxSize = m_runtimeConfig.GetOfflineStorageMaximumSizeBytes();
    if (previousDbSize <= maxSize) {
        ARIASDK_LOG_DETAIL("Database size (%u bytes) is below the maximum allowed size (%u bytes), no trimming necessary",
            previousDbSize, maxSize);
        return true;
    }

    unsigned pct = m_runtimeConfig.GetOfflineStorageResizeThresholdPct();
    ARIASDK_LOG_ERROR("Database size (%u bytes) exceeds the maximum allowed size (%u bytes), trimming %u%% off...",
        previousDbSize, maxSize, pct);

    if (!commitIfInTransaction()) {
        ARIASDK_LOG_ERROR("Failed to commit queued events: Database error has occurred, recreating database");
        return false;
    }

    SqliteStatement deleteStmt(*m_db, m_stmtTrimEvents_percent);
    if (!deleteStmt.execute(pct)) {
        ARIASDK_LOG_ERROR("Failed to delete events to reduce size: Database error has occurred, recreating database");
        return false;
    }

    unsigned droppedCount = deleteStmt.changes();
    ARIASDK_LOG_ERROR("Deleted %u event(s) in %u ms",
        droppedCount, deleteStmt.duration());
    m_observer->OnStorageTrimmed(droppedCount);

    SqliteStatement trimStmt(*m_db, m_stmtIncrementalVacuum0);
    if (!trimStmt.select()) {
        ARIASDK_LOG_ERROR("Failed to trim free pages to reduce size: Database error has occurred, recreating database");
        return false;
    }
    while (trimStmt.getRow()) {
    }
    if (trimStmt.error()) {
        ARIASDK_LOG_ERROR("Failed to trim free pages to reduce size: Database error has occurred, recreating database");
        return false;
    }

    unsigned pageCount2;
    if (!pageCountStmt.select() || !pageCountStmt.getRow(pageCount2)) {
        ARIASDK_LOG_ERROR("Failed to query database size: Database error has occurred, recreating database");
        return false;
    }

    unsigned newDbSize = pageCount2 * m_pageSize;
    ARIASDK_LOG_DETAIL("Trimmed %d bytes in %u ms",
        previousDbSize - newDbSize, trimStmt.duration());

    if (newDbSize > maxSize) {
        ARIASDK_LOG_ERROR("Failed to trim database: previous size %u, after trimming %u, still more than limit %u bytes, recreating database",
            previousDbSize, newDbSize, maxSize);
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
