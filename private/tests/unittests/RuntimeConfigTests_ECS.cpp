// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "common/MockIEcsClient.hpp"
#include "config/RuntimeConfig_ECS.hpp"
#include "config/RuntimeConfig_Default.hpp"

using namespace testing;


struct RuntimeConfigTests_ECS : public Test
{
    MockIEcsClient                    ecsClientMock;
    MAT::RuntimeConfig_Default defaultConfig;

    void expectRemoveListener(ecsclient::IEcsCallback* callback)
    {
        EXPECT_CALL(ecsClientMock, RemoveListener(callback))
            .WillOnce(Return(ecsclient::ECS_ERROR_OK));
    }

    virtual void SetUp() override
    {
        EXPECT_CALL(ecsClientMock, AddListener(_)).WillOnce(DoAll(
            Invoke(this, &RuntimeConfigTests_ECS::expectRemoveListener),
            Return(ecsclient::ECS_ERROR_OK)));
    }

    auf::IntrusivePtr<MockIEcsConfig> expectEcsConfig(unsigned omit)
    {
        auf::IntrusivePtr<MockIEcsConfig> config(new MockIEcsConfig(), false);

        EXPECT_CALL(*config, GetSetting(StrEq("SCT"), StrEq(""), StrEq("")))
            .WillOnce(ReturnArg<2>())
            .RetiresOnSaturation();
        if (~omit & 1) {
            EXPECT_CALL(*config, GetETag())
                .WillOnce(Return(""))
                .RetiresOnSaturation();
        }
        if (~omit & 2) {
            EXPECT_CALL(*config, GetKeys(StrEq("EventToConfigIdsMapping"), StrEq("")))
                .WillOnce(Return(std::vector<std::string>{}))
                .RetiresOnSaturation();
        }
        if (~omit & 4) {
            EXPECT_CALL(*config, GetKeys(StrEq("SCT"), StrEq("Priority")))
                .WillOnce(Return(std::vector<std::string>{}))
                .RetiresOnSaturation();
        }
        if (~omit & 8) {
            EXPECT_CALL(*config, GetSettingAsString(StrEq("SCT"), StrEq("TelemetryCollectorUrl"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 16) {
            EXPECT_CALL(*config, GetSettingAsInt(StrEq("SCT"), StrEq("OfflineStorageMaxSizeBytes"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 32) {
            EXPECT_CALL(*config, GetSettingAsInt(StrEq("SCT"), StrEq("OfflineStorageResizeThresholdPercent"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 64) {
            EXPECT_CALL(*config, GetSettingAsInt(StrEq("SCT"), StrEq("RetryMaximumCount"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 128) {
            EXPECT_CALL(*config, GetSettingAsBool(StrEq("SCT"), StrEq("NeedCompress"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 256) {
            EXPECT_CALL(*config, GetSettingAsString(StrEq("SCT"), StrEq("MetaStatsTenantToken"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 512) {
            EXPECT_CALL(*config, GetSettingAsInt(StrEq("SCT"), StrEq("SendFrequency/act_stats"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 1024) {
            EXPECT_CALL(*config, GetSettingAsString(StrEq("SCT"), StrEq("UploadRetryBackoffConfig"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 2048) {
            EXPECT_CALL(*config, GetSettingAsInt(StrEq("SCT"), StrEq("MinUploadBandwidthBytesPerSec"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }
        if (~omit & 4096) {
            EXPECT_CALL(*config, GetSettingAsInt(StrEq("SCT"), StrEq("MaxUploadSizeBytes"), _))
                .WillOnce(ReturnArg<2>())
                .RetiresOnSaturation();
        }

        return config;
    }

    auf::IntrusivePtr<MockIEcsConfig> expectInitialEcsConfig(unsigned omit)
    {
        auto ecsConfigMock = expectEcsConfig(omit);
        EXPECT_CALL(ecsClientMock, GetCurrentConfig()).
            WillOnce(Return(ecsConfigMock));
        return ecsConfigMock;
    }

    void sendConfigUpdatedEvent(ecsclient::IEcsCallback& callback, ecsclient::IEcsConfigPtr const& config)
    {
        ecsclient::IEcsCallback::CallBackContext context;
        context.currentConfig = config;
        callback.OnEvent(ecsclient::IEcsCallback::CONFIG_UPDATED, &context);
    }
};


TEST_F(RuntimeConfigTests_ECS, OnEventIgnoresOtherEvents)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);

    config.OnEvent(ecsclient::IEcsCallback::TOKEN_IS_INVALID,        nullptr);
    config.OnEvent(ecsclient::IEcsCallback::CallbackEventType(-1),   nullptr);
    config.OnEvent(ecsclient::IEcsCallback::CallbackEventType(1000), nullptr);

    EXPECT_THAT(config.GetEventPriority(), MAT::EventPriority_Unspecified);
}

//--- Basic properties tests

TEST_F(RuntimeConfigTests_ECS, GetCollectorUrlProvidesCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetCollectorUrl(), defaultConfig.GetCollectorUrl());
}

TEST_F(RuntimeConfigTests_ECS, GetCollectorUrlReturnsValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(8);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsString(StrEq("SCT"), StrEq("TelemetryCollectorUrl"), _))
        .WillOnce(Return("https://CollectorUrlFromEcs/Test"));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetCollectorUrl(), Eq("https://CollectorUrlFromEcs/Test"));
}

TEST_F(RuntimeConfigTests_ECS, GetOfflineStorageMaximumSizeBytesReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetOfflineStorageMaximumSizeBytes(), defaultConfig.GetOfflineStorageMaximumSizeBytes());
}

TEST_F(RuntimeConfigTests_ECS, GetOfflineStorageMaximumSizeBytesReturnsValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(16);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("OfflineStorageMaxSizeBytes"), _)).WillOnce(Return(76124374));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetOfflineStorageMaximumSizeBytes(), 76124374);
}

TEST_F(RuntimeConfigTests_ECS, GetOfflineStorageResizeThresholdPctReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetOfflineStorageResizeThresholdPct(), defaultConfig.GetOfflineStorageResizeThresholdPct());
}

TEST_F(RuntimeConfigTests_ECS, GetOfflineStorageResizeThresholdPctReturnsValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(32);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("OfflineStorageResizeThresholdPercent"), _)).WillOnce(Return(43));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetOfflineStorageResizeThresholdPct(), 43);
}

TEST_F(RuntimeConfigTests_ECS, GetMaximumRetryCountReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMaximumRetryCount(), defaultConfig.GetMaximumRetryCount());
}

TEST_F(RuntimeConfigTests_ECS, GetMaximumRetryCountReturnsValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(64);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("RetryMaximumCount"), _)).WillOnce(Return(7));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMaximumRetryCount(), 7);
}

TEST_F(RuntimeConfigTests_ECS, IsHttpRequestCompressionEnabledReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.IsHttpRequestCompressionEnabled(), defaultConfig.IsHttpRequestCompressionEnabled());
}

TEST_F(RuntimeConfigTests_ECS, IsHttpRequestCompressionEnabledReturnsValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(128);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsBool(StrEq("SCT"), StrEq("NeedCompress"), _)).WillOnce(Return(false));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.IsHttpRequestCompressionEnabled(), false);
}

TEST_F(RuntimeConfigTests_ECS, GetMinimumUploadBandwidthBpsReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMinimumUploadBandwidthBps(), defaultConfig.GetMinimumUploadBandwidthBps());
}

TEST_F(RuntimeConfigTests_ECS, GetMinimumUploadBandwidthBpsReturnsValueFromECS)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(2048);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("MinUploadBandwidthBytesPerSec"), _)).WillOnce(Return(123));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMinimumUploadBandwidthBps(), 123);
}

TEST_F(RuntimeConfigTests_ECS, GetMaximumUploadSizeBytesReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMaximumUploadSizeBytes(), defaultConfig.GetMaximumUploadSizeBytes());
}

TEST_F(RuntimeConfigTests_ECS, GetMaximumUploadSizeBytesReturnsValueFromECS)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(4096);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("MaxUploadSizeBytes"), _)).WillOnce(Return(123456));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMaximumUploadSizeBytes(), 123456);
}

TEST_F(RuntimeConfigTests_ECS, DecoratesEventWithEcsEtag)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(1);
    std::string const etag = "jhgJHG-98sc96-sdhcg^7^giU^7";
    EXPECT_CALL(*ecsConfigMock, GetETag()).WillOnce(Return(etag));
    config.SetDefaultConfig(defaultConfig);

    std::map<std::string, std::string> extension;
    config.DecorateEvent(extension, "", "");
    EXPECT_THAT(extension, Contains(Pair("AppInfo.EcsEtag", etag)));
}

TEST_F(RuntimeConfigTests_ECS, DecoratesEventWithExperimentationIds)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(2);
    std::string const eventConfigIds = "I-B-827354,346546,236,2346,I-F-23756238";
    EXPECT_CALL(*ecsConfigMock, GetKeys("EventToConfigIdsMapping", "")).WillOnce(Return(std::vector<std::string>{"aaa", "CustomEcsProject"}));
    EXPECT_CALL(*ecsConfigMock, GetKeys("EventToConfigIdsMapping", "aaa")).WillOnce(Return(std::vector<std::string>{"aaa_test"}));
    EXPECT_CALL(*ecsConfigMock, GetSettingAsString(StrEq("EventToConfigIdsMapping"), StrEq("aaa/aaa_test"), _)).WillOnce(Return("E-bbb"));
    EXPECT_CALL(*ecsConfigMock, GetKeys("EventToConfigIdsMapping", "CustomEcsProject")).WillOnce(Return(std::vector<std::string>{"CustomEventName", "CustomEnd"}));
    EXPECT_CALL(*ecsConfigMock, GetSettingAsString(StrEq("EventToConfigIdsMapping"), StrEq("CustomEcsProject/CustomEventName"), _)).WillOnce(Return(eventConfigIds));
    EXPECT_CALL(*ecsConfigMock, GetSettingAsString(StrEq("EventToConfigIdsMapping"), StrEq("CustomEcsProject/CustomEnd"), _)).WillOnce(Return("E-end"));
    config.SetDefaultConfig(defaultConfig);

    std::map<std::string, std::string> extension;
    config.DecorateEvent(extension, "CustomEcsProject", "CustomEventName");
    EXPECT_THAT(extension, Contains(Pair("AppInfo.ExperimentIds", eventConfigIds)));

    std::map<std::string, std::string> extension2;
    config.DecorateEvent(extension2, "CustomEcsProject", "aaa_test");
    EXPECT_THAT(extension2, Contains(Pair("AppInfo.ExperimentIds", "E-bbb")));
}

TEST_F(RuntimeConfigTests_ECS, DecoratesEventWithEverything)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    std::string const etag = "jhgJHG-98sc96-sdhcg^7^giU^7";
    std::string const eventConfigIds = "I-B-827354,346546,236,2346,I-F-23756238";
    auto ecsConfigMock = expectInitialEcsConfig(3);
    EXPECT_CALL(*ecsConfigMock, GetETag()).WillOnce(Return(etag));
    EXPECT_CALL(*ecsConfigMock, GetKeys("EventToConfigIdsMapping", "")).WillOnce(Return(std::vector<std::string>{"CustomEcsProject"}));
    EXPECT_CALL(*ecsConfigMock, GetKeys("EventToConfigIdsMapping", "CustomEcsProject")).WillOnce(Return(std::vector<std::string>{"CustomEventName"}));
    EXPECT_CALL(*ecsConfigMock, GetSettingAsString(StrEq("EventToConfigIdsMapping"), StrEq("CustomEcsProject/CustomEventName"), _)).WillOnce(Return(eventConfigIds));
    config.SetDefaultConfig(defaultConfig);

    std::map<std::string, std::string> extension;
    config.DecorateEvent(extension, "CustomEcsProject", "CustomEventName");

    EXPECT_THAT(extension, SizeIs(2));
    EXPECT_THAT(extension, Contains(Pair("AppInfo.EcsEtag", etag)));
    EXPECT_THAT(extension, Contains(Pair("AppInfo.ExperimentIds", eventConfigIds)));
}

TEST_F(RuntimeConfigTests_ECS, DoesNotDecorateEventWithEmptyValues)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);

    std::map<std::string, std::string> extension;
    config.DecorateEvent(extension, "CustomEcsProject", "CustomEventName");

    EXPECT_THAT(extension, SizeIs(0));
}

TEST_F(RuntimeConfigTests_ECS, GetMetaStatsTenantTokenReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMetaStatsTenantToken(), defaultConfig.GetMetaStatsTenantToken());
}

TEST_F(RuntimeConfigTests_ECS, GetMetaStatsTenantTokenReturnsValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(256);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsString(StrEq("SCT"), StrEq("MetaStatsTenantToken"), _)).WillOnce(Return("metastats-tenant-token"));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMetaStatsTenantToken(), StrEq("metastats-tenant-token"));
}

TEST_F(RuntimeConfigTests_ECS, GetMetaStatsSendIntervalSecReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMetaStatsSendIntervalSec(), defaultConfig.GetMetaStatsSendIntervalSec());
}

TEST_F(RuntimeConfigTests_ECS, GetMetaStatsSendIntervalSecReturnsValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(512);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("SendFrequency/act_stats"), _)).WillOnce(Return(0));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetMetaStatsSendIntervalSec(), 0u);

    auto ecsConfigMock1 = expectEcsConfig(512);
    EXPECT_CALL(*ecsConfigMock1, GetSettingAsInt(StrEq("SCT"), StrEq("SendFrequency/act_stats"), _)).WillOnce(Return(-2));
    sendConfigUpdatedEvent(config, ecsConfigMock1);
    EXPECT_THAT(config.GetMetaStatsSendIntervalSec(), 0u);

    auto ecsConfigMock2 = expectEcsConfig(512);
    EXPECT_CALL(*ecsConfigMock2, GetSettingAsInt(StrEq("SCT"), StrEq("SendFrequency/act_stats"), _)).WillOnce(ReturnArg<2>());
    sendConfigUpdatedEvent(config, ecsConfigMock2);
    EXPECT_THAT(config.GetMetaStatsSendIntervalSec(), defaultConfig.GetMetaStatsSendIntervalSec());

    auto ecsConfigMock3 = expectEcsConfig(512);
    EXPECT_CALL(*ecsConfigMock3, GetSettingAsInt(StrEq("SCT"), StrEq("SendFrequency/act_stats"), _)).WillOnce(Return(123));
    sendConfigUpdatedEvent(config, ecsConfigMock3);
    EXPECT_THAT(config.GetMetaStatsSendIntervalSec(), 123u);
}

TEST_F(RuntimeConfigTests_ECS, GetUploadRetryBackoffConfigReturnsCorrectDefaultValue)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetUploadRetryBackoffConfig(), defaultConfig.GetUploadRetryBackoffConfig());
}

TEST_F(RuntimeConfigTests_ECS, GetUploadRetryBackoffConfigValueFromEcs)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    auto ecsConfigMock = expectInitialEcsConfig(1024);
    EXPECT_CALL(*ecsConfigMock, GetSettingAsString(StrEq("SCT"), StrEq("UploadRetryBackoffConfig"), _)).WillOnce(Return("I,am,backoff,config"));
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetUploadRetryBackoffConfig(), StrEq("I,am,backoff,config"));
}

//--- Priority tests

TEST_F(RuntimeConfigTests_ECS, GetDefaultEventPriorityWithEmptyMap)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetEventPriority(), MAT::EventPriority_Unspecified);
    EXPECT_THAT(config.GetEventPriority("1", "3"), MAT::EventPriority_Unspecified);
}

TEST_F(RuntimeConfigTests_ECS, GetEventPriorityReturnsValueFromInitialMap)
{
    std::string const customTenantId = "sdjkgfkj";
    std::string const customEvent    = "kg5khg2kh5v2";
    MAT::RuntimeConfig_ECS::EventPriorityMap initialMap = {
        { MAT::RuntimeConfig_ECS::EventKey{"",             ""         }, MAT::EventPriority_Off       },
        { MAT::RuntimeConfig_ECS::EventKey{customTenantId, ""         }, MAT::EventPriority_Low       },
        { MAT::RuntimeConfig_ECS::EventKey{customTenantId, customEvent}, MAT::EventPriority_Immediate },
    };
    MAT::RuntimeConfig_ECS config(ecsClientMock, initialMap);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);
    EXPECT_THAT(config.GetEventPriority(),                            MAT::EventPriority_Off);
    EXPECT_THAT(config.GetEventPriority("1",            "3"),         MAT::EventPriority_Off);
    EXPECT_THAT(config.GetEventPriority(customTenantId),              MAT::EventPriority_Low);
    EXPECT_THAT(config.GetEventPriority(customTenantId, "2"),         MAT::EventPriority_Low);
    EXPECT_THAT(config.GetEventPriority(customTenantId, "3"),         MAT::EventPriority_Low);
    EXPECT_THAT(config.GetEventPriority(customTenantId, customEvent), MAT::EventPriority_Immediate);
}

TEST_F(RuntimeConfigTests_ECS, GetEventPriorityReturnsValueFromEcsMap)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);

    auto ecsConfigMock = expectEcsConfig(4);

    EXPECT_CALL(*ecsConfigMock, GetKeys(StrEq("SCT"), StrEq("Priority")))
        .WillOnce(Return(std::vector<std::string>{"tenantId1", "tenantId2"}));

    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("Priority/tenantId1/Default"), _))
        .WillOnce(Return(-2)); // will be converted to 0 (EventPriority_Off)

    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("Priority/tenantId2/Default"), _))
        .WillOnce(Return(MAT::EventPriority_High));
    EXPECT_CALL(*ecsConfigMock, GetKeys(StrEq("SCT"), StrEq("Priority/tenantId2/Events")))
        .WillOnce(Return(std::vector<std::string>{"event1", "event2", "event3", "event4"}));
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("Priority/tenantId2/Events/event1"), _))
        .WillOnce(Return(-100)); // will be converted to 0 (EventPriority_Off)
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("Priority/tenantId2/Events/event2"), _))
        .WillOnce(Return(-1)); // will be left -1 (EventPriority_Unspecified)
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("Priority/tenantId2/Events/event3"), _))
        .WillOnce(Return(MAT::EventPriority_Normal));
    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("Priority/tenantId2/Events/event4"), _))
        .WillOnce(Return(1000)); // will be converted to EventPriority_Immediate

    sendConfigUpdatedEvent(config, ecsConfigMock);

    EXPECT_THAT(config.GetEventPriority(),                            MAT::EventPriority_Unspecified);
    EXPECT_THAT(config.GetEventPriority("tenantId1", ""),              MAT::EventPriority_Off);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "event1"),        MAT::EventPriority_Off);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "unknown event"), MAT::EventPriority_High);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "event1"),        MAT::EventPriority_Off);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "event2"),        MAT::EventPriority_Unspecified);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "event3"),        MAT::EventPriority_Normal);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "event4"),        MAT::EventPriority_Immediate);
}

TEST_F(RuntimeConfigTests_ECS, GetEventPriorityRespectsEcsWithHighestPriority)
{
    MAT::RuntimeConfig_ECS::EventPriorityMap initialMap = {
        { MAT::RuntimeConfig_ECS::EventKey{"tenantId1", "event1"}, MAT::EventPriority_Low    },
        { MAT::RuntimeConfig_ECS::EventKey{"tenantId2", "event2"}, MAT::EventPriority_Normal }
    };
    expectInitialEcsConfig(0);
    MAT::RuntimeConfig_ECS config(ecsClientMock, initialMap);
    config.SetDefaultConfig(defaultConfig);

    config.SetEventPriority("tenantId1", "event1", MAT::EventPriority_High);
    config.SetEventPriority("tenantId2", "",       MAT::EventPriority_High);

    auto ecsConfigMock = expectEcsConfig(4);

    EXPECT_CALL(*ecsConfigMock, GetKeys(StrEq("SCT"), StrEq("Priority")))
        .WillOnce(Return(std::vector<std::string>{"tenantId1"}));

    EXPECT_CALL(*ecsConfigMock, GetSettingAsInt(StrEq("SCT"), StrEq("Priority/tenantId1/Default"), _))
        .WillOnce(Return(MAT::EventPriority_Immediate));

    EXPECT_CALL(*ecsConfigMock, GetKeys(StrEq("SCT"), StrEq("Priority/tenantId1/Events")))
        .WillOnce(Return(std::vector<std::string>()));

    sendConfigUpdatedEvent(config, ecsConfigMock);

    EXPECT_THAT(config.GetEventPriority("tenantId1", "event1"), MAT::EventPriority_Immediate);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "event2"), MAT::EventPriority_Normal);
}

TEST_F(RuntimeConfigTests_ECS, GetEventPriorityRespectsInitialConfigWithHigherPriority)
{
    MAT::RuntimeConfig_ECS::EventPriorityMap initialMap = {
        { MAT::RuntimeConfig_ECS::EventKey{"tenantId1", "event1"}, MAT::EventPriority_Low    },
        { MAT::RuntimeConfig_ECS::EventKey{"tenantId2", "event2"}, MAT::EventPriority_Normal }
    };
    MAT::RuntimeConfig_ECS config(ecsClientMock, initialMap);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);

    config.SetEventPriority("tenantId1", "event1", MAT::EventPriority_High);
    config.SetEventPriority("tenantId2", "",       MAT::EventPriority_High);

    EXPECT_THAT(config.GetEventPriority("tenantId1", "event1"), MAT::EventPriority_Low);
    EXPECT_THAT(config.GetEventPriority("tenantId2", "event2"), MAT::EventPriority_Normal);
}

TEST_F(RuntimeConfigTests_ECS, SetEventPrioritySetsCustomPriorityForTheEvent)
{
    MAT::RuntimeConfig_ECS config(ecsClientMock);
    expectInitialEcsConfig(0);
    config.SetDefaultConfig(defaultConfig);

    config.SetEventPriority("tenantId1", "HighPrioEvent", MAT::EventPriority_High);
    config.SetEventPriority("tenantId1", "LowPrioEvent",  MAT::EventPriority_Low);
    config.SetEventPriority("tenantId1", "",              MAT::EventPriority_Normal);
    config.SetEventPriority("",          "",              MAT::EventPriority_Immediate);

    EXPECT_THAT(config.GetEventPriority("tenantId1",        "HighPrioEvent"), MAT::EventPriority_High);
    EXPECT_THAT(config.GetEventPriority("tenantId1",        "LowPrioEvent"),  MAT::EventPriority_Low);
    EXPECT_THAT(config.GetEventPriority("tenantId1",        ""),              MAT::EventPriority_Normal);
    EXPECT_THAT(config.GetEventPriority("tenantId1",        "AnyOtherEvent"), MAT::EventPriority_Normal);
    EXPECT_THAT(config.GetEventPriority("",                 ""),              MAT::EventPriority_Immediate);
    EXPECT_THAT(config.GetEventPriority("AnyOtherTenantId", "AnyOtherEvent"), MAT::EventPriority_Immediate);
}
