//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#if 0
#include "common/Common.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "offline/OfflineStorage_SQLite.hpp"
#include <stdio.h>
#include <fstream>

#include "NullObjects.hpp"

using namespace testing;
using namespace MAT;
using namespace PAL;

char const* const TEST_STORAGE_FILENAME = "OfflineStorageTests_SQLite.db";


class OfflineStorage_SQLiteNoAutoCommit : public OfflineStorage_SQLite
{
  public:
    OfflineStorage_SQLiteNoAutoCommit(ILogManager& logManager, IRuntimeConfig& runtimeConfig, bool inMemory = false)
      : OfflineStorage_SQLite(logManager, runtimeConfig, inMemory)
    {
    }

    virtual void scheduleAutoCommitTransaction()
    {
    }
};


struct OfflineStorageTests_SQLite : public Test
{
    StrictMock<MockIRuntimeConfig>                      configMock;
    StrictMock<MockIOfflineStorageObserver>             observerMock;
    ILogManager *                                       logManager;
    ILogConfiguration                                   configuration;
    std::unique_ptr<OfflineStorage_SQLiteNoAutoCommit>  offlineStorage;

    virtual void SetUp() override
    {
        static NullLogManager nullLogManager;
        logManager = &nullLogManager;

        configuration["cacheFilePath"] = TEST_STORAGE_FILENAME;
        EXPECT_CALL(configMock, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(Return(UINT_MAX));

        offlineStorage.reset(new OfflineStorage_SQLiteNoAutoCommit(*logManager, configMock));

        EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"))
            .RetiresOnSaturation();
        offlineStorage->Initialize(observerMock);
    }

    virtual void TearDown() override
    {
        shutdownAndRemoveFile();
    }

    void shutdownAndRemoveFile()
    {
        offlineStorage->Shutdown();
        EXPECT_THAT(fileExists(TEST_STORAGE_FILENAME), true);
        ::remove(TEST_STORAGE_FILENAME);
        ASSERT_THAT(fileExists(TEST_STORAGE_FILENAME), false);
    }

    bool fileExists(std::string const& filename)
    {
        return std::ifstream(filename).good();
    }
};


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


TEST_F(OfflineStorageTests_SQLite, InitializeAndShutdownCreateFileThatCanBeDeleted)
{
}

TEST_F(OfflineStorageTests_SQLite, StorageRecordConstructorSetsAllFields)
{
    StorageRecord record{ "guid", "token", EventLatency_RealTime, EventPersistence_Critical, INT64_MIN + 1, { 5, 4, 3, 2, 1 }, 77, INT64_MAX - 1 };
    EXPECT_THAT(record.id, StrEq("guid"));
    EXPECT_THAT(record.tenantToken, StrEq("token"));
    EXPECT_THAT(record.latency, EventLatency_RealTime);
    EXPECT_THAT(record.timestamp, INT64_MIN + 1);
    EXPECT_THAT(record.blob, StorageBlob({ 5, 4, 3, 2, 1 }));
    EXPECT_THAT(record.retryCount, 77);
    EXPECT_THAT(record.reservedUntil, INT64_MAX - 1);
}

TEST_F(OfflineStorageTests_SQLite, GetAndReservedReturnsStoredRecord)
{
    StorageRecord record{ "guid", "token", EventLatency_Normal, EventPersistence_Normal, 1, { 5, 4, 3, 2, 1 } };
    ASSERT_THAT(offlineStorage->StoreRecord(record), true);
    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].id, record.id);
    EXPECT_THAT(consumer.records[0].tenantToken, record.tenantToken);
    EXPECT_THAT(consumer.records[0].latency, record.latency);
    EXPECT_THAT(consumer.records[0].timestamp, record.timestamp);
    EXPECT_THAT(consumer.records[0].blob, record.blob);
    EXPECT_THAT(consumer.records[0].retryCount, 0);
    EXPECT_THAT(consumer.records[0].reservedUntil, 0);
}

TEST_F(OfflineStorageTests_SQLite, ReservedRecordIsNotReturned)
{
    ASSERT_THAT(offlineStorage->StoreRecord({"guid1", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid2", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid3", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000, EventLatency_Unspecified, 1), true);
    ASSERT_THAT(consumer.records.size(), 1);
    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 2);
}

TEST_F(OfflineStorageTests_SQLite, DeletedRecordsAreNotReturned)
{
    ASSERT_THAT(offlineStorage->StoreRecord({"guid1", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid2", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid3", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    HttpHeaders test;
    bool fromMemory = false;
    offlineStorage->DeleteRecords({"guid1", "guid3"}, test, fromMemory);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid2"));
}

TEST_F(OfflineStorageTests_SQLite, ReservedRecordsAreReleasedAfterTimeout)
{
    ASSERT_THAT(offlineStorage->StoreRecord({"guid1", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid2", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    TestRecordConsumer consumer;
    // Reserve first for 2 secs
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 2000, EventLatency_Unspecified, 1), true);
    ASSERT_THAT(consumer.records.size(), 1);
    consumer.records.clear();

    PAL::sleep(500);

    // Reserve second for 1 sec, first still unavailable
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 1000, EventLatency_Unspecified, 1), true);
    ASSERT_THAT(consumer.records.size(), 1);
    consumer.records.clear();

    PAL::sleep(2000);

    // Both records are timed out
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 1000), true);
    ASSERT_THAT(consumer.records.size(), 2);
    EXPECT_THAT(consumer.records[0].retryCount, 1);
    EXPECT_THAT(consumer.records[1].retryCount, 1);
}

TEST_F(OfflineStorageTests_SQLite, GetAndReserveRecordsReservesRecordsSortedByTimestamp)
{
    StorageRecord unsortedRecords[] = {
        { "guid-6", "token", EventLatency_Normal, EventPersistence_Normal, 3, {11} },
        { "guid-1", "token", EventLatency_Normal, EventPersistence_Normal, 4, {22} },
        { "guid-5", "token", EventLatency_Normal, EventPersistence_Normal, 1, {33} },
        { "guid-4", "token", EventLatency_Normal, EventPersistence_Normal, 2, {44} },
        { "guid-3", "token", EventLatency_Normal, EventPersistence_Normal, 6, {55} },
        { "guid-2", "token", EventLatency_Normal, EventPersistence_Normal, 5, {66} }
    };

    for (auto const& r : unsortedRecords) {
        ASSERT_THAT(offlineStorage->StoreRecord(r), true);
    }

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, EventLatency_Unspecified, 3), true);
    ASSERT_THAT(consumer.records.size(), 3);
    EXPECT_THAT(consumer.records[0].timestamp, 1);
    EXPECT_THAT(consumer.records[1].timestamp, 2);
    EXPECT_THAT(consumer.records[2].timestamp, 3);

    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000), true);
    ASSERT_THAT(consumer.records.size(), 3);
    EXPECT_THAT(consumer.records[0].timestamp, 4);
    EXPECT_THAT(consumer.records[1].timestamp, 5);
    EXPECT_THAT(consumer.records[2].timestamp, 6);
}

TEST_F(OfflineStorageTests_SQLite, GetAndReserveRecordsReturnsOnlyHighestPriority)
{
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-11", "token1", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-12", "token1", EventLatency_Normal, EventPersistence_Normal, 2, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-13", "token1", EventLatency_RealTime, EventPersistence_Critical,   3, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-21", "token2", EventLatency_Normal, EventPersistence_Normal, 4, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-22", "token2", EventLatency_RealTime, EventPersistence_Critical,   5, {}}), true);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, EventLatency_RealTime), true);
    ASSERT_THAT(consumer.records.size(), 2);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid-13"));
    EXPECT_THAT(consumer.records[1].id, StrEq("guid-22"));
}

TEST_F(OfflineStorageTests_SQLite, GetAndReserveRecordsReturnsLowerPriorityIfHighestReserved)
{
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-11", "token1", EventLatency_RealTime, EventPersistence_Critical,   1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-12", "token1", EventLatency_Normal, EventPersistence_Normal, 2, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-13", "token1", EventLatency_Normal, EventPersistence_Normal, 3, {}}), true);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, EventLatency_RealTime), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid-11"));
    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, EventLatency_Normal), true);
    ASSERT_THAT(consumer.records.size(), 2);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid-12"));
    EXPECT_THAT(consumer.records[1].id, StrEq("guid-13"));
}

TEST_F(OfflineStorageTests_SQLite, GetAndReserveRecordsReservesOnlyReturnedRecordsWhenLimited)
{
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-1", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-2", "token", EventLatency_Normal, EventPersistence_Normal, 2, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-3", "token", EventLatency_Normal, EventPersistence_Normal, 3, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-4", "token", EventLatency_Normal, EventPersistence_Normal, 4, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-5", "token", EventLatency_Normal, EventPersistence_Normal, 5, {}}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"guid-6", "token", EventLatency_Normal, EventPersistence_Normal, 6, {}}), true);

    // limiting by consumer
    TestRecordConsumer limitedConsumer;
    limitedConsumer.maxCount = 2;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(limitedConsumer, 10000), true);
    ASSERT_THAT(limitedConsumer.records.size(), 2);
    EXPECT_THAT(limitedConsumer.records[0].id, StrEq("guid-1"));
    EXPECT_THAT(limitedConsumer.records[1].id, StrEq("guid-2"));

    // limiting by maxCount in getAndReserveRecords
    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, EventLatency_Normal, 2), true);
    ASSERT_THAT(consumer.records.size(), 2);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid-3"));
    EXPECT_THAT(consumer.records[1].id, StrEq("guid-4"));

    // still can reserve not consumed records
    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000), true);
    ASSERT_THAT(consumer.records.size(), 2);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid-5"));
    EXPECT_THAT(consumer.records[1].id, StrEq("guid-6"));
}

TEST_F(OfflineStorageTests_SQLite, ReleaseRecordsMakesThemAvailableAgain)
{
    StorageRecord record{ "guid", "token", EventLatency_Normal, EventPersistence_Normal, 1, {11} };
    ASSERT_THAT(offlineStorage->StoreRecord(record), true);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].retryCount, 0);
    HttpHeaders test;
    bool fromMemory = false;
    offlineStorage->ReleaseRecords({ "guid" }, false, test, fromMemory);

    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].retryCount, 0);
}

TEST_F(OfflineStorageTests_SQLite, ReleaseRecordsIncrementsRetryCount)
{
    StorageRecord record{ "guid", "token", EventLatency_Normal, EventPersistence_Normal, 1, {11} };
    ASSERT_THAT(offlineStorage->StoreRecord(record), true);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].retryCount, 0);

    EXPECT_CALL(configMock, GetMaximumRetryCount())
        .WillOnce(Return(2));
    HttpHeaders test;
    bool fromMemory = false;
    offlineStorage->ReleaseRecords({ "guid" }, true, test, fromMemory);

    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].retryCount, 1);
}

TEST_F(OfflineStorageTests_SQLite, ReleaseUnreservedRecordsDoesntIncrementRetryCount)
{
    StorageRecord record{ "guid", "token", EventLatency_Normal, EventPersistence_Normal, 1, {11} };
    ASSERT_THAT(offlineStorage->StoreRecord(record), true);

    EXPECT_CALL(configMock, GetMaximumRetryCount())
        .WillOnce(Return(2));
    HttpHeaders test;
    bool fromMemory = false;
    offlineStorage->ReleaseRecords({ "guid" }, true, test, fromMemory);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].retryCount, 0);
}

TEST_F(OfflineStorageTests_SQLite, ReleaseRecordsDeletesRecordsOverMaxRetryCount)
{
    ASSERT_THAT(offlineStorage->StoreRecord({ "guid",  "token", EventLatency_RealTime, EventPersistence_Critical, 1, {11} }), true);
    ASSERT_THAT(offlineStorage->StoreRecord({ "guid2", "token", EventLatency_Normal, EventPersistence_Normal, 1, {22} }), true);

    TestRecordConsumer consumer;
    int const MaxRetryCount = 5;
    EXPECT_CALL(configMock, GetMaximumRetryCount())
        .Times(MaxRetryCount + 1).WillRepeatedly(Return(MaxRetryCount));

    for (int i = 0; i <= MaxRetryCount; ++i) {
        consumer.records.clear();
        EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000, EventLatency_RealTime), true);
        ASSERT_THAT(consumer.records.size(), 1);
        EXPECT_THAT(consumer.records[0].retryCount, i);
        std::map<std::string, size_t> dropedRecord;
        dropedRecord["token"] = 1;
        EXPECT_CALL(observerMock, OnStorageRecordsDropped(dropedRecord))
            .Times((i == MaxRetryCount) ? 1 : 0);
        HttpHeaders test;
        bool fromMemory = false;
        offlineStorage->ReleaseRecords({ "guid" }, true, test, fromMemory);
    }

    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000, EventLatency_Normal), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid2"));
    EXPECT_THAT(consumer.records[0].retryCount, 0);
}

TEST_F(OfflineStorageTests_SQLite, GetAndReserveRecordsReturnsRecordsSortedByTimestamp)
{
    StorageRecord unsortedRecords[] = {
        { "guid-6", "token3", EventLatency_Normal, EventPersistence_Normal,    3, {11} },
        { "guid-1", "token5", EventLatency_RealTime, EventPersistence_Critical, 4, {22} },
        { "guid-5", "token4", EventLatency_Max, EventPersistence_Critical,2, {33} },
        { "guid-4", "token2", EventLatency_Normal, EventPersistence_Normal, 1, {44} },
        { "guid-3", "token1", EventLatency_Max, EventPersistence_Critical, 6, {55} },
        { "guid-2", "token6", EventLatency_Max, EventPersistence_Critical, 5, {66} }
    };

    for (auto const& r : unsortedRecords) {
        ASSERT_THAT(offlineStorage->StoreRecord(r), true);
    }

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000, EventLatency_Max), true);
    ASSERT_THAT(consumer.records.size(), 3);
    EXPECT_THAT(consumer.records[0].id, StrEq("guid-5"));
    EXPECT_THAT(consumer.records[1].id, StrEq("guid-2"));
    EXPECT_THAT(consumer.records[2].id, StrEq("guid-3"));
}

TEST_F(OfflineStorageTests_SQLite, StoreThousandEventsTakesLessThanASecond)
{
    auto startTimeMs = PAL::getMonotonicTimeMs();

    for (int i = 0; i < 1000; ++i) {
        EXPECT_THAT(offlineStorage->StoreRecord({std::to_string(i), "token", EventLatency_Normal, EventPersistence_Normal, 1, {}}), true);
    }

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, EventLatency_Normal, 1000), true);
    EXPECT_THAT(consumer.records.size(), 1000);

    auto endTimeMs = PAL::getMonotonicTimeMs();
    EXPECT_THAT(endTimeMs - startTimeMs, Le(1000));
}

TEST_F(OfflineStorageTests_SQLite, OnInvalidFilenameInitializeCreatesTemporaryDb)
{
    offlineStorage->Shutdown();

    std::string origCacheFilePath = (const char *)configuration[CFG_STR_CACHE_FILE_PATH];
    ::remove(origCacheFilePath.c_str());

    configuration[CFG_STR_CACHE_FILE_PATH] = "/\\/*/[]\\\\";

    offlineStorage.reset(new OfflineStorage_SQLiteNoAutoCommit(*logManager, configMock));
    EXPECT_CALL(observerMock, OnStorageFailed("1"));
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Temp"));
    offlineStorage->Initialize(observerMock);

    EXPECT_THAT(fileExists(origCacheFilePath), false);
    EXPECT_THAT(fileExists(configuration[CFG_STR_CACHE_FILE_PATH]), false);

    // Recreate for destructor
    std::ofstream(origCacheFilePath, std::ios::out);
}

TEST_F(OfflineStorageTests_SQLite, InitializeDeletesFileAndCreatesNewIfFailed)
{
    shutdownAndRemoveFile();

    {
        std::ofstream ofs(TEST_STORAGE_FILENAME, std::ofstream::out);
        ofs << "Garbage";
        ofs.close();
    }

    EXPECT_CALL(observerMock, OnStorageFailed("1"));
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Clean"));
    offlineStorage->Initialize(observerMock);
}

//--- Generated tests

class GoodRecordsTests : public OfflineStorageTests_SQLite,
                         public WithParamInterface<StorageRecord>
{
};

TEST_P(GoodRecordsTests, RecordStoredAndRetrievedCorrectly)
{
    StorageRecord const& record = GetParam();
    ASSERT_THAT(offlineStorage->StoreRecord(record), true);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, record.latency), true);
    ASSERT_THAT(consumer.records.size(), 1);
    auto storedRecord = consumer.records[0];

    EXPECT_THAT(storedRecord.id, record.id);
    EXPECT_THAT(storedRecord.tenantToken, record.tenantToken);
    EXPECT_THAT(storedRecord.latency, record.latency);
    EXPECT_THAT(storedRecord.timestamp, record.timestamp);
    EXPECT_THAT(storedRecord.blob, record.blob);
    EXPECT_THAT(storedRecord.retryCount, record.retryCount);
    EXPECT_THAT(storedRecord.reservedUntil, record.reservedUntil);
}

TEST_P(GoodRecordsTests, RecordStoredAndRetrievedCorrectlyAfterDbReopen)
{
    StorageRecord const& record = GetParam();
    ASSERT_THAT(offlineStorage->StoreRecord(record), true);
    offlineStorage->Shutdown();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"))
        .RetiresOnSaturation();
    offlineStorage->Initialize(observerMock);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000, record.latency), true);
    ASSERT_THAT(consumer.records.size(), 1);
    auto storedRecord = consumer.records[0];

    EXPECT_THAT(storedRecord.id, record.id);
    EXPECT_THAT(storedRecord.tenantToken, record.tenantToken);
    EXPECT_THAT(storedRecord.latency, record.latency);
    EXPECT_THAT(storedRecord.timestamp, record.timestamp);
    EXPECT_THAT(storedRecord.blob, record.blob);
    EXPECT_THAT(storedRecord.retryCount, record.retryCount);
    EXPECT_THAT(storedRecord.reservedUntil, record.reservedUntil);
}

class BadRecordsTests : public OfflineStorageTests_SQLite,
                        public WithParamInterface<StorageRecord>
{
};

TEST_P(BadRecordsTests, BadRecordStoredIsNotFoundInDb)
{
    StorageRecord const& record = GetParam();
    ASSERT_THAT(offlineStorage->StoreRecord(record), false);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 10000), true);
    ASSERT_THAT(consumer.records.size(), 0);
}

StorageRecord GOOD_RECORDS[] = {
    { "{ guid-\"' ", "tenant -to\"ken'", EventLatency_Normal, EventPersistence_Normal, INT64_MAX, StorageBlob{ 1, 2, 3, 4, 5, 6, 7 } },
    { "guid",        "tenant-token",     EventLatency_Max, EventPersistence_Critical, 1, StorageBlob(1024 * 1024, uint8_t(7)) },
    { "guid",        "tenant-token",     EventLatency_Off, EventPersistence_Normal, 1, {} }
};

StorageRecord BAD_RECORDS[] = {
    { "",     "tenant-token", EventLatency_Normal, EventPersistence_Normal,                2, { 1, 2, 3 } },
    { "guid", "",             EventLatency_Normal, EventPersistence_Normal,                2, { 1, 2, 3 } },
    { "guid", "tenant-token", EventLatency_Unspecified,EventPersistence_Normal,       0, {} },
    { "guid", "tenant-token", static_cast<EventLatency>(987),EventPersistence_Normal,  0, {} },
    { "guid", "tenant-token", EventLatency_Normal, EventPersistence_Normal,            -1, {} }
};

INSTANTIATE_TEST_CASE_P(OfflineStorageTests_SQLite, GoodRecordsTests, ::testing::ValuesIn(GOOD_RECORDS));
INSTANTIATE_TEST_CASE_P(OfflineStorageTests_SQLite, BadRecordsTests,  ::testing::ValuesIn(BAD_RECORDS));

//--- Settings tests

TEST_F(OfflineStorageTests_SQLite, AbsentSettingsAreRerturnedAsEmpty)
{
    EXPECT_THAT(offlineStorage->GetSetting("Some dummy setting name"), StrEq(""));
    EXPECT_THAT(offlineStorage->GetSetting("Another setting name"), StrEq(""));
}

TEST_F(OfflineStorageTests_SQLite, StoredSettingsSurviveDbReopen)
{
    offlineStorage->StoreSetting("setting 1", "Value for setting 1");
    offlineStorage->StoreSetting("setting 2", "Value for setting 2");
    offlineStorage->StoreSetting("setting 3", "Value for setting 3");
    EXPECT_THAT(offlineStorage->GetSetting("setting 3"), StrEq("Value for setting 3"));
    EXPECT_THAT(offlineStorage->GetSetting("setting 1"), StrEq("Value for setting 1"));
    EXPECT_THAT(offlineStorage->GetSetting("setting 2"), StrEq("Value for setting 2"));

    offlineStorage->Shutdown();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    offlineStorage->Initialize(observerMock);

    EXPECT_THAT(offlineStorage->GetSetting("setting 3"), StrEq("Value for setting 3"));
    EXPECT_THAT(offlineStorage->GetSetting("setting 1"), StrEq("Value for setting 1"));
    EXPECT_THAT(offlineStorage->GetSetting("setting 2"), StrEq("Value for setting 2"));
}

TEST_F(OfflineStorageTests_SQLite, UpdatedSettingsAreCorrectlyStored)
{
    offlineStorage->StoreSetting("setting 1", "Value for setting 1");
    offlineStorage->StoreSetting("setting 2", "Value for setting 2");

    offlineStorage->Shutdown();
    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    offlineStorage->Initialize(observerMock);

    EXPECT_THAT(offlineStorage->GetSetting("setting 1"), StrEq("Value for setting 1"));
    EXPECT_THAT(offlineStorage->GetSetting("setting 2"), StrEq("Value for setting 2"));
    offlineStorage->StoreSetting("setting 1", "");
    offlineStorage->StoreSetting("setting 2", "Value for setting 2 [UPDATED]");
    EXPECT_THAT(offlineStorage->GetSetting("setting 1"), StrEq(""));
    EXPECT_THAT(offlineStorage->GetSetting("setting 2"), StrEq("Value for setting 2 [UPDATED]"));
}

TEST_F(OfflineStorageTests_SQLite, APICallsAreHarmlessAfterStorageIsShutdown)
{
    offlineStorage->Shutdown();

    offlineStorage->Shutdown();
    HttpHeaders test;
    bool fromMemory = false;
    offlineStorage->DeleteRecords({ "1", "2", "" }, test, fromMemory);
    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), false);
    fromMemory = false;
    offlineStorage->ReleaseRecords({ "1", "2", "" }, true, test, fromMemory);
    offlineStorage->StoreRecord({"guid-1", "token", EventLatency_Normal, EventPersistence_Normal, 1, {}});
    offlineStorage->StoreSetting("name", "value");
    EXPECT_THAT(offlineStorage->GetSetting("name"), StrEq(""));

    EXPECT_CALL(observerMock, OnStorageOpened("SQLite/Default"));
    offlineStorage->Initialize(observerMock);
}

TEST_F(OfflineStorageTests_SQLite, ExceededStorageSizeCausesDbToDropOldestEventsWithLowestPriority)
{
    EXPECT_CALL(configMock, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(Return(5 * 1024 * 1024)); // 5M
    EXPECT_CALL(configMock, GetOfflineStorageResizeThresholdPct()).WillOnce(Return(60)); // 60% = 3 of 5

    ASSERT_THAT(offlineStorage->StoreRecord({"oldest with high prio", "token", EventLatency_RealTime, EventPersistence_Critical,   1, StorageBlob(1024 * 1024)}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"oldest with mid prio1", "token", EventLatency_Normal, EventPersistence_Normal, 2, StorageBlob(1024 * 1024)}), true); // X
    ASSERT_THAT(offlineStorage->StoreRecord({"some more mid prio e1", "token", EventLatency_Normal, EventPersistence_Normal, 3, StorageBlob(1024 * 1024)}), true);
    ASSERT_THAT(offlineStorage->StoreRecord({"oldest with low prio ", "token", EventLatency_Normal, EventPersistence_Normal,    4, StorageBlob(1024 * 1024)}), true); // X
   

    std::map<std::string, size_t> trimedRecord;
    trimedRecord["token"] = 3;
    // This should exceed storage size and trigger resize
    EXPECT_CALL(observerMock, OnStorageTrimmed(trimedRecord));
    ASSERT_THAT(offlineStorage->StoreRecord({"newest with low prio", "token", EventLatency_Normal, EventPersistence_Normal, 5, StorageBlob(1024 * 1024)}), true); // X

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000, EventLatency_RealTime), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].id, StrEq("oldest with high prio"));
    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000, EventLatency_Normal), true);
    ASSERT_THAT(consumer.records.size(), 1);
    EXPECT_THAT(consumer.records[0].id, StrEq("newest with low prio"));
    consumer.records.clear();
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000, EventLatency_Normal), true);
    ASSERT_THAT(consumer.records.size(), 0);
}

TEST_F(OfflineStorageTests_SQLite, TrimmingAlwaysDropsAtLeastOneEvent)
{
    EXPECT_CALL(configMock, GetOfflineStorageMaximumSizeBytes())
        .WillRepeatedly(Return(100 * 1024)); // 100 KB
    EXPECT_CALL(configMock, GetOfflineStorageResizeThresholdPct())
        .WillOnce(Return(10)); // 10% = 10 KB, not even one 33 KB event
    std::map<std::string, size_t> trimedRecord;
    trimedRecord["token"] = 1;
    EXPECT_CALL(observerMock, OnStorageTrimmed(trimedRecord));

    ASSERT_THAT(offlineStorage->StoreRecord({"old", "token", EventLatency_Normal, EventPersistence_Normal, 1, StorageBlob(33 * 1024)}), true); // X
    ASSERT_THAT(offlineStorage->StoreRecord({"mid", "token", EventLatency_Normal, EventPersistence_Normal, 2, StorageBlob(33 * 1024)}), true);
    // The next call triggers the trimming (after the insertion is done) and
    // removes the oldest event marked with X above.
    ASSERT_THAT(offlineStorage->StoreRecord({"new", "token", EventLatency_Normal, EventPersistence_Normal, 3, StorageBlob(33 * 1024)}), true);

    TestRecordConsumer consumer;
    EXPECT_THAT(offlineStorage->GetAndReserveRecords(consumer, 100000), true);
    ASSERT_THAT(consumer.records.size(), 2);
    EXPECT_THAT(consumer.records[0].id, StrEq("mid"));
    EXPECT_THAT(consumer.records[1].id, StrEq("new"));
}
#endif

