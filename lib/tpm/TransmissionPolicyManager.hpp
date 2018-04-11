// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IBandwidthController.hpp>

#include "api/IRuntimeConfig.hpp"
#include "backoff/IBackoff.hpp"
#include "pal/PAL.hpp"

#include "system/Contexts.hpp"
#include "system/Route.hpp"
#include "system/ITelemetrySystem.hpp"

#include "DeviceStateHandler.hpp"
#include <set>

namespace ARIASDK_NS_BEGIN {

class TransmissionPolicyManager
{
  public:
    TransmissionPolicyManager(ITelemetrySystem& system, IBandwidthController* bandwidthController);
    virtual ~TransmissionPolicyManager();
    virtual void scheduleUpload(int delayInMs, EventLatency latency, bool force = false);
    virtual bool isUploadInProgress() { return m_uploadInProgress; }

  protected:
    ARIASDK_LOG_DECL_COMPONENT_CLASS();
    void checkBackoffConfigUpdate();
    
    void uploadAsync(EventLatency priority);
    void finishUpload(EventsUploadContextPtr ctx, int nextUploadInMs);

    bool handleStart();
    bool handleStopOrPause();
    void handleFinishAllUploads();

    void handleEventArrived(IncomingEventContextPtr const& event);

    void handleNothingToUpload(EventsUploadContextPtr const& ctx);
    void handlePackagingFailed(EventsUploadContextPtr const& ctx);
    void handleEventsUploadSuccessful(EventsUploadContextPtr const& ctx);
    void handleEventsUploadRejected(EventsUploadContextPtr const& ctx);
    void handleEventsUploadFailed(EventsUploadContextPtr const& ctx);
    void handleEventsUploadAborted(EventsUploadContextPtr const& ctx);

    EventLatency calculateNewPriority();

  protected:
    std::mutex                       m_lock;

    ITelemetrySystem&                m_system;
    IRuntimeConfig&                  m_config;
    IBandwidthController*            m_bandwidthController;

    std::string                      m_backoffConfig;           // TODO: [MG] - move to config
    std::unique_ptr<IBackoff>        m_backoff;
    DeviceStateHandler               m_deviceStateHandler;

    bool                             m_isPaused;
    bool                             m_isUploadScheduled;
    bool                             m_finishing;
    PAL::DeferredCallbackHandle      m_scheduledUpload;

    std::set<EventsUploadContextPtr> m_activeUploads;
    int                              m_timerdelay;
    bool                             m_uploadInProgress;
    EventLatency                     m_runningLatency;
    std::vector<int>                 m_timers;

  public:
    RoutePassThrough<TransmissionPolicyManager>                          start{this, &TransmissionPolicyManager::handleStart};
    RoutePassThrough<TransmissionPolicyManager>                          pause{this, &TransmissionPolicyManager::handleStopOrPause};
    RoutePassThrough<TransmissionPolicyManager>                          stop{this, &TransmissionPolicyManager::handleStopOrPause};
    RouteSink<TransmissionPolicyManager>                                 finishAllUploads{this, &TransmissionPolicyManager::handleFinishAllUploads};
    RouteSource<>                                                        allUploadsFinished;

    RouteSink<TransmissionPolicyManager, IncomingEventContextPtr const&> eventArrived{this, &TransmissionPolicyManager::handleEventArrived};

    RouteSource<EventsUploadContextPtr const&>                           initiateUpload;
    RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  nothingToUpload{this, &TransmissionPolicyManager::handleNothingToUpload};
    RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  packagingFailed{this, &TransmissionPolicyManager::handlePackagingFailed};
    RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadSuccessful{this, &TransmissionPolicyManager::handleEventsUploadSuccessful};
    RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadRejected{this, &TransmissionPolicyManager::handleEventsUploadRejected};
    RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadFailed{this, &TransmissionPolicyManager::handleEventsUploadFailed};
    RouteSink<TransmissionPolicyManager, EventsUploadContextPtr const&>  eventsUploadAborted{this, &TransmissionPolicyManager::handleEventsUploadAborted};
};


} ARIASDK_NS_END
