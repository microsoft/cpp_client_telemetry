//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "callbacks/DebugSourceInternal.hpp"
#include "pal/desktop/NetworkDetector.hpp"
#include "common/network-detector-test-access.hpp"

#ifdef _DEBUG
static_assert(_ITERATOR_DEBUG_LEVEL == 2, "The regression requires Debug STL proxies.");
#ifndef _DLL
#error The unload regression requires the shared Debug CRT.
#endif
#endif

namespace
{
    class Listener : public MAT::DebugEventListener
    {
    public:
        void OnDebugEvent(MAT::DebugEvent&) override
        {
            ++calls;
        }

        unsigned calls = 0;
    };
}

extern "C" __declspec(dllexport) bool ExerciseDebugListeners()
{
    Listener listener;
    if (MAT::IsDebugEventListenerPending(&listener))
    {
        return false;
    }
    MAT::DebugEventSource source;
    source.AddEventListener(MAT::EVT_LOG_EVENT, listener);
    const bool dispatched = source.DispatchEvent(MAT::DebugEvent { MAT::EVT_LOG_EVENT });
    source.RemoveEventListener(MAT::EVT_LOG_EVENT, listener);
    return dispatched && listener.calls == 1 &&
           !MAT::IsDebugEventListenerPending(&listener);
}

extern "C" __declspec(dllexport) bool HasNetworkDetector()
{
#ifdef HAVE_MAT_NETDETECT
    return true;
#else
    return false;
#endif
}

#ifdef HAVE_MAT_NETDETECT
static bool ExerciseNetworkDetectorBackend(bool legacy)
{
    MATW::NetworkDetector detector;
    if (legacy)
    {
        MATW::NetworkDetectorTestAccess::UseLegacyBackend(detector);
    }
    if (!detector.Start())
    {
        return false;
    }
    const bool running = detector.isUp();
    const auto cost = detector.GetCurrentNetworkCost();
    const bool readable = cost == MAT::NetworkCost_Unknown || cost == MAT::NetworkCost_Unmetered ||
                          cost == MAT::NetworkCost_Metered || cost == MAT::NetworkCost_Roaming;
    detector.Stop();
    return running && readable && !detector.isUp() && !detector.QueueNetworkCostRefresh();
}

extern "C" __declspec(dllexport) bool ExerciseNetworkDetector()
{
    return ExerciseNetworkDetectorBackend(false);
}

extern "C" __declspec(dllexport) bool ExerciseLegacyNetworkDetector()
{
    return ExerciseNetworkDetectorBackend(true);
}
#endif
