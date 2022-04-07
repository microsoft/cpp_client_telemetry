//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include "pal/PAL.hpp"

#include "system/ITelemetrySystem.hpp"

#include "decorators/BaseDecorator.hpp"
#include "decorators/SemanticContextDecorator.hpp"

#include "MetaStats.hpp"
#include "DebugEvents.hpp"
#include "pal/TaskDispatcher.hpp"

#include "system/Route.hpp"
#include "system/Contexts.hpp"

#include <mutex>
#include <atomic>

namespace MAT_NS_BEGIN {

    class ITelemetrySystem;
    class DebugEventListener;

    class Statistics : public DebugEventListener {

    public:
        Statistics(ITelemetrySystem& telemetrySystem, ITaskDispatcher& taskDispatcher);
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
        std::mutex                  m_metaStats_mtx;
        MetaStats                   m_metaStats;
        ITelemetrySystem&           m_iTelemetrySystem;
        ITaskDispatcher&            m_taskDispatcher;
        IRuntimeConfig&             m_config;
        ILogManager&                m_logManager;

        // Both decorators are associated with m_logManager
        BaseDecorator               m_baseDecorator;
        SemanticContextDecorator    m_semanticContextDecorator;

        PAL::DeferredCallbackHandle m_scheduledSend;
        std::atomic<bool>           m_isScheduled;
        bool                        m_isStarted;

        std::int64_t                m_statEventSentTime;

    public:

        RouteSource<>                                                   onStartupDone;
        RoutePassThrough<Statistics>                                    onStart{ this, &Statistics::handleOnStart };
        RouteSource<>                                                   onShutdownDone;
        RoutePassThrough<Statistics>                                    onStop{ this, &Statistics::handleOnStop };

#if 1   // TODO: [MG] - verify this codepath
        RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventAccepted{ this, &Statistics::handleOnIncomingEventAccepted };
        RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventFailed{ this, &Statistics::handleOnIncomingEventFailed };
#else
        bool dummy_IncomingEventContextPtr(IncomingEventContextPtr const& ctx)
        {
            UNREFERENCED_PARAMETER(ctx);
            return true;
        }

        RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventAccepted{ this, &Statistics::dummy_IncomingEventContextPtr };
        RoutePassThrough<Statistics, IncomingEventContextPtr const&>    onIncomingEventFailed{ this, &Statistics::dummy_IncomingEventContextPtr };
#endif

#if 1   // TODO: [MG] - verify this codepath
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadStarted{ this, &Statistics::handleOnUploadStarted };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onPackagingFailed{ this, &Statistics::handleOnPackagingFailed };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadSuccessful{ this, &Statistics::handleOnUploadSuccessful };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadRejected{ this, &Statistics::handleOnUploadRejected };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadFailed{ this, &Statistics::handleOnUploadFailed };
#else
        bool dummy_EventsUploadContextPtr(EventsUploadContextPtr const& ctx)
        {
            UNREFERENCED_PARAMETER(ctx);
            return true;
        }

        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadStarted{ this, &Statistics::dummy_EventsUploadContextPtr };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onPackagingFailed{ this, &Statistics::dummy_EventsUploadContextPtr };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadSuccessful{ this, &Statistics::dummy_EventsUploadContextPtr };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadRejected{ this, &Statistics::dummy_EventsUploadContextPtr };
        RoutePassThrough<Statistics, EventsUploadContextPtr const&>     onUploadFailed{ this, &Statistics::dummy_EventsUploadContextPtr };
#endif

        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageOpened{ this, &Statistics::handleOnStorageOpened };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageFailed{ this, &Statistics::handleOnStorageFailed };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageTrimmed{ this, &Statistics::handleOnStorageTrimmed };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageRecordsDropped{ this, &Statistics::handleOnStorageRecordsDropped };
        RoutePassThrough<Statistics, StorageNotificationContext const*> onStorageRecordsRejected{ this, &Statistics::handleOnStorageRecordsRejected };

        virtual void OnDebugEvent(DebugEvent &evt) override;

    };


} MAT_NS_END

#endif

