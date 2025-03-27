//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "TransmissionPolicyManager.hpp"
#include "TransmitProfiles.hpp"
#include "utils/Utils.hpp"

#include <limits>

namespace MAT_NS_BEGIN {

    class PauseGuard {
    public:
        PauseGuard() = delete;
        PauseGuard(ILogManager & logManager)
        : m_logManager(logManager)
        , m_unpaused(m_logManager.StartActivity())
        {}

        ~PauseGuard()
        {
            if (m_unpaused) {
                m_logManager.EndActivity();
            }
        }

        bool isPaused() const noexcept
        {
            return !m_unpaused;
        }
    private:
        ILogManager& m_logManager;
        bool m_unpaused;
    };

    template<typename T>
    constexpr T Abs64(const T& a, const T& b) noexcept
    {
        return (a > b) ? (a - b) : (b - a);
    }

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

    std::chrono::milliseconds TransmissionPolicyManager::increaseBackoff()
    {
        LOCKGUARD(m_backoffMutex);
        checkBackoffConfigUpdate();
        if (m_backoff == nullptr)
        {
            return std::chrono::milliseconds{};
        }

        std::chrono::milliseconds delay{m_backoff->getValue()};
        m_backoff->increase();
        return delay;
    }

    // If delayInMs is negative, do not schedule.
    void TransmissionPolicyManager::scheduleUpload(const std::chrono::milliseconds& delay, EventLatency latency, bool force)
    {
        PauseGuard guard(m_system.getLogManager());
        if (guard.isPaused()) {
            return;
        }
        if (!m_config.IsCollectorUrlSet())
        {
            LOG_TRACE("Collector URL is not set, no upload.");
            return;
        }
        LOCKGUARD(m_scheduledUploadMutex);
        if (delay.count() < 0 || m_timerdelay.count() < 0)
        {
            LOG_TRACE("Negative delay(%d) or m_timerdelay(%d), no upload", delay.count(), m_timerdelay.count());
            return;
        }
        if (m_scheduledUploadAborted)
        {
            LOG_TRACE("Scheduled upload aborted, no upload.");
            return;
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
            auto delta = Abs64(m_scheduledUploadTime, now);
            if (delta <= static_cast<uint64_t>(delay.count()))
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
        if (force || delay.count() == 0)
        {
            if (!cancelUploadTask())
            {
                LOG_TRACE("Upload either hasn't been scheduled or already done.");
            }
        }

        // Schedule new upload
        if (!m_isUploadScheduled.exchange(true))
        {
            m_scheduledUploadTime = PAL::getMonotonicTimeMs() + delay.count();
            m_runningLatency = latency;
            LOG_TRACE("SCHED upload %d ms for lat=%d", delay.count(), m_runningLatency);
            m_scheduledUpload = PAL::scheduleTask(&m_taskDispatcher, static_cast<unsigned>(delay.count()), this, &TransmissionPolicyManager::uploadAsync, latency);
        }
    }

    void TransmissionPolicyManager::uploadAsyncMax(EventLatency latency)
    {
        TransmissionPolicyManager::uploadAsync(latency);
    }

    void TransmissionPolicyManager::uploadAsync(EventLatency latency)
    {
        PauseGuard guard(m_system.getLogManager());
        if (guard.isPaused()) {
            return;
        }
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

        auto ctx = m_system.createEventsUploadContext();
        ctx->requestedMinLatency = m_runningLatency;
        addUpload(ctx);
        initiateUpload(ctx);
    }

    void TransmissionPolicyManager::finishUpload(EventsUploadContextPtr const& ctx, const std::chrono::milliseconds& nextUpload)
    {
        LOG_TRACE("HTTP upload finished for ctx=%p", ctx.get());
        if (!removeUpload(ctx))
        {
            assert(false);
            LOG_WARN("HTTP NOT removing non-existing ctx from active uploads ctx=%p", ctx.get());
        }

        PauseGuard guard(m_system.getLogManager());
        if (guard.isPaused()) {
            return;
        }
        // Rescheduling upload
        if (nextUpload.count() >= 0)
        {
            LOG_TRACE("Scheduling upload in %d ms", nextUpload.count());
            EventLatency proposed = calculateNewPriority();
            scheduleUpload(nextUpload, proposed); // reschedule uploadAsync again
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
        scheduleUpload(std::chrono::seconds{1}, calculateNewPriority());
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

    /**
     * Wait for pending uploads to finish. This handler is invoked from
     * TelemetrySystem::onCleanup after HCM has attempted to cancel all pending
     * requests via hcm.cancelAllRequests. This won't abort the uploads in the end
     * and is possible to resume the transmission
     */
     bool TransmissionPolicyManager::handleCleanup()
     {
        cancelUploadTask();
        // Make sure ongoing uploads are finished.
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

        // Check if it's time to execute the specific Max or other priority events code block
        auto currentTime = std::chrono::steady_clock::now();
        static auto maxPriorityLastExecutionTime = currentTime;
        static auto otherPriorityLastExecutionTime = currentTime;

        auto max_priority_elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - maxPriorityLastExecutionTime).count();

        /* This logic needs to be revised: one event in a dedicated HTTP post is wasteful! */
        // Initiate upload right away, but add a 2-second check to ensure some delay between consecutive initiate upload calls.
        if (event->record.latency > EventLatency_RealTime) {
            if(max_priority_elapsed_seconds < 2){
                return;
            }
            auto ctx = m_system.createEventsUploadContext();
            ctx->requestedMinLatency = event->record.latency;
            maxPriorityLastExecutionTime = currentTime;
            addUpload(ctx);
            initiateUpload(ctx);
            return;
        }

// This code block is temporarily disabled for MacOS because the MIP SDK lacks a solution for auditing on MacOS. 
// This temporary code fix was introduced to address an issue with inconsistent OneDS upload thread stoppage in all environments especially for Windows. 
// It was blocking some of the MIP SDK's tests (oneds_test.cpp) on MacOS due to timeouts. 
// This block can be removed after validation.
#ifndef __APPLE__
        // Other priorities like: Normal, Realtime, etc.
        auto other_priority_elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - otherPriorityLastExecutionTime).count();

        // Introducing a 40-second delay before forcefully scheduling the upload job, to ensure it happens at an optimal time.
        // This delay is implemented to address Issue 388, where the last cancellation might have been halted due to the issue described below.
        if ((other_priority_elapsed_seconds > 40) && m_isUploadScheduled){
            m_isUploadScheduled = false;
            LOG_TRACE("Trigger upload on event arrival");
            otherPriorityLastExecutionTime = currentTime;
        }
#endif
        // Schedule async upload if not scheduled yet
        if (!m_isUploadScheduled || TransmitProfiles::isTimerUpdateRequired())
        {
            otherPriorityLastExecutionTime = currentTime;
            if (updateTimersIfNecessary())
            {
                m_timerdelay = std::chrono::milliseconds { m_timers[1] };
                forceTimerRestart = true;
            }
            EventLatency proposed = calculateNewPriority();
            if (m_timerdelay.count() >= 0)
            {
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
            finishUpload(ctx, std::chrono::milliseconds{ -1 });
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
        finishUpload(ctx, std::chrono::milliseconds{});
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
        finishUpload(ctx, std::chrono::milliseconds{ -1 });
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
        PauseGuard guard(m_system.getLogManager());
        m_isPaused = true;
        cancelUploadTask();
    }

    std::chrono::milliseconds TransmissionPolicyManager::getCancelWaitTime() const noexcept
    {
       return (m_scheduledUploadAborted) ? DefaultTaskCancelTime : std::chrono::milliseconds {};
    }

    bool TransmissionPolicyManager::cancelUploadTask()
    {
        bool result = m_scheduledUpload.Cancel(getCancelWaitTime().count());
#ifndef __APPLE__
        // Check if it's time to execute the specific code block
        auto currentTime = std::chrono::steady_clock::now();
        static auto otherPriorityLastExecutionTimeClock = currentTime;

        auto other_priority_elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - otherPriorityLastExecutionTimeClock).count();

        // TODO: There is a potential for upload tasks to not be canceled, especially if they aren't waited for.
        //       We either need a stronger guarantee here (could impact SDK performance), or a mechanism to
        //       ensure those tasks are canceled when the log manager is destroyed. Issue 388
        // Introducing a 40-second delay before forcefully scheduling the upload job, to ensure it happens at an optimal time.
        // This delay is implemented to address Issue 388, where the last cancellation might have been halted due to the issue described below.
        if (result || other_priority_elapsed_seconds > 40)
        {
            if (other_priority_elapsed_seconds > 40)
            {
                LOG_TRACE("Reset upload on event cancellation");
            }
            m_isUploadScheduled.exchange(false);
            otherPriorityLastExecutionTimeClock = currentTime;
        }
#else
        if (result)
        {
            m_isUploadScheduled.exchange(false);
        }
#endif
        return result;
    }

    size_t TransmissionPolicyManager::uploadCount() const noexcept
    {
        LOCKGUARD(m_activeUploads_lock);
        return m_activeUploads.size();
    }

    bool TransmissionPolicyManager::isUploadInProgress() const noexcept
    {
        // unfinished uploads that haven't processed callbacks or pending upload task
        return (uploadCount() > 0) || m_isUploadScheduled;
    }

    bool TransmissionPolicyManager::isPaused() const noexcept
    {
        return m_isPaused;
    }

} MAT_NS_END
