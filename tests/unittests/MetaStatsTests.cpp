//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "stats/MetaStats.hpp"

using namespace testing;
using namespace MAT;


class MetaStatsTests : public ::testing::Test
{
  protected:
    StrictMock<MockIRuntimeConfig> runtimeConfigMock;

    MetaStats                      stats;

  public:
    MetaStatsTests() :
        stats(runtimeConfigMock)
    {
    }
};

TEST_F(MetaStatsTests, NoInputGeneratesNoEvents)
{
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-tenant-token"));
    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_START);
    EXPECT_THAT(events, SizeIs(1));
}

TEST_F(MetaStatsTests, GenerateStartEvent)
{
    unsigned const postDataLength         = 234;
    unsigned const lowLatencyCount       = 1;
    unsigned const normalLatencyCount    = 2;
    unsigned const highLatencyCount      = 3;
    unsigned const immediateLatencyCount = 1;
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-tenant-token"));
    stats.updateOnStorageOpened("MyStorage/Normal");
    stats.updateOnPostData(postDataLength, false);

    std::map<std::string, std::string> recordIdAndTenantid;
    recordIdAndTenantid["r"] = "t";
    stats.updateOnPackageSentSucceeded(recordIdAndTenantid, EventLatency_Normal,        0,   333, std::vector<unsigned>{ 1333 },          false);
    stats.updateOnPackageSentSucceeded(recordIdAndTenantid, EventLatency_Normal,     1,   444, std::vector<unsigned>{ 1444, 2444 },    false);
    stats.updateOnPackageSentSucceeded(recordIdAndTenantid, EventLatency_RealTime,       3,  5555, std::vector<unsigned>{ 15, 255, 3555 }, false);
    stats.updateOnPackageSentSucceeded(recordIdAndTenantid, EventLatency_Max,  0,   666, std::vector<unsigned>{ 666 },           false);
    stats.updateOnPackageFailed(500);
    stats.updateOnPackageFailed(500);
    stats.updateOnPackageRetry(500, 2);
    stats.updateOnPackageRetry(503, 3);
    stats.updateOnStorageFailed("reorg");
    std::map<std::string, size_t> dropCount;
    dropCount["t"] = 12;
    stats.updateOnRecordsDropped(DROPPED_REASON_OFFLINE_STORAGE_OVERFLOW, dropCount);
    dropCount["t"] = 34;
    stats.updateOnRecordsDropped(DROPPED_REASON_RETRY_EXCEEDED, dropCount);

    // Expect 2 events for builds with per-tenant stats enabled
    size_t expectedCount = (runtimeConfigMock[CFG_MAP_METASTATS_CONFIG]["split"]) ? 2 : 1;
    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_START);
    ASSERT_THAT(events, SizeIs(expectedCount));
}

TEST_F(MetaStatsTests, GenerateStopEvent)
{
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-tenant-token"));
    stats.updateOnPostData(16, false);
    std::map<std::string, std::string> recordIdAndTenantid;
    recordIdAndTenantid["r"] = "t";
    stats.updateOnPackageSentSucceeded(recordIdAndTenantid, EventLatency_RealTime, 1, 99, std::vector<unsigned>{ 100, 101, 102, 103, 104, 105, 106 }, false);
    stats.updateOnPackageFailed(501);
    stats.updateOnPackageFailed(403);
    stats.updateOnPackageRetry(505, 2);

    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_STOP);
    ASSERT_THAT(events, SizeIs(1));
}

TEST_F(MetaStatsTests, NoNewDataOrMetastatsOnlyGenerateNoEvents)
{
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsSendIntervalSec())
        .WillRepeatedly(Return(123));
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-tenant-token"));

    // Send one normal event first to verify that stats are reset on generation.
    stats.updateOnEventIncoming("t1", 123, EventLatency_RealTime, false);
    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(1));

    events.clear();
    // Verify nothing is generated if nothing happens.
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(0));
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(0));

    // Simulate logging and uploading some metastats events only. Nothing should be generated either.
    stats.updateOnEventIncoming("s",123, EventLatency_RealTime, true);
    stats.updateOnEventIncoming("s",123, EventLatency_Normal, true);
    stats.updateOnPostData(123, true);
    std::map<std::string, std::string> recordIdAndTenantid;
    recordIdAndTenantid["r"] = "t";
    stats.updateOnPackageSentSucceeded(recordIdAndTenantid, EventLatency_RealTime, 0, 123, std::vector<unsigned>{ 1234 }, true);
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    //EXPECT_THAT(events, SizeIs(0));
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    //EXPECT_THAT(events, SizeIs(0));

    // Verify events are generated again once some normal event arrives.
    stats.updateOnEventIncoming("t1",123, EventLatency_RealTime, false);
    // Even if the last record is metastats.
    stats.updateOnEventIncoming("t1",123, EventLatency_RealTime, true);
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    //EXPECT_THAT(events, SizeIs(1));
    //EXPECT_THAT(events[0].Extension, Contains(Pair("records_received_count",   "4")));
    //EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_succeeded", "1")));
}

