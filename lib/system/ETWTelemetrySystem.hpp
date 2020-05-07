// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "mat/config.h"
#ifdef HAVE_MAT_ETW

#include "pal/PAL.hpp"

#include "system/TelemetrySystemBase.hpp"
#include "LogConfiguration.hpp"
#include "bond/BondSerializer.hpp"
#include "system/Contexts.hpp"

#include <map>

namespace ARIASDK_NS_BEGIN
{

    class ETWTelemetrySystem : public TelemetrySystemBase
    {

    public:

        ETWTelemetrySystem(
            ILogManager& logManager,
            IRuntimeConfig& runtimeConfig,
            ITaskDispatcher& taskDispatcher
            // No Offline storage DB
            // No HTTP client
            // No bandwidth controller
        );

        ~ETWTelemetrySystem();

    protected:

        void handleIncomingEventPrepared(IncomingEventContextPtr const& event) override;

        int sendEventToETW(IncomingEventContextPtr const& eventCtx);

    public:
        RouteSink<ETWTelemetrySystem, IncomingEventContextPtr const&> incomingEventPrepared{ this, &ETWTelemetrySystem::handleIncomingEventPrepared };
    };

} ARIASDK_NS_END

#endif