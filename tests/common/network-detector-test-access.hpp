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

            static void UseLegacyBackend(NetworkDetector& detector)
            {
                detector.getConnectivityHint = nullptr;
                detector.notifyConnectivityHint = nullptr;
                detector.queryLegacyCost = NetworkDetector::QueryLegacyCostInterface;
                detector.findLegacyPoint = NetworkDetector::FindLegacyConnectionPoint;
                detector.disconnectLegacyHandler = CoDisconnectObject;
            }

            static void UseLegacyBackendWithoutCost(NetworkDetector& detector)
            {
                UseLegacyBackend(detector);
                detector.queryLegacyCost = UnsupportedCost;
            }

            static bool HasLegacyCost(const NetworkDetector& detector)
            {
                return detector.networkCostManager != nullptr;
            }

            static bool HasLegacyManager(const NetworkDetector& detector)
            {
                return detector.networkListManager != nullptr;
            }

            static size_t LegacySubscriptionCount(const NetworkDetector& detector)
            {
                size_t count = 0;
                for (const auto& subscription : detector.legacySubscriptions)
                {
                    count += subscription.subscribed ? 1 : 0;
                }
                return count;
            }

            static void FailLegacyCostQuery(NetworkDetector& detector)
            {
                UseLegacyBackend(detector);
                detector.queryLegacyCost = RejectCostQuery;
            }

            static void FailLegacySubscription(NetworkDetector& detector)
            {
                UseLegacyBackend(detector);
                detector.findLegacyPoint = RejectConnectionEvents;
            }

            static void FailLegacyDisconnect(NetworkDetector& detector)
            {
                UseLegacyBackend(detector);
                detector.disconnectLegacyHandler = RejectDisconnect;
            }

            static void FailNativeSubscription(NetworkDetector& detector)
            {
                detector.getConnectivityHint = GetUnknownHint;
                detector.notifyConnectivityHint = RejectSubscription;
            }

        private:
            static HRESULT WINAPI UnsupportedCost(INetworkListManager*, INetworkCostManager** cost)
            {
                *cost = nullptr;
                return E_NOINTERFACE;
            }

            static HRESULT WINAPI RejectCostQuery(INetworkListManager*, INetworkCostManager** cost)
            {
                *cost = nullptr;
                return E_ACCESSDENIED;
            }

            static HRESULT WINAPI RejectConnectionEvents(
                IConnectionPointContainer* container, REFIID iid, IConnectionPoint** point)
            {
                if (iid == __uuidof(INetworkConnectionEvents))
                {
                    *point = nullptr;
                    return E_ACCESSDENIED;
                }
                return container->FindConnectionPoint(iid, point);
            }

            static HRESULT WINAPI RejectDisconnect(IUnknown*, DWORD)
            {
                return E_FAIL;
            }

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
