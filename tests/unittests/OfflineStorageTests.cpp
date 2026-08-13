// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "common/MockIOfflineStorage.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "NullObjects.hpp"
#include "offline/OfflineStorageHandler.hpp"
#include "offline/StorageObserver.hpp"

#include <stdexcept>

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

namespace MAT_NS_BEGIN
{
    class OfflineStorageHandlerTests : public ::testing::Test
    {
    protected:
        class NoCheckpointRuntimeConfig final : public testing::MockIRuntimeConfig
        {
        public:
            bool HasConfig(const char*) override
            {
                return false;
            }
        };

        class CountingLogManager final : public NullLogManager
        {
        public:
            bool StartActivity() override
            {
                ++activeActivities;
                return true;
            }

            void EndActivity() override
            {
                --activeActivities;
            }

            int activeActivities = 0;
        };

        class PausedLogManager final : public NullLogManager
        {
        public:
            bool StartActivity() override
            {
                ++startActivityCalls;
                return false;
            }

            int startActivityCalls = 0;
        };

        class NoopTaskDispatcher final : public ITaskDispatcher
        {
        public:
            void Join() override {}
            void Queue(Task*) override {}
            bool Cancel(Task*, uint64_t = 0) override { return true; }
        };

        class ThrowingTaskDispatcher final : public ITaskDispatcher
        {
        public:
            void Join() override {}

            void Queue(Task* task) override
            {
                std::unique_ptr<Task> ownedTask(task);
                throw std::runtime_error("queue failed");
            }

            bool Cancel(Task*, uint64_t = 0) override { return true; }
        };

        class DroppingTaskDispatcher final : public ITaskDispatcher
        {
        public:
            void Join() override {}
            void Queue(Task* task) override { delete task; }
            bool Cancel(Task*, uint64_t = 0) override { return true; }
        };

        static void MarkFlushPending(OfflineStorageHandler& handler)
        {
            handler.m_flushComplete.Reset();
            handler.m_flushPending = true;
        }

        static bool IsFlushPending(OfflineStorageHandler const& handler)
        {
            return handler.m_flushPending;
        }

        static bool IsFlushComplete(OfflineStorageHandler const& handler)
        {
            return handler.m_flushComplete.wait(0);
        }

        static testing::MockIOfflineStorage& InstallMemoryStorage(OfflineStorageHandler& handler)
        {
            auto storage = std::make_unique<StrictMock<testing::MockIOfflineStorage>>();
            auto* result = storage.get();
            handler.m_offlineStorageMemory = std::move(storage);
            handler.m_cacheMemorySizeLimitInBytes = 1;
            return *result;
        }

        static testing::MockIOfflineStorage& InstallDiskStorage(OfflineStorageHandler& handler)
        {
            auto storage = std::make_shared<StrictMock<testing::MockIOfflineStorage>>();
            auto* result = storage.get();
            handler.m_offlineStorageDisk = std::move(storage);
            return *result;
        }

        static void SetObserver(
            OfflineStorageHandler& handler,
            testing::MockIOfflineStorageObserver& observer)
        {
            handler.m_observer = &observer;
        }

        static bool CanLockFlushState(OfflineStorageHandler& handler)
        {
            if (!handler.m_flushLock.try_lock())
            {
                return false;
            }
            handler.m_flushLock.unlock();
            return true;
        }
    };

    TEST_F(OfflineStorageHandlerTests, FlushExceptionRestoresCompletionState)
    {
        CountingLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        auto& memoryStorage = InstallMemoryStorage(handler);
        MarkFlushPending(handler);
        EXPECT_CALL(memoryStorage, GetSize())
            .WillOnce(Throw(std::runtime_error("flush failed")));

        EXPECT_THROW(handler.Flush(), std::runtime_error);

        EXPECT_FALSE(IsFlushPending(handler));
        EXPECT_TRUE(IsFlushComplete(handler));
        EXPECT_EQ(logManager.activeActivities, 0);
    }

    TEST_F(OfflineStorageHandlerTests, SchedulingExceptionDoesNotPublishPendingFlush)
    {
        NullLogManager logManager;
        testing::MockIRuntimeConfig config;
        ThrowingTaskDispatcher taskDispatcher;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        auto& memoryStorage = InstallMemoryStorage(handler);
        StorageRecord record(
            "id",
            "tenant-token",
            EventLatency_Normal,
            EventPersistence_Normal,
            1234567890,
            std::vector<uint8_t>{});

        EXPECT_CALL(memoryStorage, GetSize()).WillOnce(Return(2));
        EXPECT_CALL(memoryStorage, StoreRecord(Ref(record))).WillOnce(Return(true));

        EXPECT_THROW(handler.StoreRecord(record), std::runtime_error);

        EXPECT_FALSE(IsFlushPending(handler));
        EXPECT_TRUE(CanLockFlushState(handler));
    }

    TEST_F(OfflineStorageHandlerTests, DroppedTaskDoesNotPublishPendingFlush)
    {
        NullLogManager logManager;
        testing::MockIRuntimeConfig config;
        DroppingTaskDispatcher taskDispatcher;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        auto& memoryStorage = InstallMemoryStorage(handler);
        StorageRecord record(
            "id",
            "tenant-token",
            EventLatency_Normal,
            EventPersistence_Normal,
            1234567890,
            std::vector<uint8_t>{});

        EXPECT_CALL(memoryStorage, GetSize()).WillOnce(Return(2));
        EXPECT_CALL(memoryStorage, StoreRecord(Ref(record))).WillOnce(Return(true));

        EXPECT_TRUE(handler.StoreRecord(record));

        EXPECT_FALSE(IsFlushPending(handler));
        EXPECT_TRUE(CanLockFlushState(handler));
    }

    TEST_F(OfflineStorageHandlerTests, ShutdownFlushesMemoryAfterActivityPause)
    {
        PausedLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        auto& memoryStorage = InstallMemoryStorage(handler);
        auto& diskStorage = InstallDiskStorage(handler);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        SetObserver(handler, observer);
        std::vector<StorageRecord> records {
            StorageRecord(
                "persisted-id",
                "tenant-token",
                EventLatency_Normal,
                EventPersistence_Normal,
                1234567890,
                std::vector<uint8_t>{1}),
            StorageRecord(
                "memory-only-id",
                "tenant-token",
                EventLatency_Normal,
                EventPersistence_DoNotStoreOnDisk,
                1234567891,
                std::vector<uint8_t>{1})
        };

        EXPECT_CALL(memoryStorage, GetSize())
            .WillOnce(Return(1))
            .WillOnce(Return(0));
        EXPECT_CALL(memoryStorage, GetRecords(false, EventLatency_Unspecified, _))
            .WillOnce(Return(records));
        EXPECT_CALL(diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](const std::vector<StorageRecord>& persistedRecords)
            {
                EXPECT_THAT(persistedRecords, SizeIs(1));
                EXPECT_EQ(persistedRecords.front().id, "persisted-id");
                return persistedRecords.size();
            }));
        EXPECT_CALL(memoryStorage, DeleteRecords(_, _, _));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1));
        EXPECT_CALL(memoryStorage, Shutdown());
        EXPECT_CALL(diskStorage, Shutdown());

        handler.Shutdown();

        EXPECT_EQ(logManager.startActivityCalls, 0);
    }
} MAT_NS_END
