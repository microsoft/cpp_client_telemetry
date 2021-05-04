//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "EventDecoderListener.hpp"
#include "PayloadDecoder.hpp"

unsigned   latency[MAX_LATENCY_SAMPLES] = { 0 };

std::atomic<size_t>   eps(0);
std::atomic<size_t>   numLogged0(0);
std::atomic<size_t>   numLogged(0);
std::atomic<size_t>   numSent(0);
std::atomic<size_t>   numDropped(0);
std::atomic<size_t>   numReject(0);
std::atomic<size_t>   numCached(0);
std::atomic<size_t>   logLatMin(100);
std::atomic<size_t>   logLatMax(0);
unsigned long         testStartMs;

/// <summary>
/// The network cost names
/// </summary>
const char* networkCostNames[] = {
    "Unknown",
    "Unmetered",
    "Metered",
    "Roaming",
};

/// <summary>
/// Resets this instance.
/// </summary>
void EventDecoderListener::reset()
{
    testStartMs = (unsigned long)(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
    eps = 0;
    numLogged0 = 0;
    numLogged = 0;
    numSent = 0;
    numDropped = 0;
    numReject = 0;
    numCached = 0;
    logLatMin = 100;
    logLatMax = 0;
}

void EventDecoderListener::DecodeBuffer(void *data, size_t size)
{
    if (data && size)
    {
        std::vector<uint8_t> in;
        in.assign((uint8_t*)data, (uint8_t*)data + size);
        std::string s;
        exporters::DecodeRequest(in, s, true);
        // printf("%s\n", s.c_str());
    }
}

/// <summary>
/// The DebugEventListener constructor.
/// </summary>
/// <param name="evt"></param>
void EventDecoderListener::OnDebugEvent(DebugEvent &evt)
{
    auto PrintEvent = [](const char *lbl, const DebugEvent &e)
    {
        printf("%20s: seq=%llu, ts=%llu, type=0x%08x, p1=%zu, p2=%zu\n", lbl, static_cast<unsigned long long>(e.seq), static_cast<unsigned long long>(e.ts), e.type, e.param1, e.param2);
    };

    // lock for the duration of the print, so that we don't mess up the prints
    std::lock_guard<std::mutex> lock(dbg_callback_mtx);
    unsigned long ms;

    switch (evt.type) {

    case EVT_LOG_EVENT:
        // Track LogEvent latency here
        if (evt.param1 < logLatMin)
            logLatMin = evt.param1;
        if (evt.param1 > logLatMax)
            logLatMax = evt.param1;
    case EVT_LOG_LIFECYCLE:
    case EVT_LOG_FAILURE:
    case EVT_LOG_PAGEVIEW:
    case EVT_LOG_PAGEACTION:
    case EVT_LOG_SAMPLEMETR:
    case EVT_LOG_AGGRMETR:
    case EVT_LOG_TRACE:
    case EVT_LOG_USERSTATE:
    case EVT_LOG_SESSION:
        numLogged++;
        ms = (unsigned long)(std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1));
        {
            eps = (1000 * numLogged) / (ms - testStartMs);
            if ((numLogged % 500) == 0)
            {
                printf("EPS=%zu\n", eps.load());
            }
        }
        break;

    case EVT_REJECTED:
        numReject++;
        PrintEvent("EVT_REJECTED", evt);
        break;

    case EVT_ADDED:
        PrintEvent("EVT_ADDED", evt);
        break;

    case EVT_CACHED:
        numCached += evt.param1;
        PrintEvent("EVT_CACHED", evt);
        break;

    case EVT_DROPPED:
        numDropped += evt.param1;
        PrintEvent("EVT_DROPPED", evt);
        break;

    case EVT_SENT:
        numSent += evt.param1;
        PrintEvent("EVT_SENT", evt);
        break;

    case EVT_STORAGE_FULL:
        PrintEvent("EVT_STORAGE_FULL", evt);
        if (evt.param1 >= 75) {
            // UploadNow must NEVER EVER be called from Aria callback thread, so either use this structure below
            // or notify the main app that it has to do the profile timers housekeeping / force the upload...
            std::thread([]() { LogManager::UploadNow(); }).detach();
        }
        break;

    case EVT_CONN_FAILURE:
    case EVT_HTTP_FAILURE:
    case EVT_COMPRESS_FAILED:
    case EVT_UNKNOWN_HOST:
    case EVT_SEND_FAILED:
        PrintEvent("EVT_SEND_FAILED", evt);
        break;

    case EVT_HTTP_ERROR:
        PrintEvent("EVT_HTTP_ERROR", evt);
        DecodeBuffer(evt.data, evt.size);
        break;

    case EVT_HTTP_OK:
        PrintEvent("EVT_HTTP_OK", evt);
        DecodeBuffer(evt.data, evt.size);
        break;

    case EVT_SEND_RETRY:
        PrintEvent("EVT_SEND_RETRY", evt);
        break;

    case EVT_SEND_RETRY_DROPPED:
        PrintEvent("EVT_SEND_RETRY_DROPPED", evt);
        break;

    case EVT_NET_CHANGED:
        PrintEvent("EVT_NET_CHANGED", evt);
        break;

    case EVT_UNKNOWN:
    default:
        PrintEvent("EVT_UNKNOWN", evt);
        break;
    };

};

