//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef NETWORKDETECTOR_HPP
#define NETWORKDETECTOR_HPP
#include "mat/config.h"
#ifdef HAVE_MAT_NETDETECT

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

#include <Windows.h>

#include <wrl.h>
#include <windows.foundation.h>
#include <windows.networking.connectivity.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "Enums.hpp"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Networking::Connectivity;

namespace MAT_NS_BEGIN
{
            namespace Windows {

                class NetworkDetector {

                private:

                    /// <summary>
                    /// Current network info stats
                    /// </summary>
                    ComPtr<INetworkInformationStatics>  networkInfoStats;
                    ComPtr<INetworkStatusChangedEventHandler> networkStatusChangedHandler;
                    EventRegistrationToken              networkStatusChangedToken{};


                    /// <summary>
                    /// Obtain network cost RO. This function does not handle potential exceptions and must only be called from GetNetworkCost()
                    /// </summary>
                    /// <returns></returns>
                    NetworkCost _GetCurrentNetworkCost();

                    /// <summary>
                    /// Get instance of network info stats
                    /// </summary>
                    /// <returns></returns>
                    bool GetNetworkInfoStats();

                    std::mutex                          m_lock;
                    std::condition_variable             cv;
                    std::atomic<bool>                   isRunning{ false };
                    std::thread                         netDetectThread;

                    /// <summary>
                    /// 
                    /// </summary>
                    void run();

                    DWORD                               m_listener_tid = 0;

                    /// <summary>
                    /// Register and listen to network state notifications
                    /// </summary>
                    /// <returns></returns>
                    bool RegisterAndListen() noexcept;

                    /// <summary>
                    /// Reset network state listener to uninitialized state
                    /// </summary>
                    void Reset();

                    std::atomic<NetworkCost>            m_currentNetworkCost{ NetworkCost_Unknown };

                public:

                    /// <summary>
                    /// 
                    /// </summary>
                    bool isUp() { return isRunning.load(std::memory_order_relaxed); };

                    /// <summary>
                    /// Createa network status listener
                    /// </summary>
                    NetworkDetector() = default;

                    /// <summary>
                    /// 
                    /// </summary>
                    /// <returns></returns>
                    bool Start();

                    /// <summary>
                    /// 
                    /// </summary>
                    void Stop();

                    /// <summary>
                    /// 
                    /// </summary>
                    ~NetworkDetector();

                    /// <summary>
                    /// Get current network cost
                    /// </summary>
                    /// <returns></returns>
                    int GetCurrentNetworkCost();

                    /// <summary>
                    /// Get last cached network cost
                    /// </summary>
                    /// <returns></returns>
                    NetworkCost GetNetworkCost();

                };

            }
} MAT_NS_END

namespace MATW = MAT::Windows;

#endif

#endif
