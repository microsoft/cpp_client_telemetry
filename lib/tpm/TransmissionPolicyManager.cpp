// Copyright (c) Microsoft. All rights reserved.

#include "TransmissionPolicyManager.hpp"
#include "TransmitProfiles.hpp"
#include "utils/Utils.hpp"
//#include "api/CommonLogManagerInternal.hpp"

namespace ARIASDK_NS_BEGIN {

int const DEFAULT_DELAY_SEND_HTTP = 2 * 1000; // 2 sec

ARIASDK_LOG_INST_COMPONENT_CLASS(TransmissionPolicyManager, "EventsSDK.TPM", "Events telemetry client - TransmissionPolicyManager class");


TransmissionPolicyManager::TransmissionPolicyManager(IRuntimeConfig& runtimeConfig, IBandwidthController* bandwidthController)
    : m_lock(),
    m_runtimeConfig(runtimeConfig),
    m_bandwidthController(bandwidthController),
    m_isPaused(true),
    m_isUploadScheduled(false),
    m_finishing(false),
    m_timerdelay(DEFAULT_DELAY_SEND_HTTP),
    m_uploadInProgress(false),
    m_runningLatency(EventLatency_RealTime)
{
    m_backoffConfig = "E,3000,300000,2,1";
    m_backoff = IBackoff::createFromConfig(m_backoffConfig);
    assert(m_backoff);
    m_deviceStateHandler.Start();
}

TransmissionPolicyManager::~TransmissionPolicyManager()
{
    m_deviceStateHandler.Stop();
}

void TransmissionPolicyManager::checkBackoffConfigUpdate()
{
    std::string config = m_runtimeConfig.GetUploadRetryBackoffConfig();
    if (config != m_backoffConfig) {
        std::unique_ptr<IBackoff> backoff = IBackoff::createFromConfig(config);
        if (!backoff) {
            ARIASDK_LOG_WARNING("The new backoff configuration is invalid, continuing to use current settings");
        } else {
            m_backoff = std::move(backoff);
            m_backoffConfig = config;
        }
    }
}

void TransmissionPolicyManager::scheduleUpload(int delayInMs, EventLatency latency)
{
    ARIASDK_LOG_DETAIL("Scheduling another upload in %d msec, priority= %d", delayInMs, latency);

    if (m_isUploadScheduled)
    {
        m_scheduledUpload.cancel();
        m_isUploadScheduled = false;
    }

    m_scheduledUpload = PAL::scheduleOnWorkerThread(delayInMs, self(), &TransmissionPolicyManager::uploadAsync, latency);
    m_isUploadScheduled = true;
    m_uploadInProgress = true;
}

void TransmissionPolicyManager::uploadAsync(EventLatency latency)
{
    m_isUploadScheduled = false;
    m_runningLatency = latency;

    if (m_isPaused) {
        ARIASDK_LOG_DETAIL("Paused, not uploading anything until resumed");
        return;
    }

    if (!m_activeUploads.empty()) {
        ARIASDK_LOG_DETAIL("Busy, not uploading anything else until the previous upload finishes");
        return;
    }

    if (m_bandwidthController) {
        unsigned proposedBandwidthBps = m_bandwidthController->GetProposedBandwidthBps();
        unsigned minimumBandwidthBps = m_runtimeConfig.GetMinimumUploadBandwidthBps();
        if (proposedBandwidthBps >= minimumBandwidthBps) {
            ARIASDK_LOG_DETAIL("Bandwidth controller proposed sufficient bandwidth %u bytes/sec (minimum accepted is %u)",
                proposedBandwidthBps, minimumBandwidthBps);
        } else {
            unsigned delayMs = 1000;
            ARIASDK_LOG_INFO("Bandwidth controller proposed bandwidth %u bytes/sec but minimum accepted is %u, will retry %u ms later",
                proposedBandwidthBps, minimumBandwidthBps, delayMs);
            scheduleUpload(delayMs, latency);
            return;
        }
    }

    EventsUploadContextPtr ctx = EventsUploadContext::create();
    ctx->requestedMinLatency = m_runningLatency;// EventLatency_Low;
    m_activeUploads.insert(ctx);
    initiateUpload(ctx);
}

void TransmissionPolicyManager::finishUpload(EventsUploadContextPtr const& ctx, int nextUploadInMs)
{
    ARIASDK_LOG_DETAIL("HTTP upload finished for ctx=%p", ctx);
    if (m_activeUploads.find(ctx) != m_activeUploads.cend())
    {
        ARIASDK_LOG_DETAIL("HTTP removing from active uploads ctx=%p", ctx);
        m_activeUploads.erase(ctx);
        ((EventsUploadContextPtr*)(&ctx))->reset();
    }
    else
    {
        ARIASDK_LOG_WARNING("HTTP NOT removing non-existing ctx from active uploads ctx=%p", ctx);
    }

    if (m_activeUploads.empty())
    {
        m_uploadInProgress = false;
    }

    if (m_finishing) {
        if (m_activeUploads.empty()) {
            allUploadsFinished();
        }
        return;
    }

    if (nextUploadInMs >= 0 && m_activeUploads.empty())
    {
        if (m_isUploadScheduled) {
            m_scheduledUpload.cancel();
            m_isUploadScheduled = false;
        }

        EventLatency proposed = calculateNewPriority();
        scheduleUpload(nextUploadInMs, proposed);
    }
}

bool TransmissionPolicyManager::handleStart()
{
    m_isPaused = false;
    scheduleUpload(0, EventLatency_Normal);
    return true;
}

bool TransmissionPolicyManager::handleStopOrPause()
{
    m_isPaused = true;
    m_scheduledUpload.cancel();
    m_isUploadScheduled = false;
    return true;
}

void TransmissionPolicyManager::handleFinishAllUploads()
{
    if (m_activeUploads.empty()) {
        ARIASDK_LOG_DETAIL("There are no active uploads");
        allUploadsFinished();
        return;
    }

    ARIASDK_LOG_DETAIL("Waiting for %u outstanding upload(s)...",
        static_cast<unsigned>(m_activeUploads.size()));
    m_finishing = true;
}

void TransmissionPolicyManager::handleEventArrived(IncomingEventContextPtr const& event)
{
    if (m_isPaused) {
        return;
    }

    if (event->record.latency > EventLatency_RealTime) {
        EventsUploadContextPtr ctx = EventsUploadContext::create();
        ctx->requestedMinLatency = event->record.latency;
        m_activeUploads.insert(ctx);
        initiateUpload(ctx);
    } 
    else if (!m_isUploadScheduled || TransmitProfiles::isTimerUpdateRequired())
    {
        if (m_isUploadScheduled)
        {
            m_scheduledUpload.cancel();
            m_isUploadScheduled = false;
        }

        if (TransmitProfiles::isTimerUpdateRequired())
        {
            TransmitProfiles::getTimers(m_timers);
            if (m_timers.size() > 2)
            {
                m_timerdelay = m_timers[2];
            }
        }
       
        EventLatency proposed = calculateNewPriority();
           
        scheduleUpload(m_timerdelay, proposed);
    }
}

EventLatency TransmissionPolicyManager::calculateNewPriority()
{
    EventLatency proposed = m_runningLatency;
    if (m_timers.size() > 2)
    {
        if (m_timers[0] == m_timers[2])
        {
            proposed = EventLatency_Normal;
        }
        else
        {
            if (m_runningLatency == EventLatency_RealTime)
            {
                proposed = EventLatency_Normal;
            }
            else if (m_runningLatency == EventLatency_Normal)
            {
                proposed = EventLatency_RealTime;
            }
        }
    }
    else
    {
        proposed = EventLatency_Normal;
    }

    return proposed;
}

void TransmissionPolicyManager::handleNothingToUpload(EventsUploadContextPtr const& ctx)
{
    ARIASDK_LOG_DETAIL("No stored events to send at the moment");	
    m_backoff->reset();
    if (ctx->requestedMinLatency == EventLatency_Normal)
    {
        finishUpload(ctx, -1);
    }
    else
    {
        finishUpload(ctx, 0);
    }
}

void TransmissionPolicyManager::handlePackagingFailed(EventsUploadContextPtr const& ctx)
{
    finishUpload(ctx, m_timerdelay);
}

void TransmissionPolicyManager::handleEventsUploadSuccessful(EventsUploadContextPtr const& ctx)
{
    m_backoff->reset();
    finishUpload(ctx, 0);
}

void TransmissionPolicyManager::handleEventsUploadRejected(EventsUploadContextPtr const& ctx)
{
    checkBackoffConfigUpdate();
    int delayMs = m_backoff->getValue();
    m_backoff->increase();

    finishUpload(ctx, delayMs);
}

void TransmissionPolicyManager::handleEventsUploadFailed(EventsUploadContextPtr const& ctx)
{
    checkBackoffConfigUpdate();
    int delayMs = m_backoff->getValue();
    m_backoff->increase();

    finishUpload(ctx, delayMs);
}

void TransmissionPolicyManager::handleEventsUploadAborted(EventsUploadContextPtr const& ctx)
{
    finishUpload(ctx, -1);
}


} ARIASDK_NS_END
