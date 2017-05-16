// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/Version.hpp>
#include "bond/BondSerializer.hpp"

namespace ARIASDK_NS_BEGIN {


class ITelemetrySystem 
{
  public:
    virtual ~ITelemetrySystem() {};

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void pauseTransmission() = 0;
    virtual void resumeTransmission() = 0;
    virtual void UploadNow() = 0;

  protected:
    virtual void startAsync() = 0;
    virtual void stopAsync() = 0;
    virtual void handleFlushWorkerThread() = 0;
    virtual void signalDoneEvent() = 0;
    virtual void pauseTransmissionAsync() = 0;
    virtual void resumeTransmissionAsync() = 0;
    virtual void handleIncomingEventPrepared(IncomingEventContextPtr const& event) = 0;
    virtual void preparedIncomingEventAsync(IncomingEventContextPtr const& event) = 0;
  
};


} ARIASDK_NS_END
