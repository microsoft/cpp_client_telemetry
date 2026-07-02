// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIOfflineStorage.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "offline/OfflineStorageHandler.hpp"
#include "offline/StorageObserver.hpp"
#include "NullObjects.hpp"

#include <cstdio>
#include <sstream>

using namespace testing;
using namespace MAT;

class OfflineStorageTests : public StrictMock<Test> {
protected:
    MockIOfflineStorage     offlineStorageMock;
    StorageObserver         offlineStorage;

    RouteSink<OfflineStorageTests, IncomingEventContextPtr const&>                             storeRecordFailed{ this, &OfflineStorageTests::resultStoreRecordFailed };
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&, StorageRecord const&, bool&> retrievedEvent{ this, &OfflineStorageTests::resultRetrievedEvent };
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&>                              retrievalFinished{ this, &OfflineStorageTests::resultRetrievalFinished };
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&>                              retrievalFailed{ this, &OfflineStorageTests::resultRetrievalFailed };

    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          opened{ this, &OfflineStorageTests::notifOpened };
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          failed{ this, &OfflineStorageTests::notifFailed };
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          trimmed{ this, &OfflineStorageTests::notifTrimmed };
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          recordsDropped{ this, &OfflineStorageTests::notifRecordsDropped };

protected:
    OfflineStorageTests()
        : offlineStorage(testing::getSystem(), offlineStorageMock)
    {
        offlineStorage.storeRecordFailed >> storeRecordFailed;
        offlineStorage.retrievedEvent >> retrievedEvent;
        offlineStorage.retrievalFinished >> retrievalFinished;
        offlineStorage.retrievalFailed >> retrievalFailed;

        offlineStorage.opened >> opened;
        offlineStorage.failed >> failed;
        offlineStorage.trimmed >> trimmed;
        offlineStorage.recordsDropped >> recordsDropped;
    }

    MOCK_METHOD1(resultStoreRecordFailed, void(IncomingEventContextPtr const &));
    MOCK_METHOD3(resultRetrievedEvent, void(EventsUploadContextPtr const &, StorageRecord const &, bool&));
    MOCK_METHOD1(resultRetrievalFinished, void(EventsUploadContextPtr const &));
    MOCK_METHOD1(resultRetrievalFailed, void(EventsUploadContextPtr const &));

    MOCK_METHOD1(notifOpened, void(StorageNotificationContext const*));
    MOCK_METHOD1(notifFailed, void(StorageNotificationContext const*));
    MOCK_METHOD1(notifTrimmed, void(StorageNotificationContext const*));
    MOCK_METHOD1(notifRecordsDropped, void(StorageNotificationContext const*));
};


TEST_F(OfflineStorageTests, StartInitializes)
{
    EXPECT_CALL(offlineStorageMock, Initialize(Ref(offlineStorage)))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.start(), true);
}

TEST_F(OfflineStorageTests, StopShutsDown)
{
    EXPECT_CALL(offlineStorageMock, Shutdown())
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.stop(), true);
}

TEST_F(OfflineStorageTests, StoreRecordIsForwarded)
{
    auto ctx = new IncomingEventContext();

    EXPECT_CALL(offlineStorageMock, StoreRecord(Ref(ctx->record)))
        .WillOnce(Return(true));
    EXPECT_THAT(offlineStorage.storeRecord(ctx), true);
    EXPECT_THAT(ctx->record.timestamp, Near(PAL::getUtcSystemTimeMs(), 1000));

    EXPECT_CALL(offlineStorageMock, StoreRecord(Ref(ctx->record)))
        .WillOnce(Return(false));
    EXPECT_CALL(*this, resultStoreRecordFailed(ctx))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.storeRecord(ctx), false);
}

TEST_F(OfflineStorageTests, RetrieveEventsPassesRecordsThrough)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->requestedMinLatency = EventLatency_Normal;
    ctx->requestedMaxCount = 6;

    StorageRecord record1("r1", "tenant1-token", EventLatency_Normal, EventPersistence_Normal, 1234567890, std::vector<uint8_t>{1, 127, 255});
    StorageRecord record2("r2", "tenant2-token", EventLatency_Normal, EventPersistence_Normal, 1234567891, std::vector<uint8_t>{2, 128, 0});
    EXPECT_CALL(offlineStorageMock, GetAndReserveRecords(_, Gt(1000u), ctx->requestedMinLatency, ctx->requestedMaxCount))
        .WillOnce(DoAll(
            Invoke([&record1, &record2](std::function<bool(StorageRecord&&)> const& consumer, unsigned, EventLatency, unsigned) {
        EXPECT_THAT(consumer(std::move(record1)), true);
        EXPECT_THAT(consumer(std::move(record2)), false);
    }),
            Return(true)))
        .RetiresOnSaturation();

    EXPECT_CALL(offlineStorageMock, IsLastReadFromMemory())
        .WillOnce(Return(false));

    EXPECT_CALL(*this, resultRetrievedEvent(ctx, Ref(record1), _))
        .WillOnce(SetArgReferee<2>(true))
        .RetiresOnSaturation();
    EXPECT_CALL(*this, resultRetrievedEvent(ctx, Ref(record2), _))
        .WillOnce(SetArgReferee<2>(false))
        .RetiresOnSaturation();
    EXPECT_CALL(*this, resultRetrievalFinished(ctx))
        .WillOnce(Return());



    offlineStorage.retrieveEvents(ctx);
}

TEST_F(OfflineStorageTests, RetrieveEventsFailureAborts)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    ctx->requestedMinLatency = EventLatency_Normal;
    ctx->requestedMaxCount = 6;

    EXPECT_CALL(offlineStorageMock, GetAndReserveRecords(_, Gt(1000u), ctx->requestedMinLatency, ctx->requestedMaxCount))
        .WillOnce(Return(false));
    EXPECT_CALL(offlineStorageMock, IsLastReadFromMemory())
        .WillOnce(Return(false));
    EXPECT_CALL(*this, resultRetrievalFailed(ctx))
        .WillOnce(Return());
    offlineStorage.retrieveEvents(ctx);
}

TEST_F(OfflineStorageTests, DeleteRecordsIsForwarded)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    HttpHeaders test;
    bool fromMemory = false;
    std::vector<std::string> recordIds;
    for (const auto& element : ctx->recordIdsAndTenantIds)
    {
        recordIds.push_back(element.first);
    }
    ctx->fromMemory = fromMemory;
    EXPECT_CALL(offlineStorageMock, DeleteRecords(recordIds, test, fromMemory)).WillOnce(Return());
    EXPECT_THAT(offlineStorage.deleteRecords(ctx), true);
}

TEST_F(OfflineStorageTests, ReleaseRecordsIsForwarded)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    HttpHeaders test;
    bool fromMemory = false;
    std::vector<std::string> recordIds;
    for (const auto& element : ctx->recordIdsAndTenantIds)
    {
        recordIds.push_back(element.first);
    }
    ctx->fromMemory = fromMemory;
    EXPECT_CALL(offlineStorageMock, ReleaseRecords(recordIds, false, test, fromMemory))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.releaseRecords(ctx), true);
    fromMemory = false;
    EXPECT_CALL(offlineStorageMock, ReleaseRecords(recordIds, true, test, fromMemory))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.releaseRecordsIncRetryCount(ctx), true);
}

namespace
{
    // Remove a SQLite db file along with its WAL-mode companion files
    // (-wal/-shm/-journal), which would otherwise accumulate in the temp dir.
    void RemoveDbFiles(const std::string& path)
    {
        std::remove(path.c_str());
        std::remove((path + "-wal").c_str());
        std::remove((path + "-shm").c_str());
        std::remove((path + "-journal").c_str());
    }

    // No-op dispatcher that owns queued tasks and frees them, so flushes only
    // run when invoked directly and scheduled tasks (if any) are not leaked.
    class NoopTaskDispatcher : public ITaskDispatcher
    {
    public:
        void Join() override { clear(); }
        void Queue(Task* task) override { m_tasks.push_back(task); }
        bool Cancel(Task* task, uint64_t waitTime = 0) override
        {
            UNREFERENCED_PARAMETER(waitTime);
            auto it = std::find(m_tasks.begin(), m_tasks.end(), task);
            if (it != m_tasks.end())
            {
                delete *it;
                m_tasks.erase(it);
                return true;
            }
            return false;
        }
        ~NoopTaskDispatcher() override { clear(); }

    private:
        void clear()
        {
            for (auto* t : m_tasks)
                delete t;
            m_tasks.clear();
        }
        std::vector<Task*> m_tasks;
    };
}

// Regression test: when records drained from the in-memory queue fail to be
// stored by the disk backend during Flush() (StoreRecord() returns false), they
// must be returned to the queue rather than lost.
TEST(OfflineStorageHandlerFlushTests, FailedDiskStoreDuringFlushReturnsRecordsToMemory)
{
    NullLogManager logManager;
    NiceMock<MockIRuntimeConfig> config;
    NoopTaskDispatcher dispatcher;
    NiceMock<MockIOfflineStorageObserver> observer;

    ON_CALL(config, GetOfflineStorageMaximumSizeBytes()).WillByDefault(Return(32 * 4096));
    ON_CALL(config, GetMaximumRetryCount()).WillByDefault(Return(5));

    std::ostringstream dbPath;
    dbPath << GetTempDirectory() << "FlushReserveTest-" << PAL::getUtcSystemTimeMs() << ".db";
    RemoveDbFiles(dbPath.str());
    config[CFG_STR_CACHE_FILE_PATH] = dbPath.str();
    config[CFG_INT_RAM_QUEUE_SIZE] = 1024 * 1024;  // enable the in-memory queue

    OfflineStorageHandler handler(logManager, config, dispatcher);
    handler.Initialize(observer);

    // A timestamp <= 0 is accepted by the in-memory queue but rejected by the
    // SQLite disk store's input validation, so its StoreRecord() returns false.
    // This drives the same Flush() failure-handling path as any disk store
    // failure (a failed record must be returned to memory, not dropped).
    const size_t kCount = 5;
    for (size_t i = 0; i < kCount; i++)
    {
        StorageRecord r("flush-id-" + std::to_string(i), "tenant-token",
            EventLatency_Normal, EventPersistence_Normal, /*timestamp*/ 0,
            std::vector<uint8_t>{ 'x' });
        handler.StoreRecord(r);
    }
    EXPECT_EQ(handler.GetRecordCount(), kCount);

    handler.Flush();

    // The disk rejected every record; with the fix they are returned to the
    // in-memory queue rather than silently dropped.
    EXPECT_EQ(handler.GetRecordCount(), kCount);

    handler.Shutdown();
    RemoveDbFiles(dbPath.str());
}
