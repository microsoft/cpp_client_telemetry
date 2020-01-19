// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IOfflineStorage.hpp"
#include "LogConfiguration.hpp"

#include "api/IRuntimeConfig.hpp"

#include "system/Contexts.hpp"
#include "system/Route.hpp"

#include "exporters/ISerializer.hpp"
#include "exporters/ISplicer.hpp"

namespace ARIASDK_NS_BEGIN
{
    class Packager
    {
       public:
        Packager(IRuntimeConfig& runtimeConfig);
        ~Packager();

       protected:
        void handleAddEventToPackage(EventsUploadContextPtr const& ctx, StorageRecord const& record, bool& wantMore);
        void handleFinalizePackage(EventsUploadContextPtr const& ctx);

       protected:
        IRuntimeConfig& m_config;
        ISplicer& m_splicer;
        std::string m_forcedTenantToken;

       public:
        RouteSink<Packager, EventsUploadContextPtr const&, StorageRecord const&, bool&> addEventToPackage{this, &Packager::handleAddEventToPackage};
        RouteSink<Packager, EventsUploadContextPtr const&> finalizePackage{this, &Packager::handleFinalizePackage};

        RouteSource<EventsUploadContextPtr const&> emptyPackage;
        RouteSource<EventsUploadContextPtr const&> packagedEvents;
    };

}
ARIASDK_NS_END
