// Copyright (c) Microsoft. All rights reserved.

#include "TransmissionPolicyManager.hpp"
#include "TransmitProfiles.hpp"
#include "utils/Common.hpp"
#include "api\LogManager.hpp"

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
	m_timerdelay(DEFAULT_DELAY_SEND_HTTP)
{
    m_backoffConfig = "E,3000,300000,2,1";
    m_backoff = IBackoff::createFromConfig(m_backoffConfig);
    assert(m_backoff);
	TransmitProfiles::setDefaultProfile(TransmitProfile::TransmitProfile_RealTime);
	TransmitProfiles::updateStates(NetworkCost::NetworkCost_Unmetered, PowerSource::PowerSource_Charging);
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

void TransmissionPolicyManager::scheduleUpload(int delayInMs)
{
    ARIASDK_LOG_DETAIL("Scheduling another upload in %d msec", delayInMs);

    assert(!m_isUploadScheduled);
    m_scheduledUpload = PAL::scheduleOnWorkerThread(delayInMs, self(), &TransmissionPolicyManager::uploadAsync);
    m_isUploadScheduled = true;
}

void TransmissionPolicyManager::uploadAsync()
{
    m_isUploadScheduled = false;

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
            scheduleUpload(delayMs);
            return;
        }
    }

    EventsUploadContextPtr ctx = EventsUploadContext::create();
    ctx->requestedMinPriority = EventPriority_Low;
    m_activeUploads.insert(ctx);
    initiateUpload(ctx);
	LogManager::DispatchEvent(DebugEventType::EVT_SENT);
}

void TransmissionPolicyManager::finishUpload(EventsUploadContextPtr const& ctx, int nextUploadInMs)
{
    m_activeUploads.erase(ctx);

    if (m_finishing) {
        if (m_activeUploads.empty()) {
            allUploadsFinished();
        }
        return;
    }

    if (nextUploadInMs >= 0 && m_activeUploads.empty()) {
        if (m_isUploadScheduled) {
            m_scheduledUpload.cancel();
            m_isUploadScheduled = false;
        }
        scheduleUpload(nextUploadInMs);
    }
}

bool TransmissionPolicyManager::handleStart()
{
    m_isPaused = false;
    scheduleUpload(0);
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
	else if (!m_isUploadScheduled) 
	{
		std::vector<int> timers;
		TransmitProfiles::getTimers(timers);
		if(timers.size() > 2)
		{
			m_timerdelay = timers[2];
		}
		scheduleUpload(m_timerdelay);
    }
}

void TransmissionPolicyManager::handleNothingToUpload(EventsUploadContextPtr const& ctx)
{
    ARIASDK_LOG_DETAIL("No stored events to send at the moment");
    m_backoff->reset();
    finishUpload(ctx, -1);
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
