//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "TelemetrySystem.hpp"
#include "utils/Utils.hpp"
#include "ILogManager.hpp"

#include "mat/config.h"

namespace MAT_NS_BEGIN {

/// <summary>
/// Initializes a new instance of the <see cref="TelemetrySystem"/> class.
/// </summary>
/// <param name="logManager">The log manager.</param>
/// <param name="runtimeConfig">The runtime configuration.</param>
/// <param name="offlineStorage">The offline storage.</param>
/// <param name="httpClient">The HTTP client.</param>
/// <param name="taskDispatcher">The async task dispatcher.</param>
/// <param name="bandwidthController">The bandwidth controller.</param>
    TelemetrySystem::TelemetrySystem(
        ILogManager& logManager,
        IRuntimeConfig& runtimeConfig,
        IOfflineStorage& offlineStorage,
        IHttpClient& httpClient,
        ITaskDispatcher& taskDispatcher,
        IBandwidthController* bandwidthController,
        LogSessionDataProvider& logSessionDataProvider)
        :
        TelemetrySystemBase(logManager, runtimeConfig, taskDispatcher),
        compression(runtimeConfig),
        hcm(logManager, httpClient, taskDispatcher),
        httpEncoder(*this, httpClient),
        httpDecoder(*this),
        storage(*this, offlineStorage),
        packager(runtimeConfig),
        tpm(*this, taskDispatcher, bandwidthController)
    {

        // Handler for start
        onStart = [this, &logSessionDataProvider](void)
        {
            bool result = true;
            result&=storage.start();
            result&=tpm.start();
            // TODO: clarify how UTC subsystem initializes LogSessionData m_storageType=SessionStorageType::FileStore ?
            // We may not necessarily compile in SQLite support for UTC min-build. For now we assume nullptr.
            logSessionDataProvider.CreateLogSessionData();
            result&=stats.onStart();
            return result;
        };

        // Handler for stop
        onStop = [this](void)
        {
            uint32_t timeoutInSec = m_config.GetTeardownTime();

            bool result = true;
            int64_t stopTimes[5] = { 0, 0, 0, 0, 0 };

            // Perform upload only if not paused
            if ((timeoutInSec > 0) && (!tpm.isPaused()))
            {
                // perform uploads if required
                stopTimes[0] = GetUptimeMs();
                LOG_TRACE("Shutdown timer started...");
                upload();
                // Try to push thru as much data as possible.
                // If either data is available for upload or
                // If there's outstanding request (some records marked in-flight),
                // then try to wait for up to config[CFG_INT_MAX_TEARDOWN_TIME]
                while (storage.GetRecordCount() || tpm.isUploadInProgress() )
                {
                    auto uploadTime = GetUptimeMs() - stopTimes[0];
                    if (uploadTime >= (1000L * timeoutInSec))
                    {
                        // Hard-stop if it takes longer than planned
                        LOG_TRACE("Shutdown timer expired, exiting...");
                        break;
                    }
                    MAT::sleep(100);
                    LOG_INFO("offline records=%zu, pending uploads=%zu", storage.GetRecordCount(), hcm.requestCount());
                }
                stopTimes[0] = GetUptimeMs() - stopTimes[0];
            }

            // cancel all pending and force-finish all uploads
            stopTimes[1] = GetUptimeMs();
            // TODO: Should this still pause, since the TPM now has abort logic in addition to pause logic?
            // hcm.cancelAllRequests is also part of pause, so the logic is definitely redundant. Issue 387
            onPause();
            hcm.cancelAllRequests();
            tpm.finishAllUploads();
            stopTimes[1] = GetUptimeMs() - stopTimes[1];

            // initiate the stop sequence
            stopTimes[2] = GetUptimeMs();
            result &= tpm.stop();
            stopTimes[2] = GetUptimeMs() - stopTimes[2];

            // cancel all pending tasks
            stopTimes[3] = GetUptimeMs();
            LOG_TRACE("Waiting for all queued callbacks...");
            m_done.wait();
            LOG_TRACE("Stopped.");
            stopTimes[3] = GetUptimeMs() - stopTimes[3];

            // stop storage
            stopTimes[4] = GetUptimeMs();
            storage.stop();
            stopTimes[4] = GetUptimeMs() - stopTimes[4];

#if 1       // Shutdown performance printout
            LOG_WARN("upload  = %lld ms", stopTimes[0]);
            LOG_WARN("abort   = %lld ms", stopTimes[1]);
            LOG_WARN("stop    = %lld ms", stopTimes[2]);
            LOG_WARN("worker  = %lld ms", stopTimes[3]);
            LOG_WARN("storage = %lld ms", stopTimes[4]);
#endif

            return result;
        };

        // Handler for pause
        onPause = [this](void)
        {
            bool result = true;
            result &= tpm.pause();
            hcm.cancelAllRequests();
            return result;
        };

        // Handler for resume
        onResume = [this](void)
        {
            return tpm.start();
        };

        onCleanup = [this](void)
        {
            bool result = true;
            hcm.cancelAllRequests();
            result &= tpm.cleanup();
            return result;
        };

        tpm.allUploadsFinished >> stats.onStop >> this->flushTaskDispatcher;

        // On an arbitrary user thread
        this->sending >> bondSerializer.serialize >> this->incomingEventPrepared;

        // On the inner worker thread
        this->preparedIncomingEvent >> storage.storeRecord >> stats.onIncomingEventAccepted >> tpm.eventArrived;


        storage.storeRecordFailed >> stats.onIncomingEventFailed;

        tpm.initiateUpload >> storage.retrieveEvents;

        storage.retrievedEvent >> packager.addEventToPackage;
        storage.retrievalFinished >> packager.finalizePackage;

        storage.retrievalFailed >> tpm.nothingToUpload;
        packager.emptyPackage >> tpm.nothingToUpload;

        packager.packagedEvents >>
#ifdef HAVE_MAT_ZLIB
        compression.compress >>
#endif
        httpEncoder.encode >> clockSkewDelta.encode >> stats.onUploadStarted >> hcm.sendRequest;

#ifdef HAVE_MAT_ZLIB
        compression.compressionFailed >> storage.releaseRecords >> stats.onPackagingFailed >> tpm.packagingFailed;
#endif

        hcm.requestDone >> clockSkewDelta.decode >> httpDecoder.decode;

        httpDecoder.eventsAccepted >> storage.deleteRecords >> stats.onUploadSuccessful >> tpm.eventsUploadSuccessful;
        httpDecoder.eventsRejected >> storage.deleteRecords >> stats.onUploadRejected >> tpm.eventsUploadRejected;
        httpDecoder.temporaryNetworkFailure >> storage.releaseRecords >> stats.onUploadFailed >> tpm.eventsUploadFailed;
        httpDecoder.temporaryServerFailure >> storage.releaseRecordsIncRetryCount >> stats.onUploadFailed >> tpm.eventsUploadFailed;
        httpDecoder.requestAborted >> storage.releaseRecords >> stats.onUploadFailed >> tpm.eventsUploadAborted;


        //
        // Storage notifications
        //

        storage.opened >> stats.onStorageOpened;
        storage.failed >> stats.onStorageFailed;
        storage.trimmed >> stats.onStorageTrimmed;
        storage.recordsDropped >> stats.onStorageRecordsDropped;
        storage.recordsRejected >> stats.onStorageRecordsRejected;
    }

    TelemetrySystem::~TelemetrySystem()
    {
    }

    bool TelemetrySystem::upload()
    {
        size_t recordCount = storage.GetRecordCount();
        if (recordCount)
        {
            tpm.scheduleUpload(std::chrono::milliseconds {}, EventLatency_Normal, true);
            return true;
        }

        return false;
    }

    void TelemetrySystem::handleIncomingEventPrepared(IncomingEventContextPtr const& event)
    {
        uint32_t maxBlobSize = m_config[CFG_MAP_TPM][CFG_INT_TPM_MAX_BLOB_BYTES];
        if (event->record.blob.size() > maxBlobSize)
        {
            DebugEvent evt;
            evt.type = DebugEventType::EVT_REJECTED;
            evt.param1 = REJECTED_REASON_EVENT_SIZE_LIMIT_EXCEEDED;
            m_logManager.DispatchEvent(evt);
            LOG_INFO("Event %s/%s dropped because size more than 2 MB",
                tenantTokenToId(event->record.tenantToken).c_str(), event->source->baseType.c_str());
            return;
        }

        event->source = nullptr;
        preparedIncomingEventAsync(event);
    }

    void TelemetrySystem::handleFlushTaskDispatcher()
    {
        signalDone();
    }

} MAT_NS_END

