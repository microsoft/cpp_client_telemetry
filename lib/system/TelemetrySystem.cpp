// Copyright (c) Microsoft. All rights reserved.

#include "TelemetrySystem.hpp"
#include "utils/Utils.hpp"
#include "ILogManager.hpp"

namespace ARIASDK_NS_BEGIN {

// XXX: [MG] - This isn't perfect
#define BASE        PAL::RefCountedImpl<TelemetrySystemBase>::self()
#define SELF        PAL::RefCountedImpl<TelemetrySystem>::self()

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
        //
        // Management
        //
        this->started >> storage.start >> tpm.start >> stats.onStart;

        this->stopped >> tpm.stop >> hcm.cancelAllRequestsAsync >> tpm.finishAllUploads;
        tpm.allUploadsFinished >> stats.onStop >> storage.stop >> this->flushWorkerThread;

        this->paused >> tpm.pause >> hcm.cancelAllRequestsAsync;

        this->resumed >> tpm.start;


        //
        // Incoming events
        //

        // On an arbitrary user thread
        this->sending >> bondSerializer.serialize >> this->incomingEventPrepared;

        // On the inner worker thread
        this->preparedIncomingEvent >> storage.storeRecord >> stats.onIncomingEventAccepted >> tpm.eventArrived;

#if 0
        stats.eventGenerated >> bondSerializer.serialize >> storage.storeRecord >> stats.onIncomingEventAccepted >> tpm.eventArrived;
#endif

        storage.storeRecordFailed >> stats.onIncomingEventFailed;


        //
        // Uploading
        //

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

    /// <summary>
    /// Starts this instance.
    /// </summary>
    void TelemetrySystem::start()
    {
        startAsync();
        // PAL::executeOnWorkerThread(SELF, &TelemetrySystemBase::startAsync);
    }

    void TelemetrySystem::stop()
    {
        uint32_t timeoutInSec = m_config.GetTeardownTime();
        if (timeoutInSec > 0)
        {
            std::uint64_t shutdownStartSec = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
            std::uint64_t nowSec = 0;
            upload();
            MAT::sleep(1000);
            timeoutInSec = timeoutInSec - 1;
            bool isuploadinProgress = true;
            LOG_TRACE("Shutdown timer started.");
            // Repeat this loop while we still have events to send OR while there are requests still pending
            while (isuploadinProgress)
            {
                //isuploadinProgress = tpm.isUploadInProgress();
                nowSec = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
                if ((nowSec - shutdownStartSec) >= timeoutInSec)
                { // Hard-stop if it takes longer than planned
                    LOG_TRACE("Shutdown timer expired, exiting...");
                    break;
                }
                //upload();
                MAT::sleep(25);  // Sleep in 25 ms increments
                isuploadinProgress = tpm.isUploadInProgress();
            }
        }

        stopAsync();
        // PAL::executeOnWorkerThread(SELF, &TelemetrySystemBase::stopAsync);
        LOG_TRACE("Waiting for all queued callbacks...");
        m_done.wait();
    }

    void TelemetrySystem::upload()
    {
        tpm.scheduleUpload(0, EventLatency_Normal, true);
    }

    void TelemetrySystem::pause()
    {
        pauseAsync();
        // PAL::executeOnWorkerThread(SELF, &TelemetrySystemBase::pauseTransmissionAsync);
    }

    void TelemetrySystem::resume()
    {
        resumeAsync();
        // PAL::executeOnWorkerThread(SELF, &TelemetrySystemBase::resumeTransmissionAsync);
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
