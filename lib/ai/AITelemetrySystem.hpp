// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include "pal/PAL.hpp"

#include "system/TelemetrySystemBase.hpp"

#include "bond/BondSerializer.hpp"

#ifdef HAVE_MAT_ZLIB
#include "compression/HttpDeflateCompression.hpp"
#endif

#include "http/HttpClientManager.hpp"
#include "offline/StorageObserver.hpp"
#include "IOfflineStorage.hpp"
#include "ITaskDispatcher.hpp"

#include "packager/Packager.hpp"

#include "tpm/TransmissionPolicyManager.hpp"
#include "system/ClockSkewDelta.h"

#include "AIJsonSerializer.hpp"
#include "AIHttpRequestEncoder.hpp"
#include "AIHttpResponseDecoder.hpp"
#include "AIPackager.hpp"

namespace ARIASDK_NS_BEGIN {

    class AITelemetrySystem : public TelemetrySystemBase
    {

    public:

        AITelemetrySystem(
                ILogManager& logManager,
                IRuntimeConfig& runtimeConfig,
                IOfflineStorage& offlineStorage,
                IHttpClient& httpClient,
                ITaskDispatcher& taskDispatcher,
                IBandwidthController* bandwidthController
        );

        ~AITelemetrySystem();

        virtual bool upload() override;
        virtual void handleIncomingEventPrepared(IncomingEventContextPtr const& event) override;

    protected:

        virtual void handleFlushTaskDispatcher() override;

#ifdef HAVE_MAT_ZLIB
        HttpDeflateCompression    compression;
#else
        NullCompression           compression;
#endif

        HttpClientManager         hcm;
        AIHttpRequestEncoder      httpEncoder;
        AIHttpResponseDecoder       httpDecoder;
        StorageObserver           storage;
        AIPackager                packager;
        TransmissionPolicyManager tpm;
        ClockSkewDelta            clockSkewDelta;
        AIJsonSerializer          aiJsonSerializer;

    public:
        RouteSink<AITelemetrySystem>                                 flushTaskDispatcher{ this, &AITelemetrySystem::handleFlushTaskDispatcher };
        RouteSink<AITelemetrySystem, IncomingEventContextPtr const&> incomingEventPrepared{ this, &AITelemetrySystem::handleIncomingEventPrepared };
    };

} ARIASDK_NS_END
