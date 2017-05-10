// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "stats/MetaStats.hpp"

using namespace testing;
using namespace ARIASDK_NS;


class MetaStatsTests : public ::testing::Test
{
  protected:
    StrictMock<MockIRuntimeConfig> runtimeConfigMock;
    ContextFieldsProvider          contextFieldsProvider;
    MetaStats                      stats;

  public:
    MetaStatsTests()
      : contextFieldsProvider(nullptr),
        stats(runtimeConfigMock, contextFieldsProvider)
    {
    }
};

TEST_F(MetaStatsTests, NoInputGeneratesNoEvents)
{
    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_START);
    EXPECT_THAT(events, IsEmpty());
}

TEST_F(MetaStatsTests, GenerateStartEvent)
{
    unsigned const postDataLength         = 234;
    unsigned const lowPriorityCount       = 1;
    unsigned const normalPriorityCount    = 2;
    unsigned const highPriorityCount      = 3;
    unsigned const immediatePriorityCount = 1;
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
    stats.updateOnStorageOpened("MyStorage/Normal");
    stats.updateOnPostData(postDataLength, false);
    stats.updateOnPackageSentSucceeded(EventPriority_Low,        0,   333, std::vector<unsigned>{ 1333 },          false);
    stats.updateOnPackageSentSucceeded(EventPriority_Normal,     1,   444, std::vector<unsigned>{ 1444, 2444 },    false);
    stats.updateOnPackageSentSucceeded(EventPriority_High,       3,  5555, std::vector<unsigned>{ 15, 255, 3555 }, false);
    stats.updateOnPackageSentSucceeded(EventPriority_Immediate,  0,   666, std::vector<unsigned>{ 666 },           false);
    stats.updateOnPackageFailed(500);
    stats.updateOnPackageFailed(500);
    stats.updateOnPackageRetry(500, 2);
    stats.updateOnPackageRetry(503, 3);
    stats.updateOnStorageFailed("reorg");
    stats.updateOnRecordsDropped(DROPPED_REASON_OFFLINE_STORAGE_OVERFLOW, 12);
    stats.updateOnRecordsDropped(DROPPED_REASON_RETRY_EXCEEDED, 34);

    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_START);
    ASSERT_THAT(events, SizeIs(1));
    EXPECT_THAT(events[0].Type, Eq("client_telemetry"));
    EXPECT_THAT(events[0].EventType, Eq("act_stats"));
    EXPECT_THAT(events[0].Extension, Contains(Pair("EventInfo.Time", _)));
    EXPECT_THAT(events[0].Extension, Contains(Pair("DeviceInfo.OsName", _)));
    EXPECT_THAT(events[0].Extension, Contains(Pair("stats_rollup_kind", "start")));

    EXPECT_THAT(events[0].Extension, Contains(Pair("low_priority_records_sent_count",                                         toString(lowPriorityCount))));
    EXPECT_THAT(events[0].Extension, Contains(Pair("low_priority_log_to_successful_send_latency_millisec_distribution",       "0-1000:0,1000-2000:1,2000-4000:0,4000-8000:0,8000-16000:0,16000-32000:0,>32000:0")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("low_priority_log_to_successful_send_latency_millisec_max",                "1333")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("low_priority_log_to_successful_send_latency_millisec_min",                "1333")));

    EXPECT_THAT(events[0].Extension, Contains(Pair("normal_priority_records_sent_count",                                      toString(normalPriorityCount))));
    EXPECT_THAT(events[0].Extension, Contains(Pair("normal_priority_log_to_successful_send_latency_millisec_distribution",    "0-1000:0,1000-2000:1,2000-4000:1,4000-8000:0,8000-16000:0,16000-32000:0,>32000:0")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("normal_priority_log_to_successful_send_latency_millisec_max",             "2444")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("normal_priority_log_to_successful_send_latency_millisec_min",             "1444")));

    EXPECT_THAT(events[0].Extension, Contains(Pair("high_priority_records_sent_count",                                        toString(highPriorityCount))));
    EXPECT_THAT(events[0].Extension, Contains(Pair("high_priority_log_to_successful_send_latency_millisec_distribution",      "0-1000:2,1000-2000:0,2000-4000:1,4000-8000:0,8000-16000:0,16000-32000:0,>32000:0")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("high_priority_log_to_successful_send_latency_millisec_max",               "3555")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("high_priority_log_to_successful_send_latency_millisec_min",               "15")));

    EXPECT_THAT(events[0].Extension, Contains(Pair("immediate_priority_records_sent_count",                                   toString(immediatePriorityCount))));
    EXPECT_THAT(events[0].Extension, Contains(Pair("immediate_priority_log_to_successful_send_latency_millisec_distribution", "0-1000:1,1000-2000:0,2000-4000:0,4000-8000:0,8000-16000:0,16000-32000:0,>32000:0")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("immediate_priority_log_to_successful_send_latency_millisec_max",          "666")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("immediate_priority_log_to_successful_send_latency_millisec_min",          "666")));

    EXPECT_THAT(events[0].Extension, Contains(Pair("records_sent_count",                               toString(lowPriorityCount + normalPriorityCount + highPriorityCount + immediatePriorityCount))));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked",                                   "8")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_dropped",                           "2")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_dropped_on_HTTP_500",               "2")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_retried_on_HTTP_500",               "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_retried_on_HTTP_503",               "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_retried",                           "2")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_succeeded",                         "4")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_fail_on_HTTP_retries_count_distribution", "0:2,1:1,2:1,3:2")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_to_be_acked",                             "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("rm_bw_bytes_consumed_count",                       toString(postDataLength))));
    EXPECT_THAT(events[0].Extension, Contains(Pair("rtt_millisec_min",                                 "333")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("rtt_millisec_max",                                 "5555")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("rtt_millisec_distribution",                        "0-100:0,100-200:0,200-400:1,400-800:2,800-1600:0,1600-3200:0,>3200:1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("offline_storage_format_type",                      "MyStorage/Normal")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("offline_storage_last_failure",                     "reorg")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("records_dropped_offline_storage_overflow",         "12")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("records_dropped_retry_exceeded",                   "34")));
}

TEST_F(MetaStatsTests, GenerateStopEvent)
{
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
    stats.updateOnPostData(16, false);
    stats.updateOnPackageSentSucceeded(EventPriority_High, 1, 99, std::vector<unsigned>{ 100, 101, 102, 103, 104, 105, 106 }, false);
    stats.updateOnPackageFailed(501);
    stats.updateOnPackageFailed(403);
    stats.updateOnPackageRetry(505, 2);

    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_STOP);
    ASSERT_THAT(events, SizeIs(1));
    EXPECT_THAT(events[0].Type, Eq("client_telemetry"));
    EXPECT_THAT(events[0].EventType, Eq("act_stats"));
    EXPECT_THAT(events[0].Extension, Contains(Pair("EventInfo.Time", _)));
    EXPECT_THAT(events[0].Extension, Contains(Pair("DeviceInfo.OsName", _)));
    EXPECT_THAT(events[0].Extension, Contains(Pair("stats_rollup_kind",                                "stop")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("high_priority_records_sent_count",                 "7")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("records_sent_count",                               "7")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked",                                   "4")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_dropped",                           "2")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_dropped_on_HTTP_403",               "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_dropped_on_HTTP_501",               "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_retried",                           "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_succeeded",                         "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_fail_on_HTTP_retries_count_distribution", "1:1,2:1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_to_be_acked",                             "1")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("rm_bw_bytes_consumed_count",                       "16")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("rtt_millisec_distribution",                        "0-100:1,100-200:0,200-400:0,400-800:0,800-1600:0,1600-3200:0,>3200:0")));
}

TEST_F(MetaStatsTests, NoNewDataOrMetastatsOnlyGenerateNoEvents)
{
    EXPECT_CALL(runtimeConfigMock, GetMetaStatsSendIntervalSec())
        .WillRepeatedly(Return(123));

    // Send one normal event first to verify that stats are reset on generation.
    stats.updateOnEventIncoming(123, EventPriority_High, false);
    auto events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(1));

    // Verify nothing is generated if nothing happens.
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(0));
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(0));

    // Simulate logging and uploading some metastats events only. Nothing should be generated either.
    stats.updateOnEventIncoming(123, EventPriority_High, true);
    stats.updateOnEventIncoming(123, EventPriority_Normal, true);
    stats.updateOnPostData(123, true);
    stats.updateOnPackageSentSucceeded(EventPriority_High, 0, 123, std::vector<unsigned>{ 1234 }, true);
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(0));
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(0));

    // Verify events are generated again once some normal event arrives.
    stats.updateOnEventIncoming(123, EventPriority_High, false);
    // Even if the last record is metastats.
    stats.updateOnEventIncoming(123, EventPriority_High, true);
    events = stats.generateStatsEvent(ACT_STATS_ROLLUP_KIND_ONGOING);
    EXPECT_THAT(events, SizeIs(1));
    EXPECT_THAT(events[0].Extension, Contains(Pair("records_received_count",   "4")));
    EXPECT_THAT(events[0].Extension, Contains(Pair("requests_acked_succeeded", "1")));
}
