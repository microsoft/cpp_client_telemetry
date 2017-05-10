// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/IRuntimeConfig.hpp>

namespace testing {


class MockIRuntimeConfig : public ARIASDK_NS::IRuntimeConfig {
  public:
    MockIRuntimeConfig();
    virtual ~MockIRuntimeConfig();

    MOCK_METHOD1(SetDefaultConfig, void(IRuntimeConfig &));
    MOCK_CONST_METHOD3(DecorateEvent, void(std::map<std::string, std::string> &, std::string const &, std::string const &));
    MOCK_CONST_METHOD0(GetCollectorUrl, std::string());
    MOCK_CONST_METHOD0(GetMetaStatsTenantToken, std::string());
    MOCK_CONST_METHOD2(GetEventPriority, ARIASDK_NS::EventPriority(std::string const &, std::string const &));
    MOCK_CONST_METHOD0(GetMetaStatsSendIntervalSec, unsigned());
    MOCK_CONST_METHOD0(GetOfflineStorageMaximumSizeBytes, unsigned());
    MOCK_CONST_METHOD0(GetOfflineStorageResizeThresholdPct, unsigned());
    MOCK_CONST_METHOD0(GetMaximumRetryCount, unsigned());
    MOCK_CONST_METHOD0(GetUploadRetryBackoffConfig, std::string());
    MOCK_CONST_METHOD0(IsHttpRequestCompressionEnabled, bool());
    MOCK_CONST_METHOD0(GetMinimumUploadBandwidthBps, unsigned());
    MOCK_CONST_METHOD0(GetMaximumUploadSizeBytes, unsigned());
    MOCK_METHOD3(SetEventPriority, void(std::string const &, std::string const &, ARIASDK_NS::EventPriority));
	bool IsClockSkewEnabled() const override
	{
		return false;
	}
};


} // namespace testing
