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
#include <vector>

#include "modules/utc/traceloggingdynamic.h"

using namespace tld;

namespace ARIASDK_NS_BEGIN
{

    typedef struct
    {
        ULONGLONG providerHandle = 0;
        std::vector<BYTE> providerMetaVector;
        GUID providerGuid;
    } ETWProviderData;

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

        std::map<std::string, ETWProviderData> providers;

        void handleIncomingEventPrepared(IncomingEventContextPtr const& event) override;

        int sendEventToETW(IncomingEventContextPtr const& eventCtx);

        ETWProviderData& getProviderForToken(const std::string& token);

        ETWProviderData& registerProviderForTenant(const std::string& tenant);

        status_t unregisterProvider(ETWProviderData& data);

        void PutData(std::vector<::CsProtocol::Data>& ext,
                     std::vector<std::string>& MD,
                     tld::EventMetadataBuilder<std::vector<BYTE>>& builder,
                     tld::EventDataBuilder<std::vector<BYTE>>& dbuilder);

    public:
        RouteSink<ETWTelemetrySystem, IncomingEventContextPtr const&> incomingEventPrepared{ this, &ETWTelemetrySystem::handleIncomingEventPrepared };
    };

} ARIASDK_NS_END

#endif