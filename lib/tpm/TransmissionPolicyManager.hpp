// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IBandwidthController.hpp"

#include "api/IRuntimeConfig.hpp"
#include "backoff/IBackoff.hpp"
#include "pal/PAL.hpp"

#include "system/Contexts.hpp"
#include "system/Route.hpp"
#include "system/ITelemetrySystem.hpp"

#include "DeviceStateHandler.hpp"

#include <set>
#include <atomic>

namespace ARIASDK_NS_BEGIN {

    class TransmissionPolicyManager
    {

    public:
        TransmissionPolicyManager(ITelemetrySystem& system, IBandwidthController* bandwidthController);
        virtual ~TransmissionPolicyManager();
        virtual void scheduleUpload(int delayInMs, EventLatency latency, bool force = false);

    protected:
        MATSDK_LOG_DECL_COMPONENT_CLASS();
        void checkBackoffConfigUpdate();

        void uploadAsync(EventLatency priority);
        void finishUpload(EventsUploadContextPtr ctx, int nextUploadInMs);

        bool handleStart();
        bool handlePause();
        bool handleStop();
        void handleFinishAllUploads();

        void handleEventArrived(IncomingEventContextPtr const& event);

        void handleNothingToUpload(EventsUploadContextPtr const& ctx);
        void handlePackagingFailed(EventsUploadContextPtr const& ctx);
        void handleEventsUploadSuccessful(EventsUploadContextPtr const& ctx);
        void handleEventsUploadRejected(EventsUploadContextPtr const& ctx);
        void handleEventsUploadFailed(EventsUploadContextPtr const& ctx);
        void handleEventsUploadAborted(EventsUploadContextPtr const& ctx);

        EventLatency calculateNewPriority();

        std::mutex                       m_lock;

        ITelemetrySystem&                m_system;
        IRuntimeConfig&                  m_config;
        IBandwidthController*            m_bandwidthController;

        std::string                      m_backoffConfig;           // TODO: [MG] - move to config
        std::unique_ptr<IBackoff>        m_backoff;
        DeviceStateHandler               m_deviceStateHandler;

        std::atomic<bool>                m_isPaused;
        std::atomic<bool>                m_isUploadScheduled;
        uint64_t                         m_scheduledUploadTime;
        PAL::DeferredCallbackHandle      m_scheduledUpload;

        std::mutex                       m_activeUploads_lock;
        std::set<EventsUploadContextPtr> m_activeUploads;
        
        /// <summary>
        /// Thread-safe method to add the upload to active uploads.
        /// </summary>
        /// <param name="ctx">The CTX.</param>
        void addUpload(EventsUploadContextPtr ctx)
        {
            LOCKGUARD(m_activeUploads_lock);
            m_activeUploads.insert(ctx);
        }
        
        /// <summary>
        /// Thread-safe method to remove the upload from active uploads.
        /// </summary>
        /// <param name="ctx">The CTX.</param>
        /// <returns></returns>
        bool removeUpload(EventsUploadContextPtr ctx)
        {
            LOCKGUARD(m_activeUploads_lock);
            auto it = m_activeUploads.find(ctx);
            if (it != m_activeUploads.cend())
            {
                LOG_TRACE("HTTP removing from active uploads ctx=%p", ctx);
                m_activeUploads.erase(it);
                delete ctx;
                return true;
            }
            return false;
        }
        
        /// <summary>
        /// Cancel pending upload task and stop scheduling further uploads.
        /// </summary>
        void pauseAllUploads()
        {
            m_isPaused = true;
            cancelUploadTask();
        }
        
        /// <summary>
        /// Cancels pending upload task.
        /// </summary>
        bool cancelUploadTask()
        {
            bool result = m_scheduledUpload.cancel();
            m_isUploadScheduled.exchange(false);
            return result;
        }
        
        /// <summary>
        /// Calculate the number of pending upload contexts.
        /// </summary>
        /// <returns></returns>
        size_t uploadCount()
        {
            LOCKGUARD(m_activeUploads_lock);
            return m_activeUploads.size();
        }

        int                              m_timerdelay;
        EventLatency                     m_runningLatency;
        std::vector<int>                 m_timers;

    public:
        RoutePassThrough<TransmissionPolicyManager>                          start{ this, &TransmissionPolicyManager::handleStart };
        RoutePassThrough<TransmissionPolicyManager>                          pause{ this, &TransmissionPolicyManager::handlePause };
        RoutePassThrough<TransmissionPolicyManager>                          stop{ this, &TransmissionPolicyManager::handleStop };
        RouteSink<TransmissionPolicyManager>                                 finishAllUploads{ this, &TransmissionPolicyManager::handleFinishAllUploads };
        RouteSource<>                                                        allUploadsFinished;

        RouteSink<TransmissionPolicyManager, IncomingEventContextPtr const&> eventArrived{ this, &TransmissionPolicyManager::handleEventArrived };

        RouteSource<EventsUploadContextPtr const&>                           initiateUpload;
        RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  nothingToUpload{ this, &TransmissionPolicyManager::handleNothingToUpload };
        RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  packagingFailed{ this, &TransmissionPolicyManager::handlePackagingFailed };
        RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadSuccessful{ this, &TransmissionPolicyManager::handleEventsUploadSuccessful };
        RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadRejected{ this, &TransmissionPolicyManager::handleEventsUploadRejected };
        RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadFailed{ this, &TransmissionPolicyManager::handleEventsUploadFailed };
        RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadAborted{ this, &TransmissionPolicyManager::handleEventsUploadAborted };

        virtual bool isUploadInProgress()
        {
            // unfinished uploads that haven't processed callbacks or pending upload task
            return (uploadCount() > 0) || m_isUploadScheduled;
        }

        virtual bool isPaused()
        {
            return m_isPaused;
        }

    };


} ARIASDK_NS_END
