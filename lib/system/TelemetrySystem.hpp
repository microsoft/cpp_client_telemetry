// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "ITelemetrySystem.hpp"
#include <Version.hpp>
#include "bond/BondSerializer.hpp"
#include "compression/HttpDeflateCompression.hpp"
#include "http/HttpClientManager.hpp"
#include "http/HttpRequestEncoder.hpp"
#include "http/HttpResponseDecoder.hpp"
#include "offline/OfflineStorage.hpp"
#include "packager/Packager.hpp"
#include "stats/Statistics.hpp"
#include "tpm/TransmissionPolicyManager.hpp"
#include "ClockSkewDelta.h"

namespace ARIASDK_NS_BEGIN {


class TelemetrySystem : public PAL::RefCountedImpl<TelemetrySystem>,
                        public ITelemetrySystem
{
  public:
    TelemetrySystem(LogConfiguration& configuration, IRuntimeConfig& runtimeConfig, IOfflineStorage& offlineStorage,
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
