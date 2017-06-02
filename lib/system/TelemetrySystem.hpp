// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "ITelemetrySystem.hpp"
#include <Version.hpp>
#include "BondSerializer.hpp"
#include "HttpDeflateCompression.hpp"
#include "HttpClientManager.hpp"
#include "HttpRequestEncoder.hpp"
#include "HttpResponseDecoder.hpp"
#include "OfflineStorage.hpp"
#include "Packager.hpp"
#include "Statistics.hpp"
#include "TransmissionPolicyManager.hpp"
#include "ClockSkewDelta.h"
#if ARIASDK_UTC_ENABLED
    #include "utc/UtcForwarder.hpp"
#endif

namespace ARIASDK_NS_BEGIN {


class TelemetrySystem : public PAL::RefCountedImpl<TelemetrySystem>,
                        public ITelemetrySystem
{
  public:
    TelemetrySystem(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig, IOfflineStorage& offlineStorage,
        IHttpClient& httpClient, ContextFieldsProvider const& globalContext, IBandwidthController* bandwidthController);
    ~TelemetrySystem();

  public:
    void start();
    void stop();
    void pauseTransmission();
    void resumeTransmission();
    void UploadNow();
    void addIncomingEventSystem(IncomingEventContextPtr const& event);

  protected:
    void startAsync();
    void stopAsync();
    void handleFlushWorkerThread();
    void signalDoneEvent();
    void pauseTransmissionAsync();
    void resumeTransmissionAsync();
    void handleIncomingEventPrepared(IncomingEventContextPtr const& event);
    void preparedIncomingEventAsync(IncomingEventContextPtr const& event);

  protected:
    bool                      m_isPaused;
    PAL::Event                m_doneEvent;

    BondSerializer            bondSerializer;
    HttpDeflateCompression    compression;
    HttpClientManager         hcm;
    HttpRequestEncoder        httpEncoder;
    HttpResponseDecoder       httpDecoder;
    OfflineStorage            storage;
    Packager                  packager;
    Statistics                stats;
    TransmissionPolicyManager tpm;
    ClockSkewDelta            clockSkewDelta;
    LogConfiguration          configuration;
#if ARIASDK_UTC_ENABLED
    UtcForwarder              utcForwarder;
#endif

  public:
    RouteSource<>                                              started;
    RouteSource<>                                              stopped;
    RouteSource<>                                              paused;
    RouteSource<>                                              resumed;

    RouteSource<IncomingEventContextPtr const&>                addIncomingEvent;
    RouteSink<TelemetrySystem, IncomingEventContextPtr const&> incomingEventPrepared{this, &TelemetrySystem::handleIncomingEventPrepared};

    RouteSource<IncomingEventContextPtr const&>                preparedIncomingEvent;

    RouteSink<TelemetrySystem>                                 flushWorkerThread{this, &TelemetrySystem::handleFlushWorkerThread};
};


} ARIASDK_NS_END
