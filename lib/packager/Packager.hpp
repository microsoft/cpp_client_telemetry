// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/IOfflineStorage.hpp>
#include <aria/LogConfiguration.hpp>
#include "system/Route.hpp"
#include "system/Contexts.hpp"

namespace ARIASDK_NS_BEGIN {


class Packager {
  public:
    Packager(LogConfiguration const& configuration, IRuntimeConfig const& runtimeConfig);
    ~Packager();

  protected:
    void handleAddEventToPackage(EventsUploadContextPtr const& ctx, StorageRecord const& record, bool& wantMore);
    void handleFinalizePackage(EventsUploadContextPtr const& ctx);

  protected:
    IRuntimeConfig const& m_runtimeConfig;
    std::string           m_forcedTenantToken;

  public:
    RouteSink<Packager, EventsUploadContextPtr const&, StorageRecord const&, bool&> addEventToPackage{this, &Packager::handleAddEventToPackage};
    RouteSink<Packager, EventsUploadContextPtr const&>                              finalizePackage{this, &Packager::handleFinalizePackage};

    RouteSource<EventsUploadContextPtr const&>                                      emptyPackage;
    RouteSource<EventsUploadContextPtr const&>                                      packagedEvents;
};


} ARIASDK_NS_END
