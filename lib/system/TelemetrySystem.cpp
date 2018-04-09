// Copyright (c) Microsoft. All rights reserved.

#include "TelemetrySystem.hpp"
#include "utils/Utils.hpp"
#include "ILogManager.hpp"

namespace ARIASDK_NS_BEGIN {

// XXX: [MG] - This isn't perfect
#define BASE        PAL::RefCountedImpl<TelemetrySystemBase>::self()
#define SELF        PAL::RefCountedImpl<TelemetrySystem>::self()

/// <summary>
/// Initializes a new instance of the <see cref="TelemetrySystem"/> class.
/// </summary>
/// <param name="logManager">The log manager.</param>
/// <param name="runtimeConfig">The runtime configuration.</param>
/// <param name="offlineStorage">The offline storage.</param>
/// <param name="httpClient">The HTTP client.</param>
/// <param name="bandwidthController">The bandwidth controller.</param>
    TelemetrySystem::TelemetrySystem(
        ILogManager& logManager,
        IRuntimeConfig& runtimeConfig,
        IOfflineStorage& offlineStorage,
        IHttpClient& httpClient,
        IBandwidthController* bandwidthController)
        :
        TelemetrySystemBase(logManager, runtimeConfig),
        compression(runtimeConfig),
        hcm(httpClient),
        httpEncoder(*this, httpClient),
        httpDecoder(*this),
        storage(*this, offlineStorage),
        packager(runtimeConfig),
        tpm(*this, bandwidthController)
    {
        
        // Handler for start
        onStart = [this](void)
        {
            bool result = true;
            result&=storage.start();
            result&=tpm.start();
            result&=stats.onStart();
            return result;
        };

        // Handler for stop
        onStop = [this](void)
        {
            uint32_t timeoutInSec = m_config.GetTeardownTime();
            if (timeoutInSec > 0)
            {
                LOG_TRACE("Shutdown timer started...");
                std::uint64_t startTime = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
                upload();
                while (tpm.isUploadInProgress())
                {
                    std::uint64_t nowTime = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
                    if ((nowTime - startTime) > timeoutInSec)
                    {
                        // Hard-stop if it takes longer than planned
                        LOG_TRACE("Shutdown timer expired, exiting...");
                        break;
                    }
                    MAT::sleep(100);
                }
            }
            bool result = true;
            result &= tpm.stop();
            result &= hcm.cancelAllRequestsAsync();
            tpm.finishAllUploads();

            LOG_TRACE("Waiting for all queued callbacks...");
            m_done.wait();
            LOG_TRACE("Stopped.");

            return result;
        };

        // Handler for pause
        onPause = [this](void)
        {
            bool result = true;
            result &= tpm.pause();
            result &= hcm.cancelAllRequestsAsync();
            return result;
        };

        // Handler for resume
        onResume = [this](void)
        {
            return tpm.start();
        };

        tpm.allUploadsFinished >> stats.onStop >> storage.stop >> this->flushWorkerThread;

        // On an arbitrary user thread
        this->sending >> bondSerializer.serialize >> this->incomingEventPrepared;

        // On the inner worker thread
        this->preparedIncomingEvent >> storage.storeRecord >> stats.onIncomingEventAccepted >> tpm.eventArrived;

        // FIXME: [MG] - reenable stats!!!
#if 0
        stats.eventGenerated >> bondSerializer.serialize >> storage.storeRecord >> stats.onIncomingEventAccepted >> tpm.eventArrived;
#endif

        storage.storeRecordFailed >> stats.onIncomingEventFailed;

        tpm.initiateUpload >> storage.retrieveEvents;

        storage.retrievedEvent >> packager.addEventToPackage;
        storage.retrievalFinished >> packager.finalizePackage;

        storage.retrievalFailed >> tpm.nothingToUpload;
        packager.emptyPackage >> tpm.nothingToUpload;

        packager.packagedEvents >> compression.compress >> httpEncoder.encode >> clockSkewDelta.encode >> stats.onUploadStarted >> hcm.sendRequest;

        compression.compressionFailed >> storage.releaseRecords >> stats.onPackagingFailed >> tpm.packagingFailed;


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

    void TelemetrySystem::upload()
    {
        tpm.scheduleUpload(0, EventLatency_Normal, true);
    }

    void TelemetrySystem::handleIncomingEventPrepared(IncomingEventContextPtr const& event)
    {
        uint32_t maxBlobSize = m_config["tpm"]["maxBlobSize"];
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
        // PAL::executeOnWorkerThread(SELF, &TelemetrySystemBase::preparedIncomingEventAsync, event);
    }

    void TelemetrySystem::handleFlushWorkerThread()
    {
        signalDone();
        // PAL::executeOnWorkerThread(SELF, &TelemetrySystemBase::signalDoneEvent);
    }

} ARIASDK_NS_END
