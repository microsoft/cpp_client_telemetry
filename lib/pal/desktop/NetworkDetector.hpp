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
#include <netlistmgr.h>
#include <nldef.h>

#include <atomic>
#include <array>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "Enums.hpp"

using namespace Microsoft::WRL;

namespace MAT_NS_BEGIN
{
            namespace Windows {

                NetworkCost MapNetworkCost(
                    NL_NETWORK_CONNECTIVITY_COST_HINT costType,
                    bool roaming,
                    bool overDataLimit,
                    bool approachingDataLimit);

                NetworkCost MapLegacyNetworkCost(DWORD cost);

                class NetworkDetector
                {
                   private:
                    struct CallbackState;
                    struct EventDispatchState;
                    struct NetworkStatusChangedSink;
                    friend class NetworkDetectorTestAccess;
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
                    using GetConnectivityHint = DWORD(WINAPI*)(NL_NETWORK_CONNECTIVITY_HINT*);
                    using HintChangedCallback = void(WINAPI*)(void*, NL_NETWORK_CONNECTIVITY_HINT);
                    using NotifyConnectivityHint = DWORD(WINAPI*)(HintChangedCallback, void*, BOOLEAN, HANDLE*);
                    GetConnectivityHint getConnectivityHint = nullptr;
                    NotifyConnectivityHint notifyConnectivityHint = nullptr;
                    HANDLE networkStatusNotification = nullptr;
                    ComPtr<INetworkListManager> networkListManager;
                    ComPtr<INetworkCostManager> networkCostManager;
                    struct LegacySubscription
                    {
                        ComPtr<IConnectionPoint> point;
                        DWORD cookie = 0;
                        bool subscribed = false;
                    };
                    std::array<LegacySubscription, 3> legacySubscriptions;
                    ComPtr<IUnknown> networkStatusChangedHandler;
                    using QueryLegacyCost = HRESULT(WINAPI*)(INetworkListManager*, INetworkCostManager**);
                    using FindLegacyPoint = HRESULT(WINAPI*)(IConnectionPointContainer*, REFIID, IConnectionPoint**);
                    static HRESULT WINAPI QueryLegacyCostInterface(INetworkListManager*, INetworkCostManager**);
                    static HRESULT WINAPI FindLegacyConnectionPoint(IConnectionPointContainer*, REFIID, IConnectionPoint**);
                    QueryLegacyCost queryLegacyCost = QueryLegacyCostInterface;
                    FindLegacyPoint findLegacyPoint = FindLegacyConnectionPoint;
                    decltype(&CoDisconnectObject) disconnectLegacyHandler = CoDisconnectObject;
                    std::shared_ptr<CallbackState> networkStatusCallbackState;
                    std::shared_ptr<EventDispatchState> eventDispatchState;

                    /// <summary>
                    /// Get instance of network info stats
                    /// </summary>
                    /// <returns></returns>
                    bool InitializeNetworkCost();
                    NetworkCost QueryNetworkCost();
                    static void WINAPI NetworkHintChanged(void* context, NL_NETWORK_CONNECTIVITY_HINT hint);

                    std::mutex m_lifecycleLock;
                    std::mutex m_lock;
                    std::condition_variable cv;
                    std::atomic<bool> isRunning{false};
                    std::thread netDetectThread;
                    StartupState startupState = StartupState::Stopped;
                    bool stopRequested = false;
                    uint64_t refreshSequence = 0;
                    HANDLE stopEvent = nullptr;

                    /// <summary>
                    ///
                    /// </summary>
                    void run();

                    DWORD m_listener_tid = 0;

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
                    NetworkDetector();

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
                    /// Queue the same refresh performed by a network status callback.
                    /// </summary>
                    bool QueueNetworkCostRefresh();
                };
            }
} MAT_NS_END

namespace MATW = MAT::Windows;

#endif

#endif
