//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#include "pal/desktop/NetworkDetector.hpp"

#ifdef HAVE_MAT_NETDETECT
namespace MAT_NS_BEGIN
{
    namespace Windows
    {
        class NetworkDetectorTestAccess
        {
        public:
            static bool HasNativeBackend(const NetworkDetector& detector)
            {
                return detector.getConnectivityHint != nullptr && detector.notifyConnectivityHint != nullptr;
            }

            static void DisableNativeBackend(NetworkDetector& detector, bool query = true, bool subscription = true)
            {
                if (query)
                {
                    detector.getConnectivityHint = nullptr;
                }
                if (subscription)
                {
                    detector.notifyConnectivityHint = nullptr;
                }
            }

            static bool HasListenerResources(const NetworkDetector& detector)
            {
                return detector.netDetectThread.joinable() || detector.stopEvent != nullptr ||
                       detector.networkStatusNotification != nullptr ||
                       detector.networkStatusCallbackState != nullptr || detector.m_listener_tid != 0;
            }

            static bool HasDispatchState(const NetworkDetector& detector)
            {
                return detector.eventDispatchState != nullptr;
            }

            static void SetCachedCost(NetworkDetector& detector, NetworkCost cost)
            {
                detector.m_currentNetworkCost->store(cost, std::memory_order_relaxed);
            }

            static void RestoreNativeBackend(NetworkDetector& detector)
            {
                const auto module = GetModuleHandleW(L"iphlpapi.dll");
                detector.getConnectivityHint = reinterpret_cast<NetworkDetector::GetConnectivityHint>(
                    GetProcAddress(module, "GetNetworkConnectivityHint"));
                detector.notifyConnectivityHint = reinterpret_cast<NetworkDetector::NotifyConnectivityHint>(
                    GetProcAddress(module, "NotifyNetworkConnectivityHintChange"));
            }

            static void FailNativeSubscription(NetworkDetector& detector)
            {
                detector.getConnectivityHint = GetUnknownHint;
                detector.notifyConnectivityHint = RejectSubscription;
            }

        private:
            static DWORD WINAPI GetUnknownHint(NL_NETWORK_CONNECTIVITY_HINT* hint)
            {
                *hint = {};
                return NO_ERROR;
            }

            static DWORD WINAPI RejectSubscription(NetworkDetector::HintChangedCallback, void*, BOOLEAN, HANDLE*)
            {
                return ERROR_ACCESS_DENIED;
            }
        };
    }
}
MAT_NS_END
#endif
