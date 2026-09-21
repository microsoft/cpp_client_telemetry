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
#include <memory>
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

                NetworkCost MapNetworkCost(
                    NetworkCostType costType,
                    boolean roaming,
                    boolean overDataLimit,
                    boolean approachingDataLimit,
                    boolean backgroundDataUsageRestricted);

                class NetworkDetector
                {
                   private:
                    struct CallbackState;
                    enum class StartupState
                    {
                        Stopped,
                        Starting,
                        Ready,
                        Failed
                    };

                    /// <summary>
                    /// Current network info stats
                    /// </summary>
                    ComPtr<INetworkInformationStatics>  networkInfoStats;
                    ComPtr<INetworkStatusChangedEventHandler> networkStatusChangedHandler;
                    EventRegistrationToken              networkStatusChangedToken{};
                    std::shared_ptr<CallbackState>       networkStatusCallbackState;


                    /// <summary>
                    /// Get instance of network info stats
                    /// </summary>
                    /// <returns></returns>
                    bool GetNetworkInfoStats();

                    std::mutex                          m_lock;
                    std::condition_variable             cv;
                    std::atomic<bool>                   isRunning{ false };
                    std::thread                         netDetectThread;
                    StartupState startupState = StartupState::Stopped;
                    bool stopRequested = false;

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

                    std::shared_ptr<std::atomic<NetworkCost>> m_currentNetworkCost{
                        std::make_shared<std::atomic<NetworkCost>>(NetworkCost_Unknown)
                    };

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

                    /// <summary>
                    /// Queue the same refresh performed by a WinRT network status callback.
                    /// </summary>
                    bool QueueNetworkCostRefresh();
                };
            }
} MAT_NS_END

namespace MATW = MAT::Windows;

#endif

#endif
