// Copyright (c) Microsoft. All rights reserved.

#include "TransmissionPolicyManager.hpp"
#include "TransmitProfiles.hpp"
#include "utils/Utils.hpp"
#include "LogManager.hpp"

namespace ARIASDK_NS_BEGIN {

int const DEFAULT_DELAY_SEND_HTTP = 2 * 1000; // 2 sec

ARIASDK_LOG_INST_COMPONENT_CLASS(TransmissionPolicyManager, "AriaSDK.TPM", "Aria telemetry client - TransmissionPolicyManager class");


TransmissionPolicyManager::TransmissionPolicyManager(IRuntimeConfig& runtimeConfig, IBandwidthController* bandwidthController)
    : m_lock(),
    m_runtimeConfig(runtimeConfig),
    m_bandwidthController(bandwidthController),
    m_isPaused(true),
    m_isUploadScheduled(false),
    m_finishing(false),
    m_timerdelay(DEFAULT_DELAY_SEND_HTTP),
    m_uploadInProgress(false),
    m_runningPriority(EventPriority_High)
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

void TransmissionPolicyManager::scheduleUpload(int delayInMs, EventPriority priority)
{
    ARIASDK_LOG_DETAIL("Scheduling another upload in %d msec, priority= %d", delayInMs, priority);

    if (m_isUploadScheduled)
    {
        m_scheduledUpload.cancel();
        m_isUploadScheduled = false;
    }

    m_scheduledUpload = PAL::scheduleOnWorkerThread(delayInMs, self(), &TransmissionPolicyManager::uploadAsync, priority);
    m_isUploadScheduled = true;
    m_uploadInProgress = true;
}

void TransmissionPolicyManager::uploadAsync(EventPriority priority)
{
    m_isUploadScheduled = false;
    m_runningPriority = priority;

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
            scheduleUpload(delayMs, priority);
            return;
        }
    }

    EventsUploadContextPtr ctx = EventsUploadContext::create();
    ctx->requestedMinPriority = m_runningPriority;// EventPriority_Low;
    m_activeUploads.insert(ctx);
    initiateUpload(ctx);
}

void TransmissionPolicyManager::finishUpload(EventsUploadContextPtr const& ctx, int nextUploadInMs)
{
    m_activeUploads.erase(ctx);
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

        EventPriority proposed = calculateNewPriority();
        scheduleUpload(nextUploadInMs, proposed);
    }
}

bool TransmissionPolicyManager::handleStart()
{
    m_isPaused = false;
    scheduleUpload(0, EventPriority_Low);
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

    if (event->record.priority >= EventPriority_Immediate) {
        EventsUploadContextPtr ctx = EventsUploadContext::create();
        ctx->requestedMinPriority = event->record.priority;
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
       
        EventPriority proposed = calculateNewPriority();
           
        scheduleUpload(m_timerdelay, proposed);
    }
}

EventPriority TransmissionPolicyManager::calculateNewPriority()
{
    EventPriority proposed = m_runningPriority;
    if (m_timers.size() > 2)
    {
        if (m_timers[0] == m_timers[2])
        {
            proposed = EventPriority_Low;
        }
        else
        {
            bool isLowPriorityEnabled = false;
            if (m_timers[0] > -1)
            {
                isLowPriorityEnabled = true;
            }
            bool isNormalPriorityEnabled = false;
            if (m_timers[1] > -1)
            {
                isNormalPriorityEnabled = true;
            }
            bool isHighPriorityEnabled = false;
            if (m_timers[2] > -1)
            {
                isHighPriorityEnabled = true;
            }

            if (m_runningPriority == EventPriority_High)
            {
                if (isNormalPriorityEnabled)
                {
                    proposed = EventPriority_Normal;
                }
            }
            else if (m_runningPriority == EventPriority_Normal)
            {
                if (isLowPriorityEnabled)
                {
                    proposed = EventPriority_Low;
                }
                else
                {
                    proposed = EventPriority_High;
                }
            }
            else if (m_runningPriority == EventPriority_Low)
            {
                proposed = EventPriority_High;
            }
        }
    }
    else
    {
        proposed = EventPriority_Low;
    }

    return proposed;
}

void TransmissionPolicyManager::handleNothingToUpload(EventsUploadContextPtr const& ctx)
{
    ARIASDK_LOG_DETAIL("No stored events to send at the moment");	
    m_backoff->reset();
    if (ctx->requestedMinPriority == EventPriority::EventPriority_Low)
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
