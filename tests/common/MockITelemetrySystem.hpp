//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <system/ITelemetrySystem.hpp>
#include "config/RuntimeConfig_Default.hpp"
#include "NullObjects.hpp"

using namespace MAT;

namespace testing {

#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override" // GMock MOCK_METHOD* macros don't use override.
#endif

    class MockITelemetrySystem : public ITelemetrySystem {
    public:
        MockITelemetrySystem() {};
        virtual ~MockITelemetrySystem() {};

        MOCK_METHOD0(start, void());
        MOCK_METHOD0(stop, void());
        MOCK_METHOD0(pause, void());
        MOCK_METHOD0(resume, void());
        MOCK_METHOD0(upload, bool());
        MOCK_METHOD0(cleanup, void());

        // MOCK_METHOD0(getLogManager, ILogManager&());
        ILogManager& getLogManager()
        {
            static NullLogManager nullLogManager;
            return nullLogManager;
        }

        // MOCK_METHOD0(getConfig, IRuntimeConfig&());        
        IRuntimeConfig& getConfig()
        {
            static ILogConfiguration & config = getLogManager().GetLogConfiguration();
            static RuntimeConfig_Default testConfig(config);
            return testConfig;
        }
        
        EventsUploadContextPtr createEventsUploadContext() override
        {
            return std::make_shared<EventsUploadContext>();
        }

        MOCK_METHOD0(getContext, ISemanticContext&());
        MOCK_METHOD1(DispatchEvent, bool(DebugEvent evt));
        MOCK_METHOD1(sendEvent, void(IncomingEventContextPtr const& event));
        MOCK_METHOD0(startAsync, void());
        MOCK_METHOD0(stopAsync, void());
        MOCK_METHOD0(handleFlushTaskDispatcher, void());
        MOCK_METHOD0(signalDone, void());
        MOCK_METHOD0(pauseAsync, void());
        MOCK_METHOD0(resumeAsync, void());
        MOCK_METHOD1(handleIncomingEventPrepared, void(IncomingEventContextPtr const& event));
        MOCK_METHOD1(preparedIncomingEventAsync, void(IncomingEventContextPtr const& event));
    };

#if defined( __clang__ )
#pragma clang diagnostic pop
#endif

}  // namespace MAT_NS_BEGIN

