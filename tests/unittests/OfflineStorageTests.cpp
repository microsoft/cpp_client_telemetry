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
        class ConfigurableLogManager : public NullLogManager
        {
        public:
            ILogConfiguration& GetLogConfiguration() override
            {
                return m_configuration;
            }

        private:
            ILogConfiguration m_configuration;
        };

        class NoCheckpointRuntimeConfig final : public testing::MockIRuntimeConfig
        {
        public:
            bool HasConfig(const char*) override
            {
                return false;
            }
        };

        class CountingLogManager final : public ConfigurableLogManager
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

        class PausedLogManager final : public ConfigurableLogManager
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
                ++queueCalls;
                std::unique_ptr<Task> ownedTask(task);
                throw std::runtime_error("queue failed");
            }

            bool Cancel(Task*, uint64_t = 0) override { return true; }

            int queueCalls = 0;
        };

        class DroppingTaskDispatcher final : public ITaskDispatcher
        {
        public:
            void Join() override {}
            void Queue(Task* task) override
            {
                ++queueCalls;
                delete task;
            }
            bool Cancel(Task*, uint64_t = 0) override { return true; }

            int queueCalls = 0;
        };

        static void ConfigureMemoryCache(
            testing::MockIRuntimeConfig& config,
            uint32_t sizeInBytes)
        {
            config[CFG_INT_RAM_QUEUE_SIZE] = sizeInBytes;
            config[CFG_INT_RAMCACHE_FULL_PCT] = 75;
        }

        static std::shared_ptr<StrictMock<testing::MockIOfflineStorage>>
        AttachDiskStorage(ConfigurableLogManager& logManager)
        {
            auto storage =
                std::make_shared<StrictMock<testing::MockIOfflineStorage>>();
            logManager.GetLogConfiguration().AddModule(
                CFG_MODULE_OFFLINE_STORAGE,
                storage);
            return storage;
        }

        static StorageRecord MakeRecord(
            const char* id,
            EventPersistence persistence = EventPersistence_Normal)
        {
            return StorageRecord(
                id,
                "tenant-token",
                EventLatency_Normal,
                persistence,
                1234567890,
                std::vector<uint8_t>{1});
        }
    };

    TEST_F(OfflineStorageHandlerTests, FlushExceptionReleasesActivityAndAllowsRetry)
    {
        CountingLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1024 * 1024);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("persisted-id")));
        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](std::vector<StorageRecord>& records) -> size_t
            {
                records.clear();
                throw std::runtime_error("flush failed");
            }));

        EXPECT_THROW(handler.Flush(), std::runtime_error);

        EXPECT_EQ(logManager.activeActivities, 0);
        EXPECT_CALL(*diskStorage, StoreRecords(_)).WillOnce(Return(1));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1));
        EXPECT_NO_THROW(handler.Flush());
        EXPECT_EQ(logManager.activeActivities, 0);
        EXPECT_CALL(*diskStorage, Shutdown());
        handler.Shutdown();
    }

    TEST_F(OfflineStorageHandlerTests, SchedulingExceptionAllowsAnotherFlushAttempt)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        ThrowingTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        ASSERT_TRUE(handler.StoreRecord(MakeRecord("first")));
        EXPECT_THROW(
            handler.StoreRecord(MakeRecord("second")),
            std::runtime_error);
        EXPECT_THROW(
            handler.StoreRecord(MakeRecord("third")),
            std::runtime_error);
        EXPECT_EQ(taskDispatcher.queueCalls, 2);
    }

    TEST_F(OfflineStorageHandlerTests, DroppedTaskAllowsAnotherFlushAttempt)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        DroppingTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        ASSERT_TRUE(handler.StoreRecord(MakeRecord("first")));
        EXPECT_TRUE(handler.StoreRecord(MakeRecord("second")));
        EXPECT_TRUE(handler.StoreRecord(MakeRecord("third")));
        EXPECT_EQ(taskDispatcher.queueCalls, 2);
    }

    TEST_F(OfflineStorageHandlerTests, PartialFlushRestoresBatchForRetry)
    {
        CountingLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1024 * 1024);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("first")));
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("second")));

        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](std::vector<StorageRecord>& records)
            {
                EXPECT_THAT(records, SizeIs(2));
                records.clear();
                return 1;
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1));
        handler.Flush();

        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](const std::vector<StorageRecord>& records)
            {
                EXPECT_THAT(records, SizeIs(2));
                return records.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(2));
        handler.Flush();

        EXPECT_CALL(*diskStorage, Shutdown());
        handler.Shutdown();
    }

    TEST_F(OfflineStorageHandlerTests, ShutdownFlushesMemoryAfterActivityPause)
    {
        PausedLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1024 * 1024);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("persisted-id")));
        ASSERT_TRUE(handler.StoreRecord(MakeRecord(
            "memory-only-id",
            EventPersistence_DoNotStoreOnDisk)));

        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](const std::vector<StorageRecord>& persistedRecords)
            {
                EXPECT_THAT(persistedRecords, SizeIs(1));
                EXPECT_EQ(persistedRecords.front().id, "persisted-id");
                return persistedRecords.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1));
        EXPECT_CALL(*diskStorage, Shutdown());

        handler.Shutdown();

        EXPECT_EQ(logManager.startActivityCalls, 0);
    }
} MAT_NS_END
