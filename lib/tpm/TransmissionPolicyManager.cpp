// Copyright (c) Microsoft. All rights reserved.

#include "TransmissionPolicyManager.hpp"
#include "TransmitProfiles.hpp"
#include "utils/Utils.hpp"

#include <limits>

#define ABS64(a,b)    ((a>b)?(a-b):(b-a))

namespace ARIASDK_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(TransmissionPolicyManager, "EventsSDK.TPM", "Events telemetry client - TransmissionPolicyManager class");

    TransmissionPolicyManager::TransmissionPolicyManager(ITelemetrySystem& system, ITaskDispatcher& taskDispatcher, IBandwidthController* bandwidthController) :
        m_system(system),
        m_taskDispatcher(taskDispatcher),
        m_config(m_system.getConfig()),
        m_bandwidthController(bandwidthController)
    {
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
        LOCKGUARD(m_backoffMutex);
        std::string config = m_config.GetUploadRetryBackoffConfig();
        if (config != m_backoffConfig)
        {
            std::unique_ptr<IBackoff> backoff = IBackoff::createFromConfig(config);
            if (!backoff)
            {
                LOG_WARN("The new backoff configuration is invalid, continuing to use current settings");
            }
            else
            {
                m_backoff = std::move(backoff);
                m_backoffConfig = config;
            }
        }
    }

    void TransmissionPolicyManager::resetBackoff()
    {
        LOCKGUARD(m_backoffMutex);
        if (m_backoff)
            m_backoff->reset();
    }

    int TransmissionPolicyManager::increaseBackoff()
    {
        int delayMs = 0;
        LOCKGUARD(m_backoffMutex);
        checkBackoffConfigUpdate();
        if (m_backoff)
        {
            delayMs = m_backoff->getValue();
            m_backoff->increase();
        }
        return delayMs;
    }

    // TODO: consider changing int delayInMs to std::chrono::duration<> in millis.
    // If delayInMs is negative, do not schedule.
    void TransmissionPolicyManager::scheduleUpload(int delayInMs, EventLatency latency, bool force)
    {
        LOCKGUARD(m_scheduledUploadMutex);
        if (delayInMs < 0 || m_timerdelay < 0) {
            return; // profile: no upload allowed
        }
        if (m_scheduledUploadAborted)
        {
            return;
        }
        if (delayInMs < 0 || m_timerdelay < 0) {
            LOG_TRACE("Negative delayInMs or m_timerdelay, no upload");
            return; // transmission prohibited by profile
        }
        if (uploadCount() >= static_cast<uint32_t>(m_config[CFG_INT_MAX_PENDING_REQ]) )
        {
            LOG_TRACE("Maximum number of HTTP requests reached");
            return;
        }

        if (m_isPaused)
        {
            LOG_TRACE("Paused, not uploading anything until resumed");
            return;
        }

        updateTimersIfNecessary();
        if (m_timers[0] < 0) {
            latency = std::max(latency, EventLatency_RealTime); // low priority disabled by profile
        }

        if ((!force)&&(m_isUploadScheduled))
        {
            if (m_runningLatency > latency)
            {
                // Allow lower priority (normal) events to get thru in the next batch
                m_runningLatency = latency;
            }
            auto now = PAL::getMonotonicTimeMs();
            auto delta = ABS64(m_scheduledUploadTime, now);
            if (delta <= static_cast<uint64_t>(delayInMs))
            {
                // Don't need to cancel and reschedule if it's about to happen now anyways.
                // m_isUploadScheduled check does not have to be strictly atomic because
                // the completion of upload will schedule more uploads as-needed, we only
                // want to avoid the unnecessary wasteful rescheduling.
                LOG_TRACE("WAIT  upload %d ms for lat=%d", delta, m_runningLatency);
                return;
            }
        }

        // Cancel upload if already scheduled.
        if (force || delayInMs == 0)
        {
            if (!cancelUploadTask())
            {
                LOG_TRACE("Upload either hasn't been scheduled or already done.");
            }
        }

        // Schedule new upload
        if (!m_isUploadScheduled.exchange(true))
        {
            m_scheduledUploadTime = PAL::getMonotonicTimeMs() + delayInMs;
            m_runningLatency = latency;
            LOG_TRACE("SCHED upload %d ms for lat=%d", delayInMs, m_runningLatency);
            m_scheduledUpload = PAL::scheduleTask(&m_taskDispatcher, delayInMs, this, &TransmissionPolicyManager::uploadAsync, latency);
        }
    }

    void TransmissionPolicyManager::uploadAsync(EventLatency latency)
    {
        m_runningLatency = latency;
        m_scheduledUploadTime = std::numeric_limits<uint64_t>::max();

        {
            LOCKGUARD(m_scheduledUploadMutex);
            m_isUploadScheduled = false;  // Allow to schedule another uploadAsync
            if ((m_isPaused) || (m_scheduledUploadAborted))
            {
                LOG_TRACE("Paused or upload aborted: cancel pending upload task.");
                cancelUploadTask();  // If there is a pending upload task, kill it
                return;
            }
        }

#ifdef ENABLE_BW_CONTROLLER   /* Bandwidth controller is not currently supported */
        if (m_bandwidthController) {
            unsigned proposedBandwidthBps = m_bandwidthController->GetProposedBandwidthBps();
            unsigned minimumBandwidthBps = m_config.GetMinimumUploadBandwidthBps();
            if (proposedBandwidthBps >= minimumBandwidthBps) {
                LOG_TRACE("Bandwidth controller proposed sufficient bandwidth %u bytes/sec (minimum accepted is %u)",
                    proposedBandwidthBps, minimumBandwidthBps);
            }
            else {
                unsigned delayMs = 1000;
                LOG_INFO("Bandwidth controller proposed bandwidth %u bytes/sec but minimum accepted is %u, will retry %u ms later",
                    proposedBandwidthBps, minimumBandwidthBps, delayMs);
                scheduleUpload(delayMs, latency); // reschedule uploadAsync to run again 1000 ms later
                return;
            }
        }
#endif

        auto ctx = std::make_shared<EventsUploadContext>();
        ctx->requestedMinLatency = m_runningLatency;
        addUpload(ctx);
        initiateUpload(ctx);
    }

    void TransmissionPolicyManager::finishUpload(EventsUploadContextPtr const& ctx, int nextUploadInMs)
    {
        LOG_TRACE("HTTP upload finished for ctx=%p", ctx.get());
        if (!removeUpload(ctx))
        {
            assert(false);
            LOG_WARN("HTTP NOT removing non-existing ctx from active uploads ctx=%p", ctx.get());
        }

        // Rescheduling upload
        if (nextUploadInMs >= 0)
        {
            LOG_TRACE("Scheduling upload in %d ms", nextUploadInMs);
            EventLatency proposed = calculateNewPriority();
            scheduleUpload(nextUploadInMs, proposed); // reschedule uploadAsync again
        }
    }

    bool TransmissionPolicyManager::updateTimersIfNecessary()
    {
        bool needsUpdate = TransmitProfiles::isTimerUpdateRequired();
        if (needsUpdate)
        {
            TransmitProfiles::getTimers(m_timers);
        }
        return needsUpdate;
    }

    bool TransmissionPolicyManager::handleStart()
    {
        m_isPaused = false;
        // TODO: [MG] - this implies that start would force the immediate upload, but
        // some customers require to be able to start in a paused (no telemetry) state.
        // We may avoid the issue if we schedule the first upload to happen 1 second
        // after start
        scheduleUpload(1000, calculateNewPriority());
        return true;
    }

    /**
     * Stop scheduling upload task, but don't wait for in-progress uploads to stop.
     * Once paused we let all in-progress uploads to run to completion.
     */
    bool TransmissionPolicyManager::handlePause()
    {
        pauseAllUploads();
        return true;
    }

    /**
     * Wait for all pending uploads to complete. This handler is invoked from
     * TelemetrySystem::onStop after HCM has attempted to cancel all pending
     * requests via hcm.cancelAllRequests. Since we don't directly couple TPM
     * with HCM implementation, the system asks HCM to stop and TPM must wait
     * for all callbacks to come.
     */
    bool TransmissionPolicyManager::handleStop()
    {
        {
            LOCKGUARD(m_scheduledUploadMutex);
            // Prevent execution of all upload tasks
            m_scheduledUploadAborted = true;
            // Make sure we wait for completion of the upload scheduling task that may be running
            cancelUploadTask();
        }

        // Make sure we wait for all active upload callbacks to finish
        while (uploadCount() > 0)
        {
            std::this_thread::yield();
        }
        allUploadsFinished();
        return true;
    }

    // Called from finishAllUploads
    void TransmissionPolicyManager::handleFinishAllUploads()
    {
        // TODO: This pause appears to server no practical purpose? Issue 387
        pauseAllUploads();
        allUploadsFinished();   // calls stats.onStop >> this->flushTaskDispatcher;
    }

    void TransmissionPolicyManager::handleEventArrived(IncomingEventContextPtr const& event)
    {
        if (m_isPaused) {
            return;
        }
        bool forceTimerRestart = false;

        /* This logic needs to be revised: one event in a dedicated HTTP post is wasteful! */
        // Initiate upload right away
        if (event->record.latency > EventLatency_RealTime) {
            auto ctx = std::make_shared<EventsUploadContext>();
            ctx->requestedMinLatency = event->record.latency;
            addUpload(ctx);
            initiateUpload(ctx);
            return;
        }

        // Schedule async upload if not scheduled yet
        if (!m_isUploadScheduled || TransmitProfiles::isTimerUpdateRequired())
        {
            if (updateTimersIfNecessary())
            {
                m_timerdelay = m_timers[1];
                forceTimerRestart = true;
            }
            EventLatency proposed = calculateNewPriority();
            if (m_timerdelay >= 0) {
                scheduleUpload(m_timerdelay, proposed, forceTimerRestart);
            }
        }
    }

    // We do only Normal if too few values or timers[0] == timers[2]
    // We do only RealTime if timers[0] < 0 (do not transmit)
    // We alternate RealTime and Normal otherwise (timers differ)
    EventLatency TransmissionPolicyManager::calculateNewPriority()
    {
        updateTimersIfNecessary();

        if (m_timers[0] == m_timers[1])
        {
            return EventLatency_Normal;
        }

        if (m_timers[0] < 0)
        {
            return EventLatency_RealTime;
        }

        if (m_runningLatency == EventLatency_RealTime)
        {
            return EventLatency_Normal;
        }

        return EventLatency_RealTime;
    }

    void TransmissionPolicyManager::handleNothingToUpload(EventsUploadContextPtr const& ctx)
    {
        LOG_TRACE("No stored events to send at the moment");
        resetBackoff();
        if (ctx->requestedMinLatency == EventLatency_Normal)
        {
            finishUpload(ctx, -1);
        }
        else
        {
            finishUpload(ctx, m_timerdelay);
        }
    }

    void TransmissionPolicyManager::handlePackagingFailed(EventsUploadContextPtr const& ctx)
    {
        finishUpload(ctx, m_timerdelay);
    }

    void TransmissionPolicyManager::handleEventsUploadSuccessful(EventsUploadContextPtr const& ctx)
    {
        resetBackoff();
        finishUpload(ctx, 0);
    }

    void TransmissionPolicyManager::handleEventsUploadRejected(EventsUploadContextPtr const& ctx)
    {
        finishUpload(ctx, increaseBackoff());
    }

    void TransmissionPolicyManager::handleEventsUploadFailed(EventsUploadContextPtr const& ctx)
    {
        finishUpload(ctx, increaseBackoff());
    }

    void TransmissionPolicyManager::handleEventsUploadAborted(EventsUploadContextPtr const& ctx)
    {
        finishUpload(ctx, -1);
    }

    void TransmissionPolicyManager::addUpload(EventsUploadContextPtr const& ctx)
    {
        LOCKGUARD(m_activeUploads_lock);
        m_activeUploads.insert(ctx);
    }

    bool TransmissionPolicyManager::removeUpload(EventsUploadContextPtr const& ctx)
    {
        LOCKGUARD(m_activeUploads_lock);
        auto it = m_activeUploads.find(ctx);
        if (it != m_activeUploads.cend())
        {
            LOG_TRACE("HTTP removing from active uploads ctx=%p", ctx.get());
            m_activeUploads.erase(it);
            return true;
        }
        return false;
    }

    void TransmissionPolicyManager::pauseAllUploads()
    {
        m_isPaused = true;
        cancelUploadTask();
    }

    std::chrono::milliseconds TransmissionPolicyManager::getCancelWaitTime() noexcept
    {
       return (m_scheduledUploadAborted) ? DefaultTaskCancelTime : std::chrono::milliseconds {};
    }

    bool TransmissionPolicyManager::cancelUploadTask()
    {
        bool result = m_scheduledUpload.Cancel(getCancelWaitTime().count());

        // TODO: There is a potential for upload tasks to not be canceled, especially if they aren't waited for.
        //       We either need a stronger guarantee here (could impact SDK performance), or a mechanism to
        //       ensure those tasks are canceled when the log manager is destroyed. Issue 388
        if (result)
        {
            m_isUploadScheduled.exchange(false);
        }
        return result;
    }

    size_t TransmissionPolicyManager::uploadCount()
    {
        LOCKGUARD(m_activeUploads_lock);
        return m_activeUploads.size();
    }

    bool TransmissionPolicyManager::isUploadInProgress()
    {
        // unfinished uploads that haven't processed callbacks or pending upload task
        return (uploadCount() > 0) || m_isUploadScheduled;
    }

    bool TransmissionPolicyManager::isPaused()
    {
        return m_isPaused;
    }

} ARIASDK_NS_END
