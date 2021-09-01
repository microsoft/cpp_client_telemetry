//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <memory>

#include "api/IRuntimeConfig.hpp"
#include "config/RuntimeConfig_Default.hpp"

namespace testing {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use override.
#endif

    class MockIRuntimeConfig : public MAT::RuntimeConfig_Default /* MAT::IRuntimeConfig */ {

    protected:
        std::unique_ptr<ILogConfiguration>& GetStaticConfig()
        {
            static std::unique_ptr<ILogConfiguration> staticConfig;
            return staticConfig;
        }

        MAT::ILogConfiguration& GetDefaultConfig()
        {
            std::unique_ptr<ILogConfiguration>& staticConfig = GetStaticConfig();
            if (!staticConfig)
            {
                staticConfig = std::make_unique<ILogConfiguration>();
            }
            return *staticConfig;
        }

    public:
        MockIRuntimeConfig() : MAT::RuntimeConfig_Default(GetDefaultConfig()) {}
        MockIRuntimeConfig(ILogConfiguration& customConfig) : MAT::RuntimeConfig_Default(customConfig) {}
        virtual ~MockIRuntimeConfig()
        {
            GetStaticConfig().reset(nullptr);
        }

        MOCK_METHOD1(SetDefaultConfig, void(IRuntimeConfig &));
        MOCK_METHOD3(DecorateEvent, void(std::map<std::string, std::string> &, std::string const &, std::string const &));
        MOCK_METHOD0(GetCollectorUrl, std::string());
        MOCK_METHOD0(GetMetaStatsTenantToken, std::string());
        MOCK_METHOD2(GetEventLatency, MAT::EventLatency(std::string const &, std::string const &));
        MOCK_METHOD0(GetMetaStatsSendIntervalSec, unsigned());
        MOCK_METHOD0(GetOfflineStorageMaximumSizeBytes, unsigned());
        MOCK_METHOD0(GetOfflineStorageResizeThresholdPct, unsigned());
        MOCK_METHOD0(GetMaximumRetryCount, unsigned());
        MOCK_METHOD0(GetUploadRetryBackoffConfig, std::string());
        MOCK_METHOD0(IsHttpRequestCompressionEnabled, bool());
        MOCK_METHOD0(GetMinimumUploadBandwidthBps, unsigned());
        MOCK_METHOD0(GetMaximumUploadSizeBytes, unsigned());
        MOCK_METHOD3(SetEventLatency, void(std::string const &, std::string const &, MAT::EventLatency));
        MOCK_METHOD0(GetTeardownTime, uint32_t());
        MOCK_METHOD0(IsClockSkewEnabled, bool());

        virtual MAT::Variant & operator[](const char* key)
        {
            return config[key];
        };

    };

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace testing

