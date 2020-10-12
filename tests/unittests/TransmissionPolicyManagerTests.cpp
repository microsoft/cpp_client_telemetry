//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
//
// TODO: re-enable TPM testcases for backoff configuration change
//
#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "common/MockIBandwidthController.hpp"
#include "tpm/TransmissionPolicyManager.hpp"
#include "TransmitProfiles.hpp"

using namespace testing;
using namespace MAT;


class TransmissionPolicyManager4Test : public TransmissionPolicyManager {
  public:
    TransmissionPolicyManager4Test(ITelemetrySystem& system, IBandwidthController* bandwidthController)
      : TransmissionPolicyManager(system, *PAL::getDefaultTaskDispatcher(), bandwidthController)
    {
    }

    void uploadAsyncParent(EventLatency latency)
    {
        TransmissionPolicyManager::uploadAsync(latency);
    }

    void scheduleUploadParent(const std::chrono::milliseconds& delay, EventLatency latency, bool force)
    {
        TransmissionPolicyManager::scheduleUpload(delay, latency, force);
    }

    using TransmissionPolicyManager::increaseBackoff;
    using TransmissionPolicyManager::addUpload;
    using TransmissionPolicyManager::removeUpload;
    using TransmissionPolicyManager::getCancelWaitTime;
    using TransmissionPolicyManager::cancelUploadTask;

    using TransmissionPolicyManager::m_backoff;
    using TransmissionPolicyManager::m_isPaused;
    using TransmissionPolicyManager::m_scheduledUploadAborted;
    using TransmissionPolicyManager::m_isUploadScheduled;
    using TransmissionPolicyManager::m_scheduledUploadTime;
    using TransmissionPolicyManager::m_timerdelay;
    using TransmissionPolicyManager::m_runningLatency;
    using TransmissionPolicyManager::m_backoffConfig;

    MOCK_METHOD3(scheduleUpload, void(const std::chrono::milliseconds&, EventLatency,bool));
    MOCK_METHOD1(uploadAsync, void(EventLatency));
    MOCK_METHOD0(handleStop, bool());

    bool uploadScheduled() const { return m_isUploadScheduled; }
    void uploadScheduled(bool state) { m_isUploadScheduled = state; }

    std::set<EventsUploadContextPtr> const& activeUploads() const { return m_activeUploads; }
    EventsUploadContextPtr fakeActiveUpload() { auto ctx = std::make_shared<EventsUploadContext>(); ctx->requestedMinLatency = EventLatency_RealTime; m_activeUploads.insert(ctx); return ctx; }
    EventsUploadContextPtr fakeActiveUpload(EventLatency latency) { auto ctx = std::make_shared<EventsUploadContext>(); ctx->requestedMinLatency = latency; m_activeUploads.insert(ctx); return ctx; }

    bool paused() const { return m_isPaused; }
    void paused(bool state) { m_isPaused = state; }

    void runningLatency(EventLatency latency) { m_runningLatency = latency; }

    void NotMockScheduleUpload(const std::chrono::milliseconds& delay, EventLatency latency, bool force)
    {
        TransmissionPolicyManager::scheduleUpload(delay, latency, force);
    }
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
      : tpm(testing::getSystem(), &bandwidthControllerMock)
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

        ON_CALL(tpm, uploadAsync(_)).
            WillByDefault(Invoke(&tpm, &TransmissionPolicyManager4Test::uploadAsyncParent));
    }
};

#if 0
TEST_F(TransmissionPolicyManagerTests, StartSchedulesUploadImmediately)
{
    tpm.uploadScheduled(false);
    tpm.paused(false);
    EXPECT_CALL(tpm, scheduleUpload(0, EventLatency_Normal,false)).WillOnce(Return());
    EXPECT_THAT(tpm.start(), true);
    // EXPECT_CALL(tpm, uploadAsync(EventLatency_Normal)).WillOnce(Return());
    EXPECT_THAT(tpm.paused(), false);
}
#endif

TEST_F(TransmissionPolicyManagerTests, StopLeavesNoScheduledUploads)
{
    tpm.paused(false);

    EXPECT_CALL(tpm, scheduleUpload(std::chrono::milliseconds { 1000 }, AnyOf(EventLatency_Normal, EventLatency_RealTime), false))
        .WillOnce(Return());
    tpm.start();

    size_t i = 1000;
    while (i--)
    {
        EXPECT_CALL(tpm, scheduleUpload(_, AnyOf(EventLatency_Normal, EventLatency_RealTime), false))
            .Times(3)
            .WillOnce(Return())
            .WillOnce(Return())
            .WillOnce(Return());
        for (auto k :
            {
                tpm.eventsUploadSuccessful,
                tpm.eventsUploadRejected,
                tpm.eventsUploadFailed,
                tpm.eventsUploadAborted,
            }
            )
        {
            k(tpm.fakeActiveUpload());
        }
    }

    EXPECT_CALL(*this, resultAllUploadsFinished())
        .WillOnce(Return());
    tpm.stop();

    EXPECT_THAT(tpm.activeUploads(), SizeIs(0));
}

TEST_F(TransmissionPolicyManagerTests, IncomingEventDoesNothingWhenPaused)
{
    tpm.paused(true);

    auto event = new IncomingEventContext();
    tpm.eventArrived(event);
}

TEST_F(TransmissionPolicyManagerTests, IncomingEventSchedulesUpload)
{
    tpm.paused(false);

    std::string customProfile = R"(
        [
            {
                "name": "Fred",
                "rules": [
                    {"timers": [ 4, 2, 1 ]}
                ]
            }
        ]
    )";
    EXPECT_TRUE(TransmitProfiles::load(customProfile));
    EXPECT_TRUE(TransmitProfiles::setProfile("Fred"));

    auto event = new IncomingEventContext();
    event->record.latency = EventLatency_Normal;

    
    EXPECT_CALL(tpm, scheduleUpload(std::chrono::milliseconds { 1000 }, EventLatency_Normal, true))
        .WillOnce(Return());
    tpm.eventArrived(event);
}

TEST_F(TransmissionPolicyManagerTests, ProfileAffectsSchedule)
{
    tpm.paused(false);

    std::string customProfile = R"(
        [
            {
                "name": "Fred",
                "rules": [
                    {"timers": [ -1, -1, -1 ]}
                ]
            }
        ]
    )";
    EXPECT_TRUE(TransmitProfiles::load(customProfile));
    EXPECT_TRUE(TransmitProfiles::setProfile("Fred"));

    auto event = new IncomingEventContext();
    event->record.latency = EventLatency_Normal;
    EXPECT_CALL(tpm, scheduleUpload(_, _, _)).Times(0);
    tpm.eventArrived(event);
    TransmitProfiles::reset();
}

TEST_F(TransmissionPolicyManagerTests, NoUploadForNegative)
{
    tpm.paused(false);

    std::string customProfile = R"(
        [
            {
                "name": "Fred",
                "rules": [
                    {"timers": [ -1, -1, -1 ]}
                ]
            }
        ]
    )";
    EXPECT_TRUE(TransmitProfiles::load(customProfile));
    EXPECT_TRUE(TransmitProfiles::setProfile("Fred"));

    auto event = new IncomingEventContext();
    event->record.latency = EventLatency_Normal;
    EXPECT_CALL(tpm, scheduleUpload(_, _, _)).Times(0);
    tpm.eventArrived(event);
    EXPECT_CALL(tpm, uploadAsync(_)).Times(0);
    tpm.scheduleUploadParent(std::chrono::milliseconds{-1000}, EventLatency_RealTime, true);
    TransmitProfiles::reset();
}

TEST_F(TransmissionPolicyManagerTests, ImmediateIncomingEventStartsUploadImmediately)
{
    tpm.paused(false);

    auto event = new IncomingEventContext();
    event->record.latency = EventLatency_Max;
    EventsUploadContextPtr upload;
    EXPECT_CALL(*this, resultInitiateUpload(_))
        .WillOnce(SaveArg<0>(&upload));
    tpm.eventArrived(event);

    ASSERT_THAT(upload, NotNull());
    EXPECT_THAT(upload->requestedMinLatency, EventLatency_Max);
}

TEST_F(TransmissionPolicyManagerTests, UploadDoesNothingWhenPaused)
{
    tpm.uploadScheduled(true);
    tpm.paused(true);
    tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, uploadAsync(_)).Times(0);
}

TEST_F(TransmissionPolicyManagerTests, UploadDoesNothingWhenAlreadyActive)
{
    tpm.uploadScheduled(true);
    tpm.paused(false);
    tpm.fakeActiveUpload();
    EXPECT_CALL( tpm, uploadAsync(_) ).Times(0);
}

#if 0
TEST_F(TransmissionPolicyManagerTests, UploadPostponedWithInsufficientAvailableBandwidth)
{
    tpm.uploadScheduled(true);
    tpm.paused(false);

    EXPECT_CALL(bandwidthControllerMock, GetProposedBandwidthBps())
        .WillOnce(Return(999999));
    EXPECT_CALL(tpm, scheduleUpload(1000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.uploadAsync(EventLatency_Normal);

    EXPECT_THAT(tpm.uploadScheduled(), false);
}
#endif

TEST_F(TransmissionPolicyManagerTests, UploadInitiatesUpload)
{
    tpm.uploadScheduled(true);
    tpm.paused(false);

    EventsUploadContextPtr upload;
    EXPECT_CALL(*this, resultInitiateUpload(_))
        .WillOnce(SaveArg<0>(&upload));
    tpm.uploadAsync(EventLatency_Normal);

    EXPECT_THAT(tpm.uploadScheduled(), false);
    EXPECT_THAT(upload, NotNull());
    EXPECT_THAT(tpm.activeUploads(), Contains(upload));
}

TEST_F(TransmissionPolicyManagerTests, EmptyUploadCeasesUploadingForRunningLatencyNormal)
{
    auto upload = tpm.fakeActiveUpload(EventLatency_Normal);
    EXPECT_CALL(tpm, scheduleUpload(_, _, false))
      .Times(0);
    tpm.nothingToUpload(upload);
}

TEST_F(TransmissionPolicyManagerTests, EmptyUploadReschedulesAtTimerDelayForRunningLatencyRealtime)
{
    auto upload = tpm.fakeActiveUpload();
    constexpr std::chrono::milliseconds delay { std::chrono::seconds(300) };
    tpm.m_timerdelay = delay;
    tpm.runningLatency(EventLatency_RealTime);
    EXPECT_CALL(tpm, scheduleUpload(delay, EventLatency_Normal, false))
      .WillOnce(Return());
    tpm.nothingToUpload(upload);
}

TEST_F(TransmissionPolicyManagerTests, FailedUploadPackagingSchedulesNextOneWithDelay)
{
    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(std::chrono::milliseconds{ 2000 }, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.packagingFailed(upload);
}

TEST_F(TransmissionPolicyManagerTests, SuccessfulUploadSchedulesNextOneImmediately)
{
    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(std::chrono::milliseconds{ 0 }, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadSuccessful(upload);
}

#if 0
TEST_F(TransmissionPolicyManagerTests, RejectedUploadSchedulesNextOneWithLargerDelay)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,3000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();

    // Call time could be greater than 3000 here, so let's use a wildcard matcher
    EXPECT_CALL(tpm, scheduleUpload(_, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(6000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);
}
#endif

#if 0
TEST_F(TransmissionPolicyManagerTests, FailedUploadSchedulesNextOneWithLargerDelay)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,3000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(6000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);
}
#endif

#if 0
TEST_F(TransmissionPolicyManagerTests, SuccessfulUploadResetsBackoffDelay)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,3000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(0, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadSuccessful(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadRejected(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(6000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(0, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadSuccessful(upload);

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(3000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);
}
#endif

#if 0
TEST_F(TransmissionPolicyManagerTests, InvalidUploadRetryBackoffConfigKeepsUsingThePreviousOne)
{
    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("E,1000,300000,2,0"));

    auto upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(1000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);

    EXPECT_CALL(runtimeConfigMock, GetUploadRetryBackoffConfig())
        .WillRepeatedly(Return("x,"));

    upload = tpm.fakeActiveUpload();
    EXPECT_CALL(tpm, scheduleUpload(2000, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.eventsUploadFailed(upload);
}
#endif

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

TEST_F(TransmissionPolicyManagerTests, FinishAllUploads)
{
    tpm.paused(false);

    EXPECT_CALL(*this, resultAllUploadsFinished())
        .WillOnce(Return());
    tpm.stop();

    EXPECT_CALL(tpm, scheduleUpload(std::chrono::milliseconds { 1000 }, EventLatency_Normal, false))
        .WillOnce(Return());
    tpm.start();

    EXPECT_CALL(tpm, scheduleUpload(std::chrono::milliseconds { 0 }, EventLatency_Normal, false))
        .Times(2)
        .WillOnce(Return())
        .WillOnce(Return());
    auto upload1 = tpm.fakeActiveUpload();
    auto upload2 = tpm.fakeActiveUpload();

    ASSERT_THAT(upload1, NotNull());
    ASSERT_THAT(upload2, NotNull());

    EXPECT_THAT(tpm.activeUploads(), SizeIs(2));

    tpm.eventsUploadSuccessful(upload1);
    tpm.eventsUploadSuccessful(upload2);
    EXPECT_THAT(tpm.activeUploads(), SizeIs(0));

    EXPECT_CALL(*this, resultAllUploadsFinished())
        .WillOnce(Return());
    tpm.stop();
}

TEST_F(TransmissionPolicyManagerTests, FredProfile)
{
    const char* fredProfile = R"(
[{
    "name": "Fred_Profile",
    "rules": [
    { "netCost": "restricted", "timers": [  -1,  -1,  -1 ] },
    { "netCost": "high",       "timers": [  -1,  -1,  -1 ] },
    { "netCost": "low",        "timers": [  -1,  -1,  -1 ] },
    { "netCost": "unknown",    "timers": [  -1,  -1,  -1 ] },
    {                          "timers": [  -1,  -1,  -1 ] }
    ]
}]
)";
    EXPECT_TRUE(TransmitProfiles::load(fredProfile));
    EXPECT_TRUE(TransmitProfiles::setProfile("Fred_Profile"));
    tpm.paused(false);

    auto event = new IncomingEventContext();
    event->record.latency = EventLatency_Normal;
    EXPECT_CALL(tpm, scheduleUpload(_, _, _))
        .Times(0);
    tpm.eventArrived(event);
}

TEST_F(TransmissionPolicyManagerTests, Constructor_IsPaused_True)
{
    ASSERT_TRUE(tpm.m_isPaused);
}

TEST_F(TransmissionPolicyManagerTests, Constructor_IsUploadScheduled_False)
{
    ASSERT_FALSE(tpm.m_isUploadScheduled);
}

TEST_F(TransmissionPolicyManagerTests, Constructor_ScheduledUploadAborted_False)
{
    ASSERT_FALSE(tpm.m_scheduledUploadAborted);
}

TEST_F(TransmissionPolicyManagerTests, Constructor_ScheduledUploadTime_Uint64Max)
{
    ASSERT_EQ(tpm.m_scheduledUploadTime, std::numeric_limits<uint64_t>::max());
}

TEST_F(TransmissionPolicyManagerTests, Constructor_TimerDelay_TwoSeconds)
{
    ASSERT_EQ(tpm.m_timerdelay, std::chrono::seconds{ 2 });
}

TEST_F(TransmissionPolicyManagerTests, Constructor_TimerDelayInteger_TwoThousand)
{
    ASSERT_EQ(tpm.m_timerdelay.count(), 2000);
}

TEST_F(TransmissionPolicyManagerTests, Constructor_RunningLatency_RealTime)
{
    ASSERT_EQ(tpm.m_runningLatency, EventLatency_RealTime);
}

TEST_F(TransmissionPolicyManagerTests, Constructor_BackoffConfig_RealTime)
{
    ASSERT_EQ(tpm.m_backoffConfig, std::string{ DefaultBackoffConfig });
}

TEST_F(TransmissionPolicyManagerTests, removeUpload_nullptr_ReturnsFalse)
{
    EXPECT_FALSE(tpm.removeUpload(nullptr));
}

TEST_F(TransmissionPolicyManagerTests, removeUpload_ContextNotAdded_ReturnsFalse)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    EXPECT_FALSE(tpm.removeUpload(ctx));
}

TEST_F(TransmissionPolicyManagerTests, removeUpload_ContextAdded_ReturnsTrue)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    tpm.addUpload(ctx);
    EXPECT_TRUE(tpm.removeUpload(ctx));
}

TEST_F(TransmissionPolicyManagerTests, removeUpload_CalledTwiceContextAdded_ReturnsTrueThenFalse)
{
    auto ctx = std::make_shared<EventsUploadContext>();
    tpm.addUpload(ctx);
    EXPECT_TRUE(tpm.removeUpload(ctx));
    EXPECT_FALSE(tpm.removeUpload(ctx));
}

TEST_F(TransmissionPolicyManagerTests, getCancelWaitTime_ScheduledUploadAborted_ReturnsDefaultValue)
{
    tpm.m_scheduledUploadAborted = true;
    ASSERT_EQ(tpm.getCancelWaitTime(), DefaultTaskCancelTime);
}

#ifndef UPLOAD_TASK_CANCEL_TIME_MS
TEST_F(TransmissionPolicyManagerTests, getCancelWaitTime_ScheduledUploadAborted_ReturnsDefaultValueInteger)
{
    tpm.m_scheduledUploadAborted = true;
    ASSERT_EQ(tpm.getCancelWaitTime().count(), 500);
}
#else
TEST_F(TransmissionPolicyManagerTests, getCancelWaitTime_ScheduledUploadAborted_ReturnsDefaultValueInteger)
{
    tpm.m_scheduledUploadAborted = true;
    ASSERT_EQ(tpm.getCancelWaitTime().count(), uint64_t { UPLOAD_TASK_CANCEL_TIME_MS });
}
#endif

TEST_F(TransmissionPolicyManagerTests, getCancelWaitTime_ScheduledUploadNotAborted_ReturnsZero)
{
    tpm.m_scheduledUploadAborted = false;
    ASSERT_EQ(tpm.getCancelWaitTime(), std::chrono::milliseconds{});
}

TEST_F(TransmissionPolicyManagerTests, getCancelWaitTime_ScheduledUploadNotAborted_ReturnsZeroInteger)
{
    tpm.m_scheduledUploadAborted = false;
    ASSERT_EQ(tpm.getCancelWaitTime().count(), uint64_t { 0 });
}

TEST_F(TransmissionPolicyManagerTests, cancelUploadTask_ScheduledUpload_ReturnsTrue)
{
    ASSERT_TRUE(tpm.cancelUploadTask());
}

TEST_F(TransmissionPolicyManagerTests, cancelUploadTask_ScheduledUpload_IsUploadScheduledSetToFalse)
{
    tpm.m_isUploadScheduled = true;
    tpm.cancelUploadTask();
    ASSERT_FALSE(tpm.m_isUploadScheduled);
}

TEST_F(TransmissionPolicyManagerTests, increaseBackoff_EmptyBackoffObject_ReturnZero)
{
    tpm.m_backoff = nullptr;
    ASSERT_EQ(tpm.increaseBackoff(), std::chrono::milliseconds{});
}

TEST_F(TransmissionPolicyManagerTests, increaseBackoff_ValidBackoffObject_ReturnsGreaterThanZero)
{
    ASSERT_GT(tpm.increaseBackoff(), std::chrono::milliseconds{});
}

TEST_F(TransmissionPolicyManagerTests, increaseBackoff_CalledTwice_ReturnsHigherValue)
{
    auto first = tpm.increaseBackoff();
    ASSERT_GT(tpm.increaseBackoff(), first);
}
