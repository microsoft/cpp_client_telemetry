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
