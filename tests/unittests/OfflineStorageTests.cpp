// Copyright (c) Microsoft Corporation. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "common/MockIOfflineStorage.hpp"
#include "common/MockIOfflineStorageObserver.hpp"
#include "NullObjects.hpp"
#include "offline/OfflineStorageHandler.hpp"
#include "pal/TaskDispatcher_CAPI.hpp"
#include "offline/StorageObserver.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <mutex>
#include <stdexcept>
#include <thread>

using namespace testing;
using namespace MAT;

namespace
{
    static void AssertQueuedImmediateCall(Task* task)
    {
        ASSERT_NE(task, nullptr);
        EXPECT_EQ(task->Type, Task::Call);
        EXPECT_EQ(task->TargetTime, 0u);
        EXPECT_EQ(task->TypeName, "OfflineStorageFlushTask");
    }
}

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
                AssertQueuedImmediateCall(task);
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
                AssertQueuedImmediateCall(task);
                ++queueCalls;
                delete task;
            }
            bool Cancel(Task*, uint64_t = 0) override { return true; }

            int queueCalls = 0;
        };

        // A one-shot, two-phase rendezvous used to make cross-thread ordering in the
        // concurrency tests below deterministic instead of sleep-based. Arrive()/
        // WaitForArrival() prove that one thread has reached a specific point in the
        // code (typically inside a mocked storage call, holding the flush I/O lock);
        // Release()/WaitForRelease() let the test control precisely when that thread
        // is allowed to continue. WaitForArrival() uses a bounded wait so a defect
        // that never reaches the expected point fails the test instead of hanging it.
        class Rendezvous
        {
        public:
            void Arrive()
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_arrived = true;
                }
                m_cv.notify_all();
            }

            bool WaitForArrival(std::chrono::milliseconds timeout)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                return m_cv.wait_for(lock, timeout, [this] { return m_arrived; });
            }

            void Release()
            {
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_released = true;
                }
                m_cv.notify_all();
            }

            void WaitForRelease()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [this] { return m_released; });
            }

        private:
            std::mutex m_mutex;
            std::condition_variable m_cv;
            bool m_arrived = false;
            bool m_released = false;
        };

        // A task dispatcher that queues tasks without ever running them
        // automatically, so a test can decide exactly when/where a "scheduled" flush
        // executes. RunNext() runs the oldest still-queued task synchronously on the
        // calling thread (standing in for the real worker thread). Cancel() mirrors
        // the real WorkerThread's queued-and-not-started case (erase + delete) so a
        // test can assert that the fixed implementation never calls it at all.
        class ControllableTaskDispatcher final : public ITaskDispatcher
        {
        public:
            void Join() override {}

            void Queue(Task* task) override
            {
                AssertQueuedImmediateCall(task);
                // Optional hook fired BEFORE the task is enqueued, while the handler
                // still holds its flush-state lock inside StoreRecord()'s scheduling
                // step. A test uses this to freeze a StoreRecord() that has already
                // passed admission at the exact "about to schedule" point.
                std::function<void()> hook;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    hook = beforeQueue;
                }
                if (hook)
                {
                    hook();
                }
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push_back(task);
                ++queueCalls;
            }

            bool Cancel(Task* task, uint64_t = 0) override
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                ++cancelCalls;
                auto it = std::find(m_queue.begin(), m_queue.end(), task);
                if (it == m_queue.end())
                {
                    return false;
                }
                delete *it;
                m_queue.erase(it);
                return true;
            }

            bool RunNext()
            {
                Task* task = nullptr;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_queue.empty())
                    {
                        return false;
                    }
                    task = m_queue.front();
                    m_queue.pop_front();
                }
                std::unique_ptr<Task> owned(task);
                (*owned)();
                return true;
            }

            size_t PendingCount()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_queue.size();
            }

            std::atomic<int> queueCalls{0};
            std::atomic<int> cancelCalls{0};

            // Set (before any concurrent Queue() call) to freeze the scheduling
            // thread inside Queue(); guarded by m_mutex on read.
            std::function<void()> beforeQueue;

        private:
            std::mutex m_mutex;
            std::list<Task*> m_queue;
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

    namespace
    {
        class CapiTaskProbe
        {
        public:
            void OnQueue(evt_task_t* task, task_callback_fn_t callback)
            {
                ++queueCalls;
                ASSERT_NE(task, nullptr);
                ASSERT_NE(task->typeName, nullptr);
                EXPECT_EQ(task->delayMs, 0);
                EXPECT_STREQ(task->typeName, "OfflineStorageFlushTask");
                callback(task->id);
            }

            bool OnCancel(const char*)
            {
                ++cancelCalls;
                return true;
            }

            void OnJoin()
            {
            }

            int queueCalls = 0;
            int cancelCalls = 0;
        };

        static std::unique_ptr<CapiTaskProbe> s_capiTaskProbe;

        class AutoCapiTaskProbe
        {
        public:
            AutoCapiTaskProbe()
            {
                s_capiTaskProbe.reset(new CapiTaskProbe());
            }

            ~AutoCapiTaskProbe()
            {
                s_capiTaskProbe = nullptr;
            }

            CapiTaskProbe* operator->()
            {
                return s_capiTaskProbe.get();
            }
        };

        void EVTSDK_LIBABI_CDECL OnCapiTaskQueue(evt_task_t* task, task_callback_fn_t callback)
        {
            s_capiTaskProbe->OnQueue(task, callback);
        }

        bool EVTSDK_LIBABI_CDECL OnCapiTaskCancel(const char* taskId)
        {
            return s_capiTaskProbe->OnCancel(taskId);
        }

        void EVTSDK_LIBABI_CDECL OnCapiTaskJoin()
        {
            s_capiTaskProbe->OnJoin();
        }
    }

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

    TEST_F(OfflineStorageHandlerTests, ScheduledFlushUsesCapiImmediateCallSemantics)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        ConfigureMemoryCache(config, 1);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        AutoCapiTaskProbe taskProbe;
        PAL::TaskDispatcher_CAPI taskDispatcher(
            &OnCapiTaskQueue,
            &OnCapiTaskCancel,
            &OnCapiTaskJoin);
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        ASSERT_TRUE(handler.StoreRecord(MakeRecord("first")));
        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .Times(AtLeast(1))
            .WillRepeatedly(Invoke([](std::vector<StorageRecord>& records) -> size_t
            {
                return records.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(_)).Times(AtLeast(1));
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("second")));
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("third")));

        EXPECT_GE(taskProbe->queueCalls, 1);
        EXPECT_EQ(taskProbe->cancelCalls, 0);

        EXPECT_CALL(*diskStorage, Shutdown());
        handler.Shutdown();
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

    TEST_F(OfflineStorageHandlerTests, FlushKeepsMemoryOnlyRecordsInMemory)
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
        ASSERT_TRUE(handler.StoreRecord(MakeRecord(
            "memory-only-id",
            EventPersistence_DoNotStoreOnDisk)));

        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](const std::vector<StorageRecord>& records)
            {
                EXPECT_THAT(records, SizeIs(1));
                if (!records.empty())
                {
                    EXPECT_EQ(records.front().id, "persisted-id");
                }
                return records.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1));
        handler.Flush();

        std::vector<StorageRecord> retrievedRecords;
        ASSERT_TRUE(handler.GetAndReserveRecords(
            [&retrievedRecords](StorageRecord&& record)
            {
                retrievedRecords.push_back(std::move(record));
                return true;
            },
            0,
            EventLatency_Unspecified,
            1));
        ASSERT_THAT(retrievedRecords, SizeIs(1));
        EXPECT_EQ(retrievedRecords.front().id, "memory-only-id");
        EXPECT_EQ(
            retrievedRecords.front().persistence,
            EventPersistence_DoNotStoreOnDisk);

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

    TEST_F(OfflineStorageHandlerTests, DirectFlushBeforeShutdownBlocksTeardown)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1024 * 1024);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        auto handler = std::unique_ptr<OfflineStorageHandler>(
            new OfflineStorageHandler(logManager, config, taskDispatcher));
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler->Initialize(observer);
        ASSERT_TRUE(handler->StoreRecord(MakeRecord("persisted-id")));

        Rendezvous flushGate;
        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([&flushGate](std::vector<StorageRecord>& records) -> size_t
            {
                size_t saved = records.size();
                flushGate.Arrive();
                flushGate.WaitForRelease();
                return saved;
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1));
        EXPECT_CALL(*diskStorage, Shutdown());

        // A direct Flush() call, as could happen concurrently from an HTTP
        // completion callback or the transmission policy manager, blocked in the
        // middle of storage I/O.
        std::thread flushThread([&handler]() { handler->Flush(); });

        ASSERT_TRUE(flushGate.WaitForArrival(std::chrono::seconds(5)))
            << "Direct Flush() never reached storage I/O";

        std::promise<void> shutdownDone;
        std::future<void> shutdownDoneFuture = shutdownDone.get_future();
        std::thread shutdownThread([&handler, &shutdownDone]()
        {
            handler->Shutdown();
            shutdownDone.set_value();
        });

        EXPECT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::milliseconds(200)),
            std::future_status::timeout)
            << "Shutdown returned before the in-flight direct Flush completed";

        flushGate.Release();
        flushThread.join();

        ASSERT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::seconds(5)),
            std::future_status::ready)
            << "Shutdown never completed after the direct Flush finished";
        shutdownThread.join();
        handler.reset();
    }

    // O4 regression test #2: an older scheduled flush generation (N) that is still
    // completing must never cancel, clear, or otherwise interfere with a newer
    // scheduled generation (N+1) that was queued while N (and an unrelated direct
    // Flush(), D) were still in flight. With the pre-fix code, FlushImpl()
    // unconditionally cancelled/cleared the single shared m_flushHandle/
    // m_flushPending on every completion, so N's belated completion (racing with D)
    // could destroy N+1's not-yet-started task outright.
    TEST_F(OfflineStorageHandlerTests, StaleScheduledFlushDoesNotClearNewerSchedule)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        ControllableTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        // "r1" alone never crosses the (1 byte) threshold measured *before* the
        // store; it primes the memory cache so the next store does.
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("r1")));
        // Crosses the threshold: schedules generation N. Not run yet.
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("r2")));
        ASSERT_EQ(taskDispatcher.queueCalls, 1);
        ASSERT_EQ(taskDispatcher.PendingCount(), 1u);

        Rendezvous directFlushGate;
        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .Times(AtLeast(2))
            .WillOnce(Invoke([&directFlushGate](std::vector<StorageRecord>& records) -> size_t
            {
                size_t saved = records.size();
                directFlushGate.Arrive();
                directFlushGate.WaitForRelease();
                return saved;
            }))
            .WillRepeatedly(Invoke([](std::vector<StorageRecord>& records) -> size_t
            {
                return records.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(_)).Times(AtLeast(2));
        EXPECT_CALL(*diskStorage, Shutdown());

        // A concurrent *direct* Flush() (D) -- e.g. from an HTTP completion callback
        // -- blocked mid-storage-I/O while generation N is still queued and has not
        // started.
        std::thread directFlushThread([&handler]() { handler.Flush(); });
        ASSERT_TRUE(directFlushGate.WaitForArrival(std::chrono::seconds(5)))
            << "Direct Flush() (D) never reached storage I/O";

        // D has registered its own generation and is holding the flush I/O lock.
        // With the O4 defect, D's FlushImpl() would unconditionally call
        // m_flushHandle.Cancel() here and delete N's still-queued task outright. The
        // fix never cancels another generation's task at all.
        EXPECT_EQ(taskDispatcher.cancelCalls, 0);
        ASSERT_EQ(taskDispatcher.PendingCount(), 1u)
            << "D must not cancel/clear the still-queued generation N";

        // Run N on a background thread (standing in for the real worker thread): it
        // must vacate the single scheduled-generation slot before doing any storage
        // I/O, then block on the flush I/O lock behind D.
        std::thread scheduledFlushThread([&taskDispatcher]() { taskDispatcher.RunNext(); });

        // The slot vacates the instant N starts running -- a fast, lock-only step
        // with no I/O -- well before D's gate will be released, so this retry loop
        // is bounded by wall-clock time but is polling for an actual, guaranteed-fast
        // state transition rather than guessing a sleep duration.
        bool scheduledSecondGeneration = false;
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        int extraRecordId = 0;
        while (!scheduledSecondGeneration && std::chrono::steady_clock::now() < deadline)
        {
            ASSERT_TRUE(handler.StoreRecord(
                MakeRecord(("extra" + std::to_string(extraRecordId++)).c_str())));
            if (taskDispatcher.queueCalls == 2)
            {
                scheduledSecondGeneration = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        ASSERT_TRUE(scheduledSecondGeneration)
            << "N+1 was never scheduled while N was still executing";
        ASSERT_EQ(taskDispatcher.PendingCount(), 1u)
            << "N+1 must be queued and distinct from N, which is mid-flight";

        // Let D finish; its completion must remove only its own generation.
        directFlushGate.Release();
        directFlushThread.join();

        // N can now acquire the flush I/O lock and complete. Its completion must
        // remove only its own generation, never touching N+1.
        scheduledFlushThread.join();
        EXPECT_EQ(taskDispatcher.cancelCalls, 0)
            << "No generation may ever be cancelled by another generation's completion";
        ASSERT_EQ(taskDispatcher.PendingCount(), 1u)
            << "Stale generation N's completion must not clear/remove N+1";

        // Give N+1 something to flush.
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("post-n")));

        // Shutdown() must still wait: N+1 has neither started nor completed yet.
        std::promise<void> shutdownDone;
        std::future<void> shutdownDoneFuture = shutdownDone.get_future();
        std::thread shutdownThread([&handler, &shutdownDone]()
        {
            handler.Shutdown();
            shutdownDone.set_value();
        });
        EXPECT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::milliseconds(200)),
            std::future_status::timeout)
            << "Shutdown() returned before scheduled generation N+1 executed";

        // Run N+1: only after this does Shutdown() unblock.
        ASSERT_TRUE(taskDispatcher.RunNext());

        ASSERT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::seconds(5)),
            std::future_status::ready)
            << "Shutdown() never completed after N+1 executed";
        shutdownThread.join();
        EXPECT_EQ(taskDispatcher.cancelCalls, 0);
    }

    // Revised-O4 regression test #1 (admission gate). Reproduces the exact race the
    // reviewer flagged: a StoreRecord() call passes admission and then schedules a
    // flush *after* Shutdown()'s drain would previously have returned. Here the
    // scheduling thread is frozen at the precise "admitted, about to schedule" point
    // (inside the dispatcher's Queue(), while the handler still holds its flush-state
    // lock). Shutdown() must not tear storage down until (a) that store finishes
    // registering its scheduled generation and (b) that scheduled generation
    // actually runs -- so no task is ever dispatched onto closed storage.
    //
    // Against a design that gated admission with only an atomic<bool>, Shutdown()'s
    // wait would observe "no pending flush" (the generation is registered only after
    // this point) and return, letting the store schedule a flush onto storage that
    // was already shut down.
    TEST_F(OfflineStorageHandlerTests, StoreRecordAdmittedBeforeSchedulingGatesShutdown)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        ControllableTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        // The scheduled flush (once it is finally allowed to run) moves the memory
        // cache to disk exactly once, then Shutdown() shuts disk down.
        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](std::vector<StorageRecord>& records) -> size_t
            {
                return records.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(_)).Times(1);
        EXPECT_CALL(*diskStorage, Shutdown());

        // Freeze the scheduling StoreRecord() inside Queue(): the call has already
        // been admitted (registered in the store count) but has not yet finished
        // registering its scheduled generation.
        Rendezvous scheduleGate;
        taskDispatcher.beforeQueue = [&scheduleGate]()
        {
            scheduleGate.Arrive();
            scheduleGate.WaitForRelease();
        };

        ASSERT_TRUE(handler.StoreRecord(MakeRecord("r1")));

        std::promise<bool> storeDone;
        std::future<bool> storeDoneFuture = storeDone.get_future();
        std::thread storeThread([&handler, &storeDone]()
        {
            // Crosses the 1-byte threshold and therefore tries to schedule a flush;
            // it will block inside Queue() at the gate above.
            storeDone.set_value(handler.StoreRecord(MakeRecord("r2")));
        });

        ASSERT_TRUE(scheduleGate.WaitForArrival(std::chrono::seconds(5)))
            << "Admitted StoreRecord() never reached the scheduling step";

        // Shutdown() begins while the admitted store is frozen mid-schedule. It must
        // block: first because the store still holds the flush-state lock, then
        // because the scheduled generation it registers is still outstanding.
        std::promise<void> shutdownDone;
        std::future<void> shutdownDoneFuture = shutdownDone.get_future();
        std::thread shutdownThread([&handler, &shutdownDone]()
        {
            handler.Shutdown();
            shutdownDone.set_value();
        });
        EXPECT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::milliseconds(200)),
            std::future_status::timeout)
            << "Shutdown() proceeded while an admitted StoreRecord() was mid-schedule";

        // No task may have been dispatched to (soon-to-be-)closed storage yet: the
        // store is still frozen before enqueue completes.
        EXPECT_EQ(taskDispatcher.queueCalls, 0);
        EXPECT_EQ(taskDispatcher.PendingCount(), 0u);

        // Let the admitted store finish scheduling. Its generation is now registered,
        // so Shutdown() must STILL wait -- the scheduled flush has not run.
        scheduleGate.Release();
        ASSERT_EQ(
            storeDoneFuture.wait_for(std::chrono::seconds(5)),
            std::future_status::ready);
        EXPECT_TRUE(storeDoneFuture.get());
        storeThread.join();

        ASSERT_EQ(taskDispatcher.queueCalls, 1);
        ASSERT_EQ(taskDispatcher.PendingCount(), 1u)
            << "The admitted store's scheduled flush must be queued";
        EXPECT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::milliseconds(200)),
            std::future_status::timeout)
            << "Shutdown() returned before the admitted store's scheduled flush ran";

        // Run the scheduled flush. Only now, with storage still open, does the queued
        // task touch storage -- proving nothing was ever dispatched onto closed
        // storage. Its completion lets Shutdown() drain and tear storage down.
        ASSERT_TRUE(taskDispatcher.RunNext());
        ASSERT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::seconds(5)),
            std::future_status::ready)
            << "Shutdown() never completed after the scheduled flush ran";
        shutdownThread.join();

        EXPECT_EQ(taskDispatcher.cancelCalls, 0);
        EXPECT_EQ(taskDispatcher.PendingCount(), 0u);
    }

    TEST_F(OfflineStorageHandlerTests, StoreRecordAfterShutdownFailsWithoutStorageAccess)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1024 * 1024);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("persisted-id")));

        // Shutdown flushes the memory cache to disk once, then shuts disk down.
        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](const std::vector<StorageRecord>& records)
            {
                return records.size();
            }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1));
        EXPECT_CALL(*diskStorage, Shutdown());
        handler.Shutdown();

        EXPECT_FALSE(handler.StoreRecord(MakeRecord("after-shutdown")));
        EXPECT_FALSE(handler.StoreRecord(
            MakeRecord("after-shutdown-mem", EventPersistence_DoNotStoreOnDisk)));
    }

    TEST_F(OfflineStorageHandlerTests, EmptyDeleteIdsDoNotAccessStorage)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        config[CFG_INT_RAM_QUEUE_SIZE] = 0;
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        std::vector<StorageRecordId> ids;
        HttpHeaders headers;
        bool fromMemory = false;
        EXPECT_NO_THROW(handler.DeleteRecords(ids, headers, fromMemory));

        EXPECT_CALL(*diskStorage, Shutdown());
        handler.Shutdown();
    }

    TEST_F(OfflineStorageHandlerTests, DirectFlushAfterAdmissionCloseIsNoOp)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        // No memory cache: StoreRecord() persists straight to the (mock) disk, which
        // gives us a clean, gate-able admitted store that holds no handler lock.
        // RuntimeConfig_Default supplies a non-zero default RAM queue size, so the
        // memory cache must be disabled explicitly.
        config[CFG_INT_RAM_QUEUE_SIZE] = 0;
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        Rendezvous admittedGate;
        // First (admitted) store: freezes inside the disk write, holding no handler
        // lock, keeping the store count non-zero so Shutdown() blocks in its drain.
        EXPECT_CALL(*diskStorage, StoreRecord(Field(&StorageRecord::id, "admitted")))
            .WillOnce(Invoke([&admittedGate](StorageRecord const&) -> bool
            {
                admittedGate.Arrive();
                admittedGate.WaitForRelease();
                return true;
            }));
        EXPECT_CALL(*diskStorage, Flush()).Times(0);
        EXPECT_CALL(*diskStorage, Shutdown());

        std::thread admittedThread([&handler]()
        {
            handler.StoreRecord(MakeRecord("admitted"));
        });
        ASSERT_TRUE(admittedGate.WaitForArrival(std::chrono::seconds(5)))
            << "Admitted store never reached the disk write";

        // Shutdown() closes admission, then blocks draining the in-flight admitted
        // store. Storage is not torn down yet, so it remains valid.
        std::promise<void> shutdownDone;
        std::future<void> shutdownDoneFuture = shutdownDone.get_future();
        std::thread shutdownThread([&handler, &shutdownDone]()
        {
            handler.Shutdown();
            shutdownDone.set_value();
        });
        ASSERT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::milliseconds(200)),
            std::future_status::timeout)
            << "Shutdown() proceeded while an admitted store was still draining";

        handler.Flush();

        // Release the admitted store; Shutdown() drains and tears storage down.
        admittedGate.Release();
        admittedThread.join();
        ASSERT_EQ(
            shutdownDoneFuture.wait_for(std::chrono::seconds(5)),
            std::future_status::ready)
            << "Shutdown() never completed after the admitted store finished";
        shutdownThread.join();
    }

    TEST_F(OfflineStorageHandlerTests, ConcurrentShutdownRunsStorageShutdownOnce)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        config[CFG_INT_RAM_QUEUE_SIZE] = 0;
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        Rendezvous shutdownGate;
        EXPECT_CALL(*diskStorage, Shutdown())
            .WillOnce(Invoke([&shutdownGate]()
            {
                shutdownGate.Arrive();
                shutdownGate.WaitForRelease();
            }));
        std::thread first([&handler] { handler.Shutdown(); });
        ASSERT_TRUE(shutdownGate.WaitForArrival(std::chrono::seconds(5)));

        std::future<void> second = std::async(std::launch::async, [&handler]
        {
            handler.Shutdown();
        });
        EXPECT_EQ(second.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout);
        shutdownGate.Release();
        first.join();
        ASSERT_EQ(second.wait_for(std::chrono::seconds(5)), std::future_status::ready);
        handler.Shutdown();
    }

    TEST_F(OfflineStorageHandlerTests, FailedShutdownCompletesTeardown)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        config[CFG_INT_RAM_QUEUE_SIZE] = 0;
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);

        EXPECT_CALL(*diskStorage, Shutdown())
            .WillOnce(Throw(std::runtime_error("shutdown failed")));
        EXPECT_THROW(handler.Shutdown(), std::runtime_error);

        handler.Shutdown();
    }

    TEST_F(OfflineStorageHandlerTests, SavedObserverCanReenterFlush)
    {
        ConfigurableLogManager logManager;
        NoCheckpointRuntimeConfig config;
        NoopTaskDispatcher taskDispatcher;
        ConfigureMemoryCache(config, 1024 * 1024);
        auto diskStorage = AttachDiskStorage(logManager);
        StrictMock<testing::MockIOfflineStorageObserver> observer;
        OfflineStorageHandler handler(logManager, config, taskDispatcher);
        EXPECT_CALL(*diskStorage, Initialize(_));
        handler.Initialize(observer);
        ASSERT_TRUE(handler.StoreRecord(MakeRecord("record")));
        EXPECT_CALL(*diskStorage, StoreRecords(_))
            .WillOnce(Invoke([](std::vector<StorageRecord>& records) { return records.size(); }));
        EXPECT_CALL(observer, OnStorageRecordsSaved(1))
            .WillOnce(Invoke([&handler](size_t) { handler.Flush(); }));

        handler.Flush();

        EXPECT_CALL(*diskStorage, Shutdown());
        handler.Shutdown();
    }
} MAT_NS_END
