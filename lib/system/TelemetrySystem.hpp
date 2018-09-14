// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include <pal/PAL.hpp>

#include "system/TelemetrySystemBase.hpp"

#include "bond/BondSerializer.hpp"
#include "compression/HttpDeflateCompression.hpp"

#include "http/HttpClientManager.hpp"
#include "http/HttpRequestEncoder.hpp"
#include "http/HttpResponseDecoder.hpp"

#include "offline/StorageObserver.hpp"
#include "IOfflineStorage.hpp"

#include "packager/Packager.hpp"

#include "tpm/TransmissionPolicyManager.hpp"
#include "ClockSkewDelta.h"

namespace ARIASDK_NS_BEGIN {

    class TelemetrySystem : public TelemetrySystemBase
    {

    public:

        TelemetrySystem(
            ILogManager& logManager,
            IRuntimeConfig& runtimeConfig,
            IOfflineStorage& offlineStorage,
            IHttpClient& httpClient,
            IBandwidthController* bandwidthController
        );

        ~TelemetrySystem();

        virtual bool upload() override;
        virtual void handleIncomingEventPrepared(IncomingEventContextPtr const& event) override;

    protected:

        virtual void handleFlushWorkerThread() override;

        HttpDeflateCompression    compression;
        HttpClientManager         hcm;
        HttpRequestEncoder        httpEncoder;
        HttpResponseDecoder       httpDecoder;
        StorageObserver           storage;
        Packager                  packager;
        TransmissionPolicyManager tpm;
        ClockSkewDelta            clockSkewDelta;

    public:
        RouteSink<TelemetrySystem>                                 flushWorkerThread{ this, &TelemetrySystem::handleFlushWorkerThread };
        RouteSink<TelemetrySystem, IncomingEventContextPtr const&> incomingEventPrepared{ this, &TelemetrySystem::handleIncomingEventPrepared };
    };

} ARIASDK_NS_END
