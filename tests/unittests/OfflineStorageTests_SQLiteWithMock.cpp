// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "common/MockISqlite3Proxy.hpp"
#include "offline/OfflineStorage_SQLite.hpp"
#include <stdio.h>
#include <regex>
#include "pal/PAL_Win32.hpp"
#include "../../sqlite/sqlite3.h"

using namespace testing;
using namespace ARIASDK_NS;
using namespace ARIASDK_NS::PAL;


// Do not try use anything else than .* in the regex, it will fail because of buggy implementations:
// http://stackoverflow.com/questions/12530406/is-gcc-4-8-or-earlier-buggy-about-regular-expressions
MATCHER_P2(PreparedStatement, testClass, recipeRegex, "")
{
    UNREFERENCED_PARAMETER(result_listener);
    return testClass->statements.find(arg) != testClass->statements.end() &&
           std::regex_match(testClass->statements[arg].recipe, std::regex(recipeRegex));
}


class TestRecordConsumer {
  public:
    operator std::function<bool(StorageRecord&&)>()
    {
        // *INDENT-OFF* Uncrustify mangles this lambda's syntax a lot
        return [=](StorageRecord&& record) -> bool {
            if (records.size() >= maxCount) {
                return false;
            }
            records.push_back(record);
            return true;
        };
        // *INDENT-ON*
    }
    size_t                     maxCount = SIZE_MAX;
    std::vector<StorageRecord> records;
};


class OfflineStorage_SQLite4Test : public OfflineStorage_SQLite {
  public:
    OfflineStorage_SQLite4Test(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig)
      : OfflineStorage_SQLite(configuration, runtimeConfig)
    {
    }

    void setSkipInitAndShutdown(bool state)
    {
        m_skipInitAndShutdown = state;
    }

    void setInTransaction(bool state)
    {
        m_isInTransaction = state;
    }

    using OfflineStorage_SQLite::initializeDatabase;
    using OfflineStorage_SQLite::autoCommitTransaction;
    using OfflineStorage_SQLite::trimDbIfNeeded;

    MOCK_METHOD0(scheduleAutoCommitTransaction, void());
};

//---

struct OfflineStorageTests_SQLiteWithMock : public Test
{
    struct FakeStatement {
        std::string recipe;
        bool reset;
    };

    LogConfiguration                               configuration;
    StrictMock<MockIRuntimeConfig>                 runtimeConfigMock;
    StrictMock<MockISqlite3Proxy>                  sqliteMock;
    ISqlite3Proxy*                                 savedSqliteProxy;
    StrictMock<MockIOfflineStorageObserver>        observerMock;
    std::unique_ptr<OfflineStorage_SQLite4Test>    os;
    sqlite3*                                       dbHandle;
    std::map<sqlite3_stmt*, FakeStatement>         statements;
    TestRecordConsumer                             consumer;

    virtual void SetUp() override
    {
        savedSqliteProxy = g_sqlite3Proxy;
        g_sqlite3Proxy   = &sqliteMock;

        EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
            .WillRepeatedly(Return(1000000));
        configuration.cacheFilePath = "OfflineStorageTests_SQLiteWithMock.db";

        os.reset(new OfflineStorage_SQLite4Test(configuration, runtimeConfigMock));

        dbHandle = reinterpret_cast<sqlite3*>(PAL::getUtcSystemTimeMs() & ~0xFFFF);

        EXPECT_CALL(sqliteMock, sqlite3_finalize(_))
            .WillRepeatedly(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeFinalizeStatement));
        EXPECT_CALL(sqliteMock, sqlite3_reset(_))
            .WillRepeatedly(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeResetStatement));
        EXPECT_CALL(sqliteMock, sqlite3_clear_bindings(_))
            .WillRepeatedly(Return(SQLITE_OK));
        EXPECT_CALL(sqliteMock, sqlite3_step(_))
            .WillRepeatedly(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepUnexpected));
        EXPECT_CALL(sqliteMock, sqlite3_changes(_))
            .WillRepeatedly(Return(123));
    }

    virtual void TearDown() override
    {
        os.reset();
        EXPECT_THAT(statements.empty(), true);
        g_sqlite3Proxy = savedSqliteProxy;
    }

    int fakePrepareStatement(sqlite3* db, char const* zsql, int size, sqlite3_stmt** pstmt, char const** pztail)
    {
        UNREFERENCED_PARAMETER(db);
        UNREFERENCED_PARAMETER(pztail);
        *pstmt = reinterpret_cast<sqlite3_stmt*>(reinterpret_cast<size_t>(dbHandle) + statements.size() + 1);
        statements[*pstmt] = FakeStatement{(size < 0) ? std::string(zsql) : std::string(zsql, size), true};
        return SQLITE_OK;
    }

    int fakeFinalizeStatement(sqlite3_stmt* stmt)
    {
        EXPECT_THAT(statements, Contains(Pair(stmt, _)));
        statements.erase(stmt);
        return SQLITE_OK;
    }

    int fakeResetStatement(sqlite3_stmt* stmt)
    {
        EXPECT_THAT(statements, Contains(Pair(stmt, _)));
        statements[stmt].reset = true;
        return SQLITE_OK;
    }

    int fakeStatementStepUnexpected(sqlite3_stmt* stmt)
    {
        EXPECT_THAT(statements, Contains(Pair(stmt, _)));
        EXPECT_THAT(statements[stmt].recipe, StrEq("is-known-and-expected"));
        return SQLITE_ERROR;
    }

    int fakeStatementStepDone(sqlite3_stmt* stmt)
    {
        EXPECT_THAT(statements, Contains(Pair(stmt, _)));
        EXPECT_THAT(statements[stmt].reset, true);
        if (!statements[stmt].reset) {
            return SQLITE_ERROR;
        }
        statements[stmt].reset = false;
        return SQLITE_DONE;
    }

    int fakeStatementStepRow(sqlite3_stmt* stmt)
    {
        EXPECT_THAT(statements, Contains(Pair(stmt, _)));
        EXPECT_THAT(statements[stmt].reset, true);
        if (!statements[stmt].reset) {
            return SQLITE_ERROR;
        }
        statements[stmt].reset = false;
        return SQLITE_ROW;
    }

    int fakeStatementStepAnotherRow(sqlite3_stmt* stmt)
    {
        EXPECT_THAT(statements, Contains(Pair(stmt, _)));
        EXPECT_THAT(statements[stmt].reset, false);
        if (statements[stmt].reset) {
            return SQLITE_ERROR;
        }
        return SQLITE_ROW;
    }

    int fakeStatementStepAnotherRowDone(sqlite3_stmt* stmt)
    {
        EXPECT_THAT(statements, Contains(Pair(stmt, _)));
        EXPECT_THAT(statements[stmt].reset, false);
        if (statements[stmt].reset) {
            return SQLITE_ERROR;
        }
        statements[stmt].reset = true;
        return SQLITE_DONE;
    }

    void expectOpenDatabase(char const* path = nullptr)
    {
        {
            OutsideSequence order;
            EXPECT_CALL(sqliteMock, sqlite3_prepare_v2(dbHandle, _, -1, _, NULL))
                .WillRepeatedly(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakePrepareStatement));
        }

        EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(path ? path : configuration.cacheFilePath.c_str()), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
            .WillOnce(DoAll(
            SetArgPointee<1>(dbHandle),
            Return(SQLITE_OK)))
            .RetiresOnSaturation();
        EXPECT_CALL(sqliteMock, sqlite3_extended_result_codes(dbHandle, 1))
            .WillOnce(Return(SQLITE_OK))
            .RetiresOnSaturation();
        EXPECT_CALL(sqliteMock, sqlite3_create_function_v2(dbHandle, StrEq("tokenize"), 1, SQLITE_UTF8, NULL, _, NULL, NULL, NULL))
            .WillOnce(Return(SQLITE_OK))
            .RetiresOnSaturation();
    }

    void expectInitializeDatabase(int steps = 999)
    {
        if (steps < 1) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA auto_vacuum=INCREMENTAL")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
            .RetiresOnSaturation();
        if (steps < 2) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA journal_mode=WAL")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
            .RetiresOnSaturation();
        if (steps < 3) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA journal_size_limit=.*")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
            .RetiresOnSaturation();
        if (steps < 4) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA user_version")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
            .RetiresOnSaturation();
        if (steps < 5) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "PRAGMA user_version"), 0))
            .WillOnce(Return(0))
            .RetiresOnSaturation();
        if (steps < 6) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA user_version=.*")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
            .RetiresOnSaturation();
        if (steps < 7) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "CREATE TABLE IF NOT EXISTS events .*")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
            .RetiresOnSaturation();
        if (steps < 8) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "CREATE INDEX IF NOT EXISTS k_priority_timestamp .*")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
            .RetiresOnSaturation();
        if (steps < 9) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "CREATE TABLE IF NOT EXISTS settings .*")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
            .RetiresOnSaturation();
        if (steps < 10) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_size")))
            .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
            .RetiresOnSaturation();
        if (steps < 11) { return; }
        EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "PRAGMA page_size"), 0))
            .WillOnce(Return(1024))
            .RetiresOnSaturation();
    }

    void expectDatabaseShutdown(bool optional = false)
    {
        EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
            .Times(Between(optional ? 0 : 1, 1))
            .WillRepeatedly(Return(SQLITE_OK))
            .RetiresOnSaturation();
        dbHandle = reinterpret_cast<sqlite3*>(reinterpret_cast<char*>(dbHandle) + 0x111);
    }

    static int vfsDeleteOk(sqlite3_vfs* vfs, char const* zName, int syncDir)
    {
        UNREFERENCED_PARAMETER(vfs);
        UNREFERENCED_PARAMETER(zName);
        UNREFERENCED_PARAMETER(syncDir);
        return SQLITE_OK;
    }

    static int vfsDeleteError(sqlite3_vfs* vfs, char const* zName, int syncDir)
    {
        UNREFERENCED_PARAMETER(vfs);
        UNREFERENCED_PARAMETER(zName);
        UNREFERENCED_PARAMETER(syncDir);
        return SQLITE_IOERR_ACCESS;
    }

    void expectVfsDelete(bool successful)
    {
        static sqlite3_vfs vfs;
        vfs.xDelete = successful ? &vfsDeleteOk : &vfsDeleteError;
        EXPECT_CALL(sqliteMock, sqlite3_vfs_find(NULL))
            .WillOnce(Return(&vfs))
            .RetiresOnSaturation();
    }
};

//---

TEST_F(OfflineStorageTests_SQLiteWithMock, AllSqliteFunctionCallsAreDeferred)
{
    // Empty, only constructor and destructor of the class are being tested
    // that they do not call any SQLite functions from the main thread.
}

TEST_F(OfflineStorageTests_SQLiteWithMock, InitializeCreatesNewDatabase)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectOpenDatabase();
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    os->Initialize(observerMock);

    expectDatabaseShutdown();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, NoInitializationMeansNoDatabase)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_ERROR))
        .RetiresOnSaturation();
    EXPECT_CALL(observerMock, OnStorageFailed("1"));
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .Times(3)
        .WillRepeatedly(Return(SQLITE_ERROR))
        .RetiresOnSaturation();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/None"));

    os->Initialize(observerMock);

    // No database is open, but other methods are still safe to call and no SQLite methods will be invoked
    os->StoreRecord({"guid", "tenant", EventPriority_Low, 2, {'x'}});
	bool fromMemory = false;
    os->GetAndReserveRecords(consumer, 1000, EventPriority_Normal, 3);
	HttpHeaders test;
    os->DeleteRecords({"guid"}, test, fromMemory);
    os->ReleaseRecords({"guid"}, true, test, fromMemory);
    os->StoreSetting("key", "value");
    os->GetSetting("key");
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, InitializationErrorsFallbackToRecreatedDatabaseFile)
{
    InSequence order;

    // Existing database is corrupted
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(configuration.cacheFilePath), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
        .WillOnce(DoAll(
        SetArgPointee<1>(dbHandle),
        Return(SQLITE_CORRUPT)))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Database corrupted"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(observerMock, OnStorageFailed("1"));

    // Old file is deleted and new database is opened successfully
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectVfsDelete(true);
    expectOpenDatabase();
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Clean"));

    os->Initialize(observerMock);

    expectDatabaseShutdown();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, InitializationErrorsFallbackToTemporaryDatabaseFile)
{
    InSequence order;

    // Existing database is corrupted
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(configuration.cacheFilePath), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
        .WillOnce(DoAll(
        SetArgPointee<1>(dbHandle),
        Return(SQLITE_CORRUPT)))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Database corrupted"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(observerMock, OnStorageFailed("1"));

    // Database cannot be recreated due to I/O error
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectVfsDelete(false);
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();

    // Temporary file database is used successfully
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectOpenDatabase("");
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Temp"));

    os->Initialize(observerMock);

    expectDatabaseShutdown();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, InitializationErrorsFallbackToInMemoryDatabase)
{
    InSequence order;

    // Existing database is corrupted
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(configuration.cacheFilePath), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
        .WillOnce(DoAll(
        SetArgPointee<1>(dbHandle),
        Return(SQLITE_CORRUPT)))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Database corrupted"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(observerMock, OnStorageFailed("1"));

    // New database cannot be created due to I/O error
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectVfsDelete(true);
    expectOpenDatabase();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA auto_vacuum=INCREMENTAL")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Cannot open file"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();

    // Temporary file database also cannot be created due to I/O error
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(""), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
        .WillOnce(DoAll(
        SetArgPointee<1>(dbHandle),
        Return(SQLITE_IOERR)))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Cannot create file"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();

    // In-memory database is used successfully
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectOpenDatabase(":memory:");
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Memory"));

    os->Initialize(observerMock);

    expectDatabaseShutdown();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, CompletelyFailedInitializationMeansNoDatabase)
{
    InSequence order;

    // Existing database is corrupted
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(configuration.cacheFilePath), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
        .WillOnce(DoAll(
        SetArgPointee<1>(dbHandle),
        Return(SQLITE_CORRUPT)))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Database corrupted"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(observerMock, OnStorageFailed("1"));

    // New database cannot be created due to I/O error
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectVfsDelete(true);
    expectOpenDatabase();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA auto_vacuum=INCREMENTAL")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Cannot open file"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();

    // Temporary file database also cannot be created due to I/O error
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    expectOpenDatabase("");
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA auto_vacuum=INCREMENTAL")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Cannot open file"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();

    // In-memory database also cannot be created due to another error
    EXPECT_CALL(sqliteMock, sqlite3_initialize())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(":memory:"), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
        .WillOnce(DoAll(
        SetArgPointee<1>(dbHandle),
        Return(SQLITE_OK)))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_extended_result_codes(dbHandle, 1))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_create_function_v2(dbHandle, StrEq("tokenize"), 1, SQLITE_UTF8, NULL, _, NULL, NULL, NULL))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_close(dbHandle))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_shutdown())
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/None"));

    os->Initialize(observerMock);

    // No database is open, but other methods are still safe to call and no SQLite methods will be invoked
    os->StoreRecord({"guid", "tenant", EventPriority_Low, 2, {'x'}});
	bool fromMemory = false;
    os->GetAndReserveRecords(consumer, 1000, EventPriority_Normal, 3);
	HttpHeaders test;
    os->DeleteRecords({"guid"}, test, fromMemory);
    os->ReleaseRecords({"guid"}, true, test, fromMemory);
    os->StoreSetting("key", "value");
    os->GetSetting("key");
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, initializeDatabase_FailsOnErrors)
{
    InSequence order;

    // Prepare a valid m_db
    os->setSkipInitAndShutdown(true);
    expectOpenDatabase();
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    os->Initialize(observerMock);

    expectInitializeDatabase(0);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA auto_vacuum=INCREMENTAL")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(1);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA journal_mode=WAL")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(2);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA journal_size_limit=.*")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(3);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA user_version")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(5);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA user_version=.*")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(6);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "CREATE TABLE IF NOT EXISTS events .*")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(7);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "CREATE INDEX IF NOT EXISTS k_priority_timestamp .*")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(8);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "CREATE TABLE IF NOT EXISTS settings .*")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(9);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_size")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectInitializeDatabase(11);
    EXPECT_THAT(os->initializeDatabase(), true);

    expectDatabaseShutdown();
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, initializeDatabase_UpgradesOlderDatabase)
{
    InSequence order;

    // Prepare a valid m_db
    os->setSkipInitAndShutdown(true);
    expectOpenDatabase();
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    os->Initialize(observerMock);

    expectInitializeDatabase(4);
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "PRAGMA user_version"), 0))
        .WillOnce(Return(-999))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA user_version=.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    // Fail on the next command, no need to continue
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "CREATE TABLE IF NOT EXISTS events .*")))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    EXPECT_THAT(os->initializeDatabase(), false);

    expectDatabaseShutdown();
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, initializeDatabase_recreatesNewerDatabase)
{
    InSequence order;

    // Prepare a valid m_db
    os->setSkipInitAndShutdown(true);
    expectOpenDatabase();
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    os->Initialize(observerMock);

    expectInitializeDatabase(4);
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "PRAGMA user_version"), 0))
        .WillOnce(Return(999))
        .RetiresOnSaturation();
    EXPECT_THAT(os->initializeDatabase(), false);

    expectDatabaseShutdown();
    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, DoubleShutdownIsHarmless)
{
    os->setSkipInitAndShutdown(true);

    InSequence order;

    expectOpenDatabase();
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    os->Initialize(observerMock);

    expectDatabaseShutdown();
    os->Shutdown();

    os->Shutdown();
}

TEST_F(OfflineStorageTests_SQLiteWithMock, CommitFailureDuringShutdownIsIgnored)
{
    os->setSkipInitAndShutdown(true);

    InSequence order;

    expectOpenDatabase();
    expectInitializeDatabase();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    os->Initialize(observerMock);

    os->setInTransaction(true);

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseShutdown();
    os->Shutdown();
}

//---

class OfflineStorageTests_SQLiteWithMockInitialized : public OfflineStorageTests_SQLiteWithMock
{
  public:
    virtual void SetUp() override
    {
        configuration.skipSqliteInitAndShutdown = true;
        OfflineStorageTests_SQLiteWithMock::SetUp();
        expectOpenDatabase();
        expectInitializeDatabase();
        EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
        os->Initialize(observerMock);
    }

    virtual void TearDown() override
    {
        expectDatabaseShutdown(true);
        os->Shutdown();
        OfflineStorageTests_SQLiteWithMock::TearDown();
    }

    void expectDatabaseRecreation(bool successful, std::string const& reason)
    {
        expectDatabaseShutdown();
        EXPECT_CALL(observerMock, OnStorageFailed(reason))
            .WillOnce(Return())
            .RetiresOnSaturation();
        expectVfsDelete(true);

        if (successful) {
            expectOpenDatabase();
            expectInitializeDatabase();
            EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Clean"));
            return;
        }

        EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(configuration.cacheFilePath), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
            .WillOnce(DoAll(
            SetArgPointee<1>(nullptr),
            Return(SQLITE_NOMEM)))
            .RetiresOnSaturation();
        EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(""), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
            .WillOnce(DoAll(
            SetArgPointee<1>(nullptr),
            Return(SQLITE_NOMEM)))
            .RetiresOnSaturation();
        EXPECT_CALL(sqliteMock, sqlite3_open_v2(StrEq(":memory:"), _, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL))
            .WillOnce(DoAll(
            SetArgPointee<1>(nullptr),
            Return(SQLITE_NOMEM)))
            .RetiresOnSaturation();
        EXPECT_CALL(observerMock, OnStorageOpened("SQLite/None"));
    }
};

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreRecord_Succeeds)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO events .*"), 1, StrEq("guid"), 4, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO events .*"), 2, StrEq("tenant"), 6, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "REPLACE INTO events .*"), 3, 1))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "REPLACE INTO events .*"), 4, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, "REPLACE INTO events .*"), 5, _, 1, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "REPLACE INTO events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(*os, scheduleAutoCommitTransaction())
        .RetiresOnSaturation();

    EXPECT_THAT(os->StoreRecord({"guid", "tenant", EventPriority_Low, 2, {3}}), true);

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreRecord_FailsWithInvalidInput)
{
    EXPECT_THAT(os->StoreRecord({"",     "tenant",  EventPriority_Low,               2, {3}}), false);
    EXPECT_THAT(os->StoreRecord({"guid", "",        EventPriority_Low,               2, {3}}), false);
    EXPECT_THAT(os->StoreRecord({"guid", "tenant",  EventPriority_Unspecified,       2, {3}}), false);
    EXPECT_THAT(os->StoreRecord({"guid", "tenant",  EventPriority_Normal,            0, {3}}), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreRecord_RetriesOnceOnError)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO events .*"), 1, StrEq("guid"), 4, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO events .*"), 2, StrEq("tenant"), 6, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "REPLACE INTO events .*"), 3, 1))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "REPLACE INTO events .*"), 4, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, "REPLACE INTO events .*"), 5, _, 20000, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "REPLACE INTO events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(*os, scheduleAutoCommitTransaction())
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "101");

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "101");

    EXPECT_THAT(os->StoreRecord({"guid", "tenant", EventPriority_Low, 2, std::vector<uint8_t>(20000, 42)}), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreRecord_AbortsIfDbCannotBeRecreated)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO events .*"), 1, StrEq("guid"), 4, _))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(false, "101");

    EXPECT_THAT(os->StoreRecord({"guid", "tenant", EventPriority_Low, 2, {3}}), false);
}

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_Succeeds)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_changes(dbHandle))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    consumer.maxCount = 1;

    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "SELECT .* FROM events .*"), 1, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 2, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(5))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("guid1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(7))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("tenant1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 2))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 3))
        .WillOnce(Return(2345))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 4))
        .WillOnce(Return(0))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 5))
        .WillOnce(Return(6789))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(9))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_blob(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("bondblob")))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepAnotherRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(5))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("guid2")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(9))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("tenantTwo")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 2))
        .WillOnce(Return(2))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 3))
        .WillOnce(Return(3456))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 4))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 5))
        .WillOnce(Return(7890))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(10))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_blob(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("blob" "\xFF" "bond")))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=. .*"), 1, _, 6, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, ".* UPDATE events SET reserved_until=. .*"), 2, Near(now + 120000, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, ".* UPDATE events SET reserved_until=. .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), true);
    EXPECT_THAT(consumer.records, SizeIs(1));
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_SucceedsWithNoOutput)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "SELECT .* FROM events .*"), 1, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 2, 4))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "ROLLBACK")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_High, 4), true);
    EXPECT_THAT(consumer.records, SizeIs(0));
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesCommitPreviousFailure)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "201");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesReleaseExpiredFailure)
{
    InSequence order;

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "202");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesBeginTransactionFailure)
{
    InSequence order;

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "203");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesSelectFailure)
{
    InSequence order;

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "SELECT .* FROM events .*"), 1, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 2, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "204");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesGetRowFailure)
{
    InSequence order;

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "SELECT .* FROM events .*"), 1, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 2, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(5))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("guid1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(7))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("tenant1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 2))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 3))
        .WillOnce(Return(2345))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 4))
        .WillOnce(Return(0))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 5))
        .WillOnce(Return(6789))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(9))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_blob(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("bondblob")))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "205");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesRollbackFailure)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "SELECT .* FROM events .*"), 1, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 2, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "ROLLBACK")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "206");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesReserveFailure)
{
    InSequence order;

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_changes(dbHandle))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "SELECT .* FROM events .*"), 1, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 2, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(5))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("guid1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(7))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("tenant1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 2))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 3))
        .WillOnce(Return(2345))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 4))
        .WillOnce(Return(0))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 5))
        .WillOnce(Return(6789))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(9))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_blob(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("bondblob")))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepAnotherRowDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=. .*"), 1, _, 6, _))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "207");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetAndReserveRecords_HandlesCommitFailure)
{
    InSequence order;

    int64_t now = PAL::getUtcSystemTimeMs();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*"), 1, Near(now, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "UPDATE events SET reserved_until=0, retry_count=retry_count.1 .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_changes(dbHandle))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, "SELECT .* FROM events .*"), 1, 2))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 2, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(5))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 0))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("guid1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(7))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM events .*"), 1))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("tenant1")))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 2))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 3))
        .WillOnce(Return(2345))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int(PreparedStatement(this, "SELECT .* FROM events .*"), 4))
        .WillOnce(Return(0))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "SELECT .* FROM events .*"), 5))
        .WillOnce(Return(6789))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(9))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_blob(PreparedStatement(this, "SELECT .* FROM events .*"), 6))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("bondblob")))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepAnotherRowDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=. .*"), 1, _, 6, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, ".* UPDATE events SET reserved_until=. .*"), 2, Near(now + 120000, 10000)))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, ".* UPDATE events SET reserved_until=. .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "208");
    EXPECT_THAT(os->GetAndReserveRecords(consumer, 120000, EventPriority_Normal, 3), false);
}

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, DeleteRecords_Succeeds)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* DELETE FROM events .*"), 1, _, 4 + 4, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, ".* DELETE FROM events .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
	HttpHeaders test;
	bool fromMemory = false;
    os->DeleteRecords({"id1", "id2"}, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, DeleteRecords_DoesNothingWithEmptyInput)
{
	HttpHeaders test;
	bool fromMemory = false;
    os->DeleteRecords({}, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, DeleteRecords_HandlesCommitFailure)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "301");
	HttpHeaders test;
	bool fromMemory = false;
    os->DeleteRecords({"id"}, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, DeleteRecords_HandlesDeletionFalure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* DELETE FROM events .*"), 1, _, 3, _))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "302");
	HttpHeaders test;
	bool fromMemory = false;
    os->DeleteRecords({"id"}, test, fromMemory);
}

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_Succeeds)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 1, _, 4 + 4, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 2, 0))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({"id1", "id2"}, false, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_SucceedsAndIncreasesRetryCount)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 1, _, 4 + 4, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 2, 1))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(runtimeConfigMock, GetMaximumRetryCount())
        .WillOnce(Return(3))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events WHERE retry_count>.*"), 1, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM events WHERE retry_count>.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(observerMock, OnStorageRecordsDropped(123))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({"id1", "id2"}, true, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_DoesNothingWithEmptyInput)
{
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({}, false, test, fromMemory);
    os->ReleaseRecords({}, true, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_HandlesCommitPreviousFailure)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "401");
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({"id"}, false, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_HandlesBeginTransactionFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "402");
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({"id"}, false, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_HandlesReleaseFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 1, _, 3, _))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "403");
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({"id"}, false, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_HandlesDeletionFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 1, _, 3, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 2, 1))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(runtimeConfigMock, GetMaximumRetryCount())
        .WillOnce(Return(3))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events WHERE retry_count>.*"), 1, 3))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "404");
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({"id"}, true, test, fromMemory);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, ReleaseRecords_HandlesFinalCommitFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "BEGIN.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_blob(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 1, _, 3, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_int(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*"), 2, 1))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, ".* UPDATE events SET reserved_until=0, retry_count=retry_count+.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(runtimeConfigMock, GetMaximumRetryCount())
        .WillOnce(Return(3))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events WHERE retry_count>.*"), 1, 3))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM events WHERE retry_count>.*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(observerMock, OnStorageRecordsDropped(123))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "405");
	HttpHeaders test;
	bool fromMemory = false;
    os->ReleaseRecords({"id"}, true, test, fromMemory);
}

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreSetting_StoringSucceeds)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO settings .*"), 1, StrEq("key"), 3, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO settings .*"), 2, StrEq("value"), 5, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "REPLACE INTO settings .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_THAT(os->StoreSetting("key", "value"), true);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreSetting_DeletionSucceeds)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "DELETE FROM settings .*"), 1, StrEq("key"), 3, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM settings .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_THAT(os->StoreSetting("key", ""), true);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreSetting_FailsWithInvalidInput)
{
    EXPECT_THAT(os->StoreSetting("", "value"), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreSetting_HandlesCommitPreviousFailure)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "501");

    EXPECT_THAT(os->StoreSetting("key", "value"), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreSetting_HandlesStoringFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "REPLACE INTO settings .*"), 1, StrEq("key"), 3, _))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "502");

    EXPECT_THAT(os->StoreSetting("key", "value"), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, StoreSetting_HandlesDeletionFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "DELETE FROM settings .*"), 1, StrEq("key"), 3, _))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "503");

    EXPECT_THAT(os->StoreSetting("key", ""), false);
}

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetSetting_Succeeds)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "SELECT .* FROM settings .*"), 1, StrEq("key"), 3, _))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "SELECT .* FROM settings .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_bytes(PreparedStatement(this, "SELECT .* FROM settings .*"), 0))
        .WillOnce(Return(5))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_text(PreparedStatement(this, "SELECT .* FROM settings .*"), 0))
        .WillOnce(Return(reinterpret_cast<unsigned const char*>("value")))
        .RetiresOnSaturation();

    EXPECT_THAT(os->GetSetting("key"), StrEq("value"));
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetSetting_FailsWithInvalidInput)
{
    EXPECT_THAT(os->GetSetting(""), StrEq(""));
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetSetting_HandlesCommitPreviousFailure)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "504");

    EXPECT_THAT(os->GetSetting("key"), StrEq(""));
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, GetSetting_HandlesSelectFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_bind_text(PreparedStatement(this, "SELECT .* FROM settings .*"), 1, StrEq("key"), 3, _))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "505");

    EXPECT_THAT(os->GetSetting("key"), StrEq(""));
}

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, autoCommitTransaction_HandlesCommitFailure)
{
    InSequence order;

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    expectDatabaseRecreation(true, "601");

    os->autoCommitTransaction();
}

//---

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_SucceedsUnderLimit)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) - 100))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), true);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_SucceedsWithTrimming)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 100))
        .RetiresOnSaturation();

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*"), 1, 75))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(observerMock, OnStorageTrimmed(123));

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA incremental_vacuum\\(0\\)")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) - 100))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), true);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_HandlesCurrentPageCountFailure)
{
    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_HandlesCommitPreviousFailure)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 100))
        .RetiresOnSaturation();

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), false);

    os->setInTransaction(false); // Left in transaction, will be recreated anyway
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_HandlesDeleteFailure)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 100))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*"), 1, 75))
        .WillOnce(Return(SQLITE_NOMEM))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("Out of memory"))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_HandlesVacuumFailure)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 100))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*"), 1, 75))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(observerMock, OnStorageTrimmed(123));

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA incremental_vacuum\\(0\\)")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_HandlesVacuumStepFailure)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 100))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*"), 1, 75))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(observerMock, OnStorageTrimmed(123));

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA incremental_vacuum\\(0\\)")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA incremental_vacuum\\(0\\)")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_HandlesNewPageCountFailure)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 100))
        .RetiresOnSaturation();

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*"), 1, 75))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(observerMock, OnStorageTrimmed(123));

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA incremental_vacuum\\(0\\)")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Return(SQLITE_IOERR))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_errmsg(dbHandle))
        .WillOnce(Return("I/O error"))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), false);
}

TEST_F(OfflineStorageTests_SQLiteWithMockInitialized, trimDbIfNeeded_FailsIfNewSizeIsStillOverLimit)
{
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(1000000));
    EXPECT_CALL(runtimeConfigMock, GetOfflineStorageResizeThresholdPct())
        .WillRepeatedly(Return(75));

    InSequence order;

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 100))
        .RetiresOnSaturation();

    os->setInTransaction(true);
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "COMMIT")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_bind_int64(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*"), 1, 75))
        .WillOnce(Return(SQLITE_OK))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "DELETE FROM events .* ORDER BY priority ASC, timestamp ASC .*")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(observerMock, OnStorageTrimmed(123));

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA incremental_vacuum\\(0\\)")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepDone))
        .RetiresOnSaturation();

    EXPECT_CALL(sqliteMock, sqlite3_step(PreparedStatement(this, "PRAGMA page_count")))
        .WillOnce(Invoke(this, &OfflineStorageTests_SQLiteWithMock::fakeStatementStepRow))
        .RetiresOnSaturation();
    EXPECT_CALL(sqliteMock, sqlite3_column_int64(PreparedStatement(this, "PRAGMA page_count"), 0))
        .WillOnce(Return((1000000 / 1024) + 50))
        .RetiresOnSaturation();

    EXPECT_THAT(os->trimDbIfNeeded(20000), false);
}
