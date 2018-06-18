// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "system/Contexts.hpp"
#include "system/Route.hpp"
#include <wrl.h>

// Telemetry is defined as a macro in <aria/Version.hpp>, use a namespace alias
#pragma push_macro("Telemetry")
#include <Windows.System.Diagnostics.Telemetry.h>
namespace AWSDT = ABI::Windows::System::Diagnostics::Telemetry;
#pragma pop_macro("Telemetry")

#include <mutex>

// From <evntprov.h>
typedef unsigned __int64 REGHANDLE, * PREGHANDLE;

namespace ARIASDK_NS_BEGIN {

class UtcForwarder {
  public:
    UtcForwarder();
    ~UtcForwarder();

  protected:
    bool handleForwardIfAvailable(IncomingEventContextPtr const& ctx);
    bool registerTenantWithUtc(std::string const& ikey, uint32_t storageSize, uint32_t uploadQuotaSize);
    bool convertProperties(::AriaProtocol::Record& record, std::string const& iKey, UINT32 eventTags, std::vector<UINT8>& metadata, std::vector<UINT8>& data);

  protected:
    struct TenantInfo {
        std::vector<UINT8> providerMetadata;
        REGHANDLE          hProvider  = NULL;
        bool               seen       = false;
        bool               registered = false;
    };

  protected:
    std::mutex                                                     m_lock;
    std::map<std::string, TenantInfo>                              m_tenantInfo;
    Microsoft::WRL::ComPtr<AWSDT::IPlatformTelemetryClientStatics> m_platformTelemetryClient;

  public:
    RoutePassThrough<UtcForwarder, IncomingEventContextPtr const&> forwardIfAvailable{this, &UtcForwarder::handleForwardIfAvailable};
};


} ARIASDK_NS_END
