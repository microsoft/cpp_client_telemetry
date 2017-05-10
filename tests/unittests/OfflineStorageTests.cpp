// Copyright (c) Microsoft. All rights reserved .

#include "common/Common.hpp"
#include "common/MockIOfflineStorage.hpp"
#include "offline/OfflineStorage.hpp"

using namespace testing;
using namespace ARIASDK_NS;


class OfflineStorageTests : public StrictMock<Test> {
  protected:
    MockIOfflineStorage offlineStorageMock;
    OfflineStorage      offlineStorage;

    RouteSink<OfflineStorageTests, IncomingEventContextPtr const&>                             storeRecordFailed{this, &OfflineStorageTests::resultStoreRecordFailed};
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&, StorageRecord const&, bool&> retrievedEvent{this, &OfflineStorageTests::resultRetrievedEvent};
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&>                              retrievalFinished{this, &OfflineStorageTests::resultRetrievalFinished};
    RouteSink<OfflineStorageTests, EventsUploadContextPtr const&>                              retrievalFailed{this, &OfflineStorageTests::resultRetrievalFailed};

    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          opened{this, &OfflineStorageTests::notifOpened};
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          failed{this, &OfflineStorageTests::notifFailed};
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          trimmed{this, &OfflineStorageTests::notifTrimmed};
    RouteSink<OfflineStorageTests, StorageNotificationContext const*>                          recordsDropped{this, &OfflineStorageTests::notifRecordsDropped};

  protected:
    OfflineStorageTests()
      : offlineStorage(offlineStorageMock)
    {
        offlineStorage.storeRecordFailed >> storeRecordFailed;
        offlineStorage.retrievedEvent    >> retrievedEvent;
        offlineStorage.retrievalFinished >> retrievalFinished;
        offlineStorage.retrievalFailed   >> retrievalFailed;

        offlineStorage.opened            >> opened;
        offlineStorage.failed            >> failed;
        offlineStorage.trimmed           >> trimmed;
        offlineStorage.recordsDropped    >> recordsDropped;
    }

    MOCK_METHOD1(resultStoreRecordFailed, void(IncomingEventContextPtr const &));
    MOCK_METHOD3(resultRetrievedEvent,    void(EventsUploadContextPtr const &, StorageRecord const &, bool&));
    MOCK_METHOD1(resultRetrievalFinished, void(EventsUploadContextPtr const &));
    MOCK_METHOD1(resultRetrievalFailed,   void(EventsUploadContextPtr const &));

    MOCK_METHOD1(notifOpened,             void(StorageNotificationContext const*));
    MOCK_METHOD1(notifFailed,             void(StorageNotificationContext const*));
    MOCK_METHOD1(notifTrimmed,            void(StorageNotificationContext const*));
    MOCK_METHOD1(notifRecordsDropped,     void(StorageNotificationContext const*));
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
    auto ctx = IncomingEventContext::create();

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
    auto ctx = EventsUploadContext::create();
    ctx->requestedMinPriority = EventPriority_Normal;
    ctx->requestedMaxCount = 6;

    StorageRecord record1("r1", "tenant1-token", EventPriority_Normal, 1234567890, std::vector<uint8_t>{1, 127, 255});
    StorageRecord record2("r2", "tenant2-token", EventPriority_Normal, 1234567891, std::vector<uint8_t>{2, 128, 0});
    EXPECT_CALL(offlineStorageMock, GetAndReserveRecords(_, Gt(1000u), ctx->requestedMinPriority, ctx->requestedMaxCount))
        .WillOnce(DoAll(
        Invoke([&record1, &record2](std::function<bool(StorageRecord&&)> const& consumer, unsigned, EventPriority, unsigned) {
        EXPECT_THAT(consumer(std::move(record1)), true);
        EXPECT_THAT(consumer(std::move(record2)), false);
    }),
        Return(true)))
        .RetiresOnSaturation();
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
    auto ctx = EventsUploadContext::create();
    ctx->requestedMinPriority = EventPriority_Normal;
    ctx->requestedMaxCount = 6;

    EXPECT_CALL(offlineStorageMock, GetAndReserveRecords(_, Gt(1000u), ctx->requestedMinPriority, ctx->requestedMaxCount))
        .WillOnce(Return(false));
    EXPECT_CALL(*this, resultRetrievalFailed(ctx))
        .WillOnce(Return());
    offlineStorage.retrieveEvents(ctx);
}

TEST_F(OfflineStorageTests, DeleteRecordsIsForwarded)
{
    auto ctx = EventsUploadContext::create();
	HttpHeaders test;
    EXPECT_CALL(offlineStorageMock, DeleteRecords(Ref(ctx->recordIds),test)).WillOnce(Return());
    EXPECT_THAT(offlineStorage.deleteRecords(ctx), true);
}

TEST_F(OfflineStorageTests, ReleaseRecordsIsForwarded)
{
    auto ctx = EventsUploadContext::create();
	HttpHeaders test;
    EXPECT_CALL(offlineStorageMock, ReleaseRecords(Ref(ctx->recordIds), false, test))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.releaseRecords(ctx), true);

    EXPECT_CALL(offlineStorageMock, ReleaseRecords(Ref(ctx->recordIds), true, test))
        .WillOnce(Return());
    EXPECT_THAT(offlineStorage.releaseRecordsIncRetryCount(ctx), true);
}
