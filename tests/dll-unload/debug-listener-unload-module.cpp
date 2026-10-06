//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "callbacks/DebugSourceInternal.hpp"
#include "pal/desktop/NetworkDetector.hpp"
#include "common/network-detector-test-access.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "pal/NetworkInformationImpl.hpp"

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
enum class NetworkBackend
{
    Native,
    Legacy,
    LegacyWithoutCost
};

static bool IsStopped(MATW::NetworkDetector& detector)
{
    return !detector.isUp() && !detector.QueueNetworkCostRefresh() &&
           !MATW::NetworkDetectorTestAccess::HasLegacyManager(detector) &&
           !MATW::NetworkDetectorTestAccess::HasLegacyCost(detector) &&
           MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector) == 0;
}

static bool ExerciseNetworkDetectorBackend(NetworkBackend backend)
{
    MATW::NetworkDetector detector;
    if (backend == NetworkBackend::Legacy)
    {
        MATW::NetworkDetectorTestAccess::UseLegacyBackend(detector);
    }
    else if (backend == NetworkBackend::LegacyWithoutCost)
    {
        MATW::NetworkDetectorTestAccess::UseLegacyBackendWithoutCost(detector);
    }
    if (!detector.Start())
    {
        return false;
    }
    const bool running = detector.isUp();
    const auto cost = detector.GetCurrentNetworkCost();
    const bool readable = cost == MAT::NetworkCost_Unknown || cost == MAT::NetworkCost_Unmetered ||
                          cost == MAT::NetworkCost_Metered || cost == MAT::NetworkCost_Roaming;
    const bool subscriptions = backend == NetworkBackend::Native ||
                              MATW::NetworkDetectorTestAccess::LegacySubscriptionCount(detector) == 3;
    const bool optionalCost = backend != NetworkBackend::LegacyWithoutCost ||
                              (cost == MAT::NetworkCost_Unknown &&
                               !MATW::NetworkDetectorTestAccess::HasLegacyCost(detector));
    detector.Stop();
    return running && readable && subscriptions && optionalCost && IsStopped(detector);
}

extern "C" __declspec(dllexport) bool ExerciseNetworkDetector()
{
    return ExerciseNetworkDetectorBackend(NetworkBackend::Native);
}

extern "C" __declspec(dllexport) bool ExerciseDisabledNetworkDetection()
{
    const auto moduleBefore = GetModuleHandleW(L"netprofm.dll");
    MAT::ILogConfiguration configuration;
    configuration[MAT::CFG_BOOL_ENABLE_NET_DETECT] = false;
    MAT::RuntimeConfig_Default runtimeConfig(configuration);
    for (unsigned iteration = 0; iteration < 10; ++iteration)
    {
        auto network = PAL::NetworkInformationImpl::Create(runtimeConfig);
        if (network->GetNetworkCost() != MAT::NetworkCost_Unmetered ||
            GetModuleHandleW(L"netprofm.dll") != moduleBefore)
        {
            return false;
        }
    }
    return true;
}

extern "C" __declspec(dllexport) bool ExerciseLegacyNetworkDetector()
{
    return ExerciseNetworkDetectorBackend(NetworkBackend::Legacy);
}

extern "C" __declspec(dllexport) bool ExerciseLegacyNetworkDetectorWithoutCost()
{
    return ExerciseNetworkDetectorBackend(NetworkBackend::LegacyWithoutCost);
}

extern "C" __declspec(dllexport) bool ExerciseLegacyNetworkDetectorFailures()
{
    MATW::NetworkDetector detector;
    for (unsigned iteration = 0; iteration < 3; ++iteration)
    {
        MATW::NetworkDetectorTestAccess::FailLegacyCostQuery(detector);
        if (!detector.Start() || detector.GetCurrentNetworkCost() != MAT::NetworkCost_Unknown ||
            MATW::NetworkDetectorTestAccess::HasLegacyCost(detector))
        {
            return false;
        }
        detector.Stop();
        if (!IsStopped(detector))
        {
            return false;
        }
        MATW::NetworkDetectorTestAccess::FailLegacySubscription(detector);
        if (detector.Start() || !IsStopped(detector))
        {
            return false;
        }
        MATW::NetworkDetectorTestAccess::FailLegacyDisconnect(detector);
        if (!detector.Start())
        {
            return false;
        }
        detector.GetCurrentNetworkCost();
        detector.Stop();
        if (!IsStopped(detector))
        {
            return false;
        }
    }
    return true;
}
#endif
