// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "common/MockIBandwidthController.hpp"
#include "tpm/TransmissionPolicyManager.hpp"

using namespace testing;
using namespace ARIASDK_NS;


class TransmissionPolicyManager4Test : public TransmissionPolicyManager {
  public:
    TransmissionPolicyManager4Test(IRuntimeConfig& runtimeConfig, IBandwidthController* bandwidthController)
      : TransmissionPolicyManager(runtimeConfig, bandwidthController)
    {
    }

    MOCK_METHOD1(scheduleUpload, void(int));
    using TransmissionPolicyManager::uploadAsync;

    bool uploadScheduled() const { return m_isUploadScheduled; }
    void uploadScheduled(bool state) { m_isUploadScheduled = state; }

    std::set<EventsUploadContextPtr> const& activeUploads() const { return m_activeUploads; }
    EventsUploadContextPtr fakeActiveUpload() { auto ctx = EventsUploadContext::create(); m_activeUploads.insert(ctx); return ctx; }

    bool paused() const { return m_isPaused; }
    void paused(bool state) { m_isPaused = state; }
};

class TransmissionPolicyManagerTests : public StrictMock<Test> {
  protected:
    StrictMock<MockIRuntimeConfig>       runtimeConfigMock;
    StrictMock<MockIBandwidthController> bandwidthControllerMock;
    TransmissionPolicyManager4Test       tpm;

    RouteSink<TransmissionPolicyManagerTests, EventsUploadContextPtr const&> initiateUpload{this, &TransmissionPolicyManagerTests::resultInitiateUpload};
    RouteSink<TransmissionPolicyManagerTests>                                allUploadsFinished{this, &TransmissionPolicyManagerTests::resultAllUploadsFinished};

  protected:
    TransmissionPolicyManagerTests()
      : tpm(runtimeConfigMock, &bandwidthControllerMock)
    {
        tpm.initiateUpload     >> initiateUpload;
        tpm.allUploadsFinished >> allUploadsFinished;
    }

    MOCK_METHOD1(resultInitiateUpload, void(EventsUploadContextPtr const &));
    MOCK_METHOD0(resultAllUploadsFinished, void());

    virtual void SetUp() override
    {
        EXPECT_CALL(bandwidthControllerMock, GetProposedBandwidthBps())
            .WillRepeatedly(Return(1000000));
        EXPECT_CALL(runtimeConfigMock, GetMinimumUploadBandwidthBps())
            .WillRepeatedly(Return(1000000));
    }
};


TEST_F(TransmissionPolicyManagerTests, StartSchedulesUploadImmediately)
{
    tpm.paused(true);
    EXPECT_CALL(tpm, scheduleUpload(0))
        .WillOnce(Return());
    EXPECT_THAT(tpm.start(), true);
    EXPECT_THAT(tpm.paused(), false);
}

TEST_F(TransmissionPolicyManagerTests, StopCancelsScheduledUploads)
{
    tpm.uploadScheduled(true);
    tpm.paused(false);
    EXPECT_THAT(tpm.stop(), true);
    EXPECT_THAT(tpm.paused(), true);
    EXPECT_THAT(tpm.uploadScheduled(), false);
}

TEST_F(TransmissionPolicyManagerTests, IncomingEventDoesNothingWhenPaused)
{
    tpm.paused(true);

    auto event = IncomingEventContext::create();
    tpm.eventArrived(event);
}

TEST_F(TransmissionPolicyManagerTests, IncomingEventSchedulesUpload)
{
    tpm.paused(false);

    auto event = IncomingEventContext::create();
    event->record.priority = EventPriority_Normal;
    EXPECT_CALL(tpm, scheduleUpload(1000))
        .WillOnce(Return());
    tpm.eventArrived(event);
}

TEST_F(TransmissionPolicyManagerTests, ImmediateIncomingEventStartsUploadImmediately)
{
    tpm.paused(false);

    auto event = IncomingEventContext::create();
    event->record.priority = EventPriority_Immediate;
    EventsUploadContextPtr upload;
    EXPECT_CALL(*this, resultInitiateUpload(_))
        .WillOnce(SaveArg<0>(&upload));
    tpm.eventArrived(event);

    ASSERT_THAT(upload.get(), NotNull());
    EXPECT_THAT(upload->requestedMinPriority, EventPriority_Immediate);
}

TEST_F(TransmissionPolicyManagerTests, UploadDoesNothingWhenPaused)
{
    tpm.uploadScheduled(true);
    tpm.paused(true);

    tpm.uploadAsync();

    EXPECT_THAT(tpm.uploadScheduled(), false);
}

TEST_F(TransmissionPolicyManagerTests, UploadDoesNothingWithActiveUploads)
{
    tpm.uploadScheduled(true);
    tpm.paused(false);
    tpm.fakeActiveUpload();

    tpm.uploadAsync();

    EXPECT_THAT(tpm.uploadScheduled(), false);
}

TEST_F(TransmissionPolicyManagerTests, UploadPostponedWithInsufficientAvailableBandwidth)
{
    tpm.uploadScheduled(true);
    tpm.paused(false);

    EXPECT_CALL(bandwidthControllerMock, GetProposedBandwidthBps())
        .WillOnce(Return(999999));
    EXPECT_CALL(tpm, scheduleUpload(1000))
        .WillOnce(Return());
    tpm.uploadAsync();

    EXPECT_THAT(tpm.uploadScheduled(), false);
}

TEST_F(TransmissionPolicyManagerTests, UploadInitiatesUpload)
{
    tpm.uploadScheduled(true);
    tpm.paused(false);

    EventsUploadContextPtr upload;
    EXPECT_CALL(*this, resultInitiateUpload(_))
        .WillOnce(SaveArg<0>(&upload));
    tpm.uploadAsync();

    EXPECT_THAT(tpm.uploadScheduled(), false);
    EXPECT_THAT(upload.get(), NotNull());
    EXPECT_THAT(tpm.activeUploads(), Contains(upload));
}

TEST_F(TransmissionPolicyManagerTests, EmptyUploadCeasesUploading)
{
    auto upload = tpm.fakeActiveUpload();
    tpm.nothingToUpload(upload);
}

TEST_F(TransmissionPolicyManagerTests, FailedUploadPackagingSchedulesNextOneWithDelay)
{
    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(2000))
        .WillOnce(Return());
    tpm.packagingFailed(upload);
}

TEST_F(TransmissionPolicyManagerTests, SuccessfulUploadSchedulesNextOneImmediately)
{
    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(0))
        .WillOnce(Return());
    tpm.eventsUploadSuccessful(upload);
}

TEST_F(TransmissionPolicyManagerTests, RejectedUploadSchedulesNextOneWithLargerDelay)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,3000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(6000))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);
}

TEST_F(TransmissionPolicyManagerTests, FailedUploadSchedulesNextOneWithLargerDelay)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,3000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(6000))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);
}

TEST_F(TransmissionPolicyManagerTests, SuccessfulUploadResetsBackoffDelay)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,3000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(0))
        .WillOnce(Return());
    tpm.eventsUploadSuccessful(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(6000))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(0))
        .WillOnce(Return());
    tpm.eventsUploadSuccessful(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);
}

TEST_F(TransmissionPolicyManagerTests, InvalidUploadRetryBackoffConfigKeepsUsingThePreviousOne)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,1000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(1000))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);

    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("x,"));

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(2000))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);
}

TEST_F(TransmissionPolicyManagerTests, AbortedUploadDoesNotScheduleNextOne)
{
    auto upload = tpm.fakeActiveUpload();
    tpm.eventsUploadAborted(upload);
}

TEST_F(TransmissionPolicyManagerTests, FinishAllUploadsWhenIdleIsSynchronous)
{
    EXPECT_CALL(*this, resultAllUploadsFinished())
        .WillOnce(Return());
    tpm.finishAllUploads();
}

TEST_F(TransmissionPolicyManagerTests, FinishAllUploadsWhenBusyWaitsForAllUploads)
{
    auto upload1 = tpm.fakeActiveUpload();
    auto upload2 = tpm.fakeActiveUpload();
    ASSERT_THAT(tpm.activeUploads(), SizeIs(2));

    tpm.finishAllUploads();

    tpm.eventsUploadAborted(upload1);
    EXPECT_THAT(tpm.activeUploads(), SizeIs(1));

    EXPECT_CALL(*this, resultAllUploadsFinished())
        .WillOnce(Return());
    tpm.eventsUploadAborted(upload2);
    EXPECT_THAT(tpm.activeUploads(), SizeIs(0));
}
