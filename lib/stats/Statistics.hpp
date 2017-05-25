// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/IRuntimeConfig.hpp>
#include "system\ITelemetrySystem.hpp"
#include "MetaStats.hpp"
#include "system/Route.hpp"
#include "system/Contexts.hpp"
#include "pal/PAL.hpp"

namespace ARIASDK_NS_BEGIN {


class Statistics : public PAL::RefCountedImpl<Statistics> {
  public:
    Statistics(IRuntimeConfig& runtimeConfig, ContextFieldsProvider const& globalContext, ITelemetrySystem*   telemetrySystem);
    ~Statistics();

  protected:
    virtual void scheduleSend();
    void send(ActRollUpKind rollupKind);

    bool handleOnStart();
    bool handleOnStop();

    bool handleOnIncomingEventAccepted(IncomingEventContextPtr const& ctx);
    bool handleOnIncomingEventFailed(IncomingEventContextPtr const& ctx);

    bool handleOnUploadStarted(EventsUploadContextPtr const& ctx);
    bool handleOnPackagingFailed(EventsUploadContextPtr const& ctx);
    bool handleOnUploadSuccessful(EventsUploadContextPtr const& ctx);
    bool handleOnUploadRejected(EventsUploadContextPtr const& ctx);
    bool handleOnUploadFailed(EventsUploadContextPtr const& ctx);

    bool handleOnStorageOpened(StorageNotificationContext const* ctx);
    bool handleOnStorageFailed(StorageNotificationContext const* ctx);
    bool handleOnStorageTrimmed(StorageNotificationContext const* ctx);
    bool handleOnStorageRecordsDropped(StorageNotificationContext const* ctx);

  protected:
    MetaStats                   m_metaStats;
    IRuntimeConfig&             m_runtimeConfig;
    PAL::DeferredCallbackHandle m_scheduledSend;
    bool                        m_isScheduled;
    bool                        m_isStarted;
    ITelemetrySystem*           m_iTelemetrySystem;
    std::int64_t                m_statEventSentTime;

  public:
    RouteSource<IncomingEventContextPtr const&>                     eventGenerated;

    RouteSource<>                                                   onStartupDone;
    RoutePassThrough<Statistics>                                    onStart{this, &Statistics::handleOnStart};
    RouteSource<>                                                   onShutdownDone;
    RoutePassThrough<Statistics>                                    onStop{this, &Statistics::handleOnStop};

    RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventAccepted{this, &Statistics::handleOnIncomingEventAccepted};
    RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventFailed{this, &Statistics::handleOnIncomingEventFailed};

    RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadStarted{this, &Statistics::handleOnUploadStarted};
    RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onPackagingFailed{this, &Statistics::handleOnPackagingFailed};
    RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadSuccessful{this, &Statistics::handleOnUploadSuccessful};
    RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadRejected{this, &Statistics::handleOnUploadRejected};
    RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadFailed{this, &Statistics::handleOnUploadFailed};

    RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageOpened{this, &Statistics::handleOnStorageOpened};
    RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageFailed{this, &Statistics::handleOnStorageFailed};
    RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageTrimmed{this, &Statistics::handleOnStorageTrimmed};
    RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageRecordsDropped{this, &Statistics::handleOnStorageRecordsDropped};
};


} ARIASDK_NS_END
