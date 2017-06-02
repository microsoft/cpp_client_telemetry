// Copyright (c) Microsoft. All rights reserved.

#include "TelemetrySystem.hpp"
#include "utils/Utils.hpp"

namespace ARIASDK_NS_BEGIN {


TelemetrySystem::TelemetrySystem(LogConfiguration const& configuration, IRuntimeConfig& runtimeConfig, IOfflineStorage& offlineStorage,
    IHttpClient& httpClient, ContextFieldsProvider const& globalContext, IBandwidthController* bandwidthController)
  : compression(runtimeConfig),
    hcm(httpClient),
    httpEncoder(httpClient, runtimeConfig),
    storage(offlineStorage),
    packager(configuration, runtimeConfig),
    stats(runtimeConfig, globalContext, nullptr),
    tpm(runtimeConfig, bandwidthController),
    configuration(configuration)
{
    //
    // Management
    //

    this->started >> storage.start >> stats.onStart >> tpm.start;

    this->stopped >> tpm.stop >> hcm.cancelAllRequestsAsync >> tpm.finishAllUploads;
    tpm.allUploadsFinished >> stats.onStop >> storage.stop >> this->flushWorkerThread;

    this->paused >> tpm.pause >> hcm.cancelAllRequestsAsync;

    this->resumed >> tpm.start;


    //
    // Incoming events
    //

    // On an arbitrary user thread
    this->addIncomingEvent >>
#if ARIASDK_UTC_ENABLED
    utcForwarder.forwardIfAvailable >>
#endif
    bondSerializer.serialize >> this->incomingEventPrepared;

    // On the inner worker thread
    this->preparedIncomingEvent                      >> storage.storeRecord >> stats.onIncomingEventAccepted >> tpm.eventArrived;

    stats.eventGenerated >> bondSerializer.serialize >> storage.storeRecord >> stats.onIncomingEventAccepted >> tpm.eventArrived;

    storage.storeRecordFailed >> stats.onIncomingEventFailed;


    //
    // Uploading
    //

    tpm.initiateUpload >> storage.retrieveEvents;

    storage.retrievedEvent    >> packager.addEventToPackage;
    storage.retrievalFinished >> packager.finalizePackage;

    storage.retrievalFailed >> tpm.nothingToUpload;
    packager.emptyPackage   >> tpm.nothingToUpload;

    packager.packagedEvents >> compression.compress >> httpEncoder.encode >> clockSkewDelta.encode >> stats.onUploadStarted >> hcm.sendRequest;

    compression.compressionFailed >> storage.releaseRecords >> stats.onPackagingFailed >> tpm.packagingFailed;


    hcm.requestDone >> clockSkewDelta.decode >> httpDecoder.decode;

    httpDecoder.eventsAccepted          >> storage.deleteRecords               >> stats.onUploadSuccessful >> tpm.eventsUploadSuccessful;
    httpDecoder.eventsRejected          >> storage.deleteRecords               >> stats.onUploadRejected   >> tpm.eventsUploadRejected;
    httpDecoder.temporaryNetworkFailure >> storage.releaseRecords              >> stats.onUploadFailed     >> tpm.eventsUploadFailed;
    httpDecoder.temporaryServerFailure  >> storage.releaseRecordsIncRetryCount >> stats.onUploadFailed     >> tpm.eventsUploadFailed;
    httpDecoder.requestAborted          >> storage.releaseRecords              >> stats.onUploadFailed     >> tpm.eventsUploadAborted;


    //
    // Storage notifications
    //

    storage.opened         >> stats.onStorageOpened;
    storage.failed         >> stats.onStorageFailed;
    storage.trimmed        >> stats.onStorageTrimmed;
    storage.recordsDropped >> stats.onStorageRecordsDropped;
}

TelemetrySystem::~TelemetrySystem()
{
}

void TelemetrySystem::start()
{
    PAL::executeOnWorkerThread(self(), &TelemetrySystem::startAsync);
}

void TelemetrySystem::stop()
{
    if (configuration.maxTeardownUploadTimeInSec > 0)
    {
        unsigned int timeoutInSec = configuration.maxTeardownUploadTimeInSec;
        std::uint64_t shutdownStartSec = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
        std::uint64_t nowSec = 0;
        UploadNow();
        PAL::sleep(1000);
        timeoutInSec = timeoutInSec - 1;
        bool isuploadinProgress = true;
        ARIASDK_LOG_DETAIL("Shutdown timer started.");
        // Repeat this loop while we still have events to send OR while there are requests still pending
        while (isuploadinProgress)
        {
            //isuploadinProgress = tpm.isUploadInProgress();
            nowSec = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
            if ((nowSec - shutdownStartSec) >= timeoutInSec)
            { // Hard-stop if it takes longer than planned
                ARIASDK_LOG_DETAIL("Shutdown timer expired, exiting...");
                break;
            }
            //UploadNow();
            PAL::sleep(25);  // Sleep in 25 ms increments
            isuploadinProgress = tpm.isUploadInProgress();
        }        
    }

    PAL::executeOnWorkerThread(self(), &TelemetrySystem::stopAsync);
    ARIASDK_LOG_DETAIL("Waiting for all queued callbacks...");
    m_doneEvent.wait();
}

void TelemetrySystem::UploadNow()
{
    tpm.scheduleUpload(0);
}

void TelemetrySystem::pauseTransmission()
{
    PAL::executeOnWorkerThread(self(), &TelemetrySystem::pauseTransmissionAsync);
}

void TelemetrySystem::resumeTransmission()
{
    PAL::executeOnWorkerThread(self(), &TelemetrySystem::resumeTransmissionAsync);
}

void TelemetrySystem::handleIncomingEventPrepared(IncomingEventContextPtr const& event)
{
    event->source = nullptr;
    PAL::executeOnWorkerThread(self(), &TelemetrySystem::preparedIncomingEventAsync, event);
}

void TelemetrySystem::startAsync()
{
    m_isPaused = false;
    started();
}

void TelemetrySystem::stopAsync()
{
    m_isPaused = true;
    stopped();
}

void TelemetrySystem::handleFlushWorkerThread()
{
    PAL::executeOnWorkerThread(self(), &TelemetrySystem::signalDoneEvent);
}

void TelemetrySystem::signalDoneEvent()
{
    m_doneEvent.post();
}

void TelemetrySystem::pauseTransmissionAsync()
{
    if (m_isPaused) {
        return;
    }

    m_isPaused = true;
    paused();
}

void TelemetrySystem::resumeTransmissionAsync()
{
    if (!m_isPaused) {
        return;
    }

    m_isPaused = false;
    resumed();
}

void TelemetrySystem::preparedIncomingEventAsync(IncomingEventContextPtr const& event)
{
    preparedIncomingEvent(event);
}

void TelemetrySystem::addIncomingEventSystem(IncomingEventContextPtr const& event)
{
    addIncomingEvent(event);
}


} ARIASDK_NS_END
