//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "api/IRuntimeConfig.hpp"

#include "system/Route.hpp"
#include "system/Contexts.hpp"

namespace MAT_NS_BEGIN {

    class Packager {
    public:
        Packager(IRuntimeConfig& runtimeConfig);

    protected:
        void handleAddEventToPackage(EventsUploadContextPtr const& ctx, StorageRecord const& record, bool& wantMore);
        void handleFinalizePackage(EventsUploadContextPtr const& ctx);

    protected:
        IRuntimeConfig & m_config;
        std::string      m_forcedTenantToken;

    public:
        RouteSink<Packager, EventsUploadContextPtr const&, StorageRecord const&, bool&> addEventToPackage{ this, &Packager::handleAddEventToPackage };
        RouteSink<Packager, EventsUploadContextPtr const&>                              finalizePackage{ this, &Packager::handleFinalizePackage };

        RouteSource<EventsUploadContextPtr const&>                                      emptyPackage;
        RouteSource<EventsUploadContextPtr const&>                                      packagedEvents;
    };


} MAT_NS_END

