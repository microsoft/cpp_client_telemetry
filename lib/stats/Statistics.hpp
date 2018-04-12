#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include "pal/PAL.hpp"

#include "system/ITelemetrySystem.hpp"

#include "MetaStats.hpp"
#include "DebugEvents.hpp"

#include "system/Route.hpp"
#include "system/Contexts.hpp"

namespace ARIASDK_NS_BEGIN {

    class ITelemetrySystem;
    class DebugEventListener;

    class Statistics : public DebugEventListener {

    public:
        Statistics(ITelemetrySystem& telemetrySystem);
        ~Statistics();

    protected:
        virtual void scheduleSend();
        void send(RollUpKind rollupKind);

        bool handleOnStart();
        bool handleOnStop();

        bool handleOnIncomingEventAccepted(IncomingEventContextPtr const& ctx);
        // bool handleOnIncomingEventRejected(DebugEvent &evt); 
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
        bool handleOnStorageRecordsRejected(StorageNotificationContext const* ctx);

    protected:
        MetaStats                   m_metaStats;
        ITelemetrySystem&           m_iTelemetrySystem;
        IRuntimeConfig&             m_config;
        ILogManager&                m_logManager;

        PAL::DeferredCallbackHandle m_scheduledSend;
        bool                        m_isScheduled;
        bool                        m_isStarted;

        std::int64_t                m_statEventSentTime;

    public:
        RouteSource<IncomingEventContextPtr const&>                     eventGenerated;

        RouteSource<>                                                   onStartupDone;
        RoutePassThrough<Statistics>                                    onStart{ this, &Statistics::handleOnStart };
        RouteSource<>                                                   onShutdownDone;
        RoutePassThrough<Statistics>                                    onStop{ this, &Statistics::handleOnStop };

        RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventAccepted{ this, &Statistics::handleOnIncomingEventAccepted };
        RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventFailed{ this, &Statistics::handleOnIncomingEventFailed };

        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadStarted{ this, &Statistics::handleOnUploadStarted };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onPackagingFailed{ this, &Statistics::handleOnPackagingFailed };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadSuccessful{ this, &Statistics::handleOnUploadSuccessful };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadRejected{ this, &Statistics::handleOnUploadRejected };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadFailed{ this, &Statistics::handleOnUploadFailed };

        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageOpened{ this, &Statistics::handleOnStorageOpened };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageFailed{ this, &Statistics::handleOnStorageFailed };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageTrimmed{ this, &Statistics::handleOnStorageTrimmed };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageRecordsDropped{ this, &Statistics::handleOnStorageRecordsDropped };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageRecordsRejected{ this, &Statistics::handleOnStorageRecordsRejected };

        virtual void OnDebugEvent(DebugEvent &evt);

    };


} ARIASDK_NS_END

#endif
