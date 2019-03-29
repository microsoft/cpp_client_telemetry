// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include "pal/PAL.hpp"

#include "system/TelemetrySystemBase.hpp"

#include "bond/BondSerializer.hpp"

#ifdef HAVE_MAT_ZLIB
#include "compression/HttpDeflateCompression.hpp"
#endif

#include "http/HttpClientManager.hpp"
#include "http/HttpRequestEncoder.hpp"
#include "http/HttpResponseDecoder.hpp"

#include "offline/StorageObserver.hpp"
#include "IOfflineStorage.hpp"

#include "packager/Packager.hpp"

#include "tpm/TransmissionPolicyManager.hpp"
#include "ClockSkewDelta.h"

namespace ARIASDK_NS_BEGIN {

    class NullCompression
    {
    public:
          NullCompression(IRuntimeConfig & ) {};
    };

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

#ifdef HAVE_MAT_ZLIB
        HttpDeflateCompression    compression;
#else
        NullCompression           compression;
#endif

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
