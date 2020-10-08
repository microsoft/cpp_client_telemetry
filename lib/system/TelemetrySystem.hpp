//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

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
#include "offline/LogSessionDataProvider.hpp"
#include "IOfflineStorage.hpp"
#include "ITaskDispatcher.hpp"

#include "packager/Packager.hpp"

#include "tpm/TransmissionPolicyManager.hpp"
#include "ClockSkewDelta.h"

namespace MAT_NS_BEGIN {

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
            ITaskDispatcher& taskDispatcher,
            IBandwidthController* bandwidthController,
            LogSessionDataProvider& logSessionDataProvider
        );

        ~TelemetrySystem();

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
        HttpRequestEncoder        httpEncoder;
        HttpResponseDecoder       httpDecoder;
        StorageObserver           storage;
        Packager                  packager;
        TransmissionPolicyManager tpm;
        ClockSkewDelta            clockSkewDelta;

    public:
        RouteSink<TelemetrySystem>                                 flushTaskDispatcher{ this, &TelemetrySystem::handleFlushTaskDispatcher };
        RouteSink<TelemetrySystem, IncomingEventContextPtr const&> incomingEventPrepared{ this, &TelemetrySystem::handleIncomingEventPrepared };
    };

} MAT_NS_END

