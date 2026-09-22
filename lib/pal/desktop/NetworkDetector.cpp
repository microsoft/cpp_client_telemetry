//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"
#include "mat/config.h"
#ifdef HAVE_MAT_NETDETECT

#pragma comment(lib, "runtimeobject.lib")

#include "NetworkDetector.hpp"

#include "DebugEvents.hpp"
#include "ILogManager.hpp"
#include "pal/PAL.hpp"

#define NETDETECTOR_REFRESH WM_USER + 1

namespace MAT_NS_BEGIN
{
    namespace Windows
    {

        static thread_local void* currentNetworkEventDispatch = nullptr;

        struct NetworkDetector::CallbackState
        {
            std::atomic<DWORD> listenerThreadId{0};

            bool QueueRefresh() const
            {
                const auto threadId = listenerThreadId.load(std::memory_order_acquire);
                return threadId != 0 &&
                       PostThreadMessage(threadId, NETDETECTOR_REFRESH, 0, NULL) != FALSE;
            }
        };

        struct NetworkDetector::EventDispatchState : std::enable_shared_from_this<NetworkDetector::EventDispatchState>
        {
            bool Queue(NetworkCost cost)
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (!acceptEvents)
                {
                    return false;
                }

                latestCost = cost;
                eventPending = true;
                if (workerScheduled)
                {
                    return true;
                }

                workerScheduled = true;
                auto context = new (std::nothrow) std::shared_ptr<EventDispatchState>(shared_from_this());
                if (context == nullptr ||
                    !QueueUserWorkItem(DispatchPendingEvents, context, WT_EXECUTEDEFAULT))
                {
                    delete context;
                    workerScheduled = false;
                    return false;
                }
                return true;
            }

            void StopAndWait()
            {
                std::unique_lock<std::mutex> lock(mutex);
                acceptEvents = false;
                eventPending = false;
                if (currentNetworkEventDispatch == this)
                {
                    return;
                }
                cv.wait(lock, [this]()
                        { return !workerScheduled; });
            }

           private:
            static DWORD CALLBACK DispatchPendingEvents(void* context)
            {
                std::shared_ptr<EventDispatchState> state =
                    *static_cast<std::shared_ptr<EventDispatchState>*>(context);
                delete static_cast<std::shared_ptr<EventDispatchState>*>(context);
                currentNetworkEventDispatch = state.get();

                while (true)
                {
                    NetworkCost cost;
                    {
                        std::lock_guard<std::mutex> lock(state->mutex);
                        if (!state->acceptEvents || !state->eventPending)
                        {
                            state->workerScheduled = false;
                            state->cv.notify_all();
                            currentNetworkEventDispatch = nullptr;
                            return 0;
                        }
                        cost = state->latestCost;
                        state->eventPending = false;
                    }

                    DebugEvent evt;
                    evt.type = DebugEventType::EVT_NET_CHANGED;
                    evt.param1 = cost;
                    evt.param2 = false;
                    ILogManager::DispatchEventBroadcast(evt);
                }
            }

            std::mutex mutex;
            std::condition_variable cv;
            NetworkCost latestCost = NetworkCost_Unknown;
            bool acceptEvents = true;
            bool eventPending = false;
            bool workerScheduled = false;
        };

        NetworkCost MapNetworkCost(
            NetworkCostType costType,
            boolean roaming,
            boolean overDataLimit,
            boolean approachingDataLimit,
            boolean backgroundDataUsageRestricted)
        {
            if (roaming || overDataLimit || approachingDataLimit || backgroundDataUsageRestricted)
            {
                return NetworkCost_Roaming;
            }

            switch (costType)
            {
            case NetworkCostType_Unrestricted:
                return NetworkCost_Unmetered;
            case NetworkCostType_Fixed:
            case NetworkCostType_Variable:
                return NetworkCost_Metered;
            case NetworkCostType_Unknown:
            default:
                return NetworkCost_Unknown;
            }
        }

        static NetworkCost QueryCurrentNetworkCost(INetworkInformationStatics* networkInfoStats)
        {
            NetworkCost result = NetworkCost_Unknown;
            LOG_TRACE("get network cost...\n");

            if (networkInfoStats == nullptr)
            {
                LOG_WARN("Windows network information is unavailable!");
                return result;
            }

            ComPtr<IConnectionProfile> connectionProfile;
            HRESULT hr = networkInfoStats->GetInternetConnectionProfile(&connectionProfile);
            if (FAILED(hr) || connectionProfile == nullptr)
            {
                return result;
            }

            ComPtr<IConnectionCost> connectionCost;
            hr = connectionProfile->GetConnectionCost(&connectionCost);
            if (FAILED(hr) || connectionCost == nullptr)
            {
                return result;
            }

            boolean roaming = false;
            boolean overDataLimit = false;
            boolean approachingDataLimit = false;
            boolean backgroundDataUsageRestricted = false;
            NetworkCostType costType = NetworkCostType_Unknown;
            if (FAILED(connectionCost->get_Roaming(&roaming)) ||
                FAILED(connectionCost->get_OverDataLimit(&overDataLimit)) ||
                FAILED(connectionCost->get_ApproachingDataLimit(&approachingDataLimit)) ||
                FAILED(connectionCost->get_NetworkCostType(&costType)))
            {
                return result;
            }

            ComPtr<IConnectionCost2> connectionCost2;
            if (SUCCEEDED(connectionCost.As(&connectionCost2)) &&
                FAILED(connectionCost2->get_BackgroundDataUsageRestricted(&backgroundDataUsageRestricted)))
            {
                return result;
            }

            return MapNetworkCost(
                costType,
                roaming,
                overDataLimit,
                approachingDataLimit,
                backgroundDataUsageRestricted);
        }

        /// <summary>
        /// Get current realtime network cost synchronously.
        /// This function provides an SEH handler for Windows Runtime failures.
        /// </summary>
#pragma warning(push)
#pragma warning(disable : 6320)
        static int RefreshNetworkCost(
            INetworkInformationStatics* networkInfoStats,
            std::atomic<NetworkCost>& currentNetworkCostState)
        {
            NetworkCost currentNetworkCost = NetworkCost_Unknown;
            __try
            {
                currentNetworkCost = QueryCurrentNetworkCost(networkInfoStats);
            }
            //******************************************************************************************************************************
            // This code is required as a workaround for an issue in Visual Studio debug host mode: crash in W.N.C.dll
            //
            // onecoreuap\net\netprofiles\winrt\networkinformation\lib\handlemanager.cpp(132)\Windows.Networking.Connectivity.dll!0FBCFB9E:
            // (caller: 0FBCEE2C) ReturnHr(1) tid(4584) 80070426 The service has not been started.
            //
            // Exception thrown at XXX (KernelBase.dll) in YYY : The binding handle is invalid.
            // If there is a handler for this exception, the program may be safely continued.
            //*******************************************************************************************************************************
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                LOG_ERROR("Unable to obtain network state!");
            }

            currentNetworkCostState.store(currentNetworkCost, std::memory_order_relaxed);
            return currentNetworkCost;
        }
#pragma warning(pop)

        NetworkCost NetworkDetector::GetNetworkCost()
        {
            return m_currentNetworkCost->load(std::memory_order_relaxed);
        }

        int NetworkDetector::GetCurrentNetworkCost()
        {
            const auto currentNetworkCost =
                RefreshNetworkCost(networkInfoStats.Get(), *m_currentNetworkCost);
            std::shared_ptr<EventDispatchState> dispatchState;
            {
                std::lock_guard<std::mutex> lock(m_lock);
                dispatchState = eventDispatchState;
            }
            if (dispatchState != nullptr && !dispatchState->Queue(static_cast<NetworkCost>(currentNetworkCost)))
            {
                LOG_WARN("Unable to queue network status event.");
            }
            return currentNetworkCost;
        }

        bool NetworkDetector::QueueNetworkCostRefresh()
        {
            std::shared_ptr<CallbackState> callbackState;
            {
                std::lock_guard<std::mutex> lock(m_lock);
                callbackState = networkStatusCallbackState;
            }
            return callbackState != nullptr && callbackState->QueueRefresh();
        }

        /// <summary>
        /// Get activation factory and look-up network info statistics
        /// </summary>
        /// <returns></returns>
        bool NetworkDetector::GetNetworkInfoStats()
        {
            HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Networking_Connectivity_NetworkInformation).Get(), &networkInfoStats);
            if (hr != S_OK)
            {
                LOG_ERROR("Unable to get Windows::Networking::Connectivity::NetworkInformation");
                return false;
            }
            return true;
        }

        bool NetworkDetector::RegisterAndListen() noexcept
        {
            MSG msg;
            PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

            const auto callbackState = networkStatusCallbackState;
            callbackState->listenerThreadId.store(GetCurrentThreadId(), std::memory_order_release);
            networkStatusChangedHandler = Callback<INetworkStatusChangedEventHandler>(
                [callbackState](IInspectable*) -> HRESULT
                {
                    callbackState->QueueRefresh();
                    return S_OK;
                });
            if (networkStatusChangedHandler == nullptr)
            {
                LOG_ERROR("Unable to create network status handler.");
                callbackState->listenerThreadId.store(0, std::memory_order_release);
                return false;
            }

            HRESULT hr = networkInfoStats->add_NetworkStatusChanged(
                networkStatusChangedHandler.Get(),
                &networkStatusChangedToken);
            if (FAILED(hr))
            {
                LOG_ERROR("Unable to subscribe to network status changes.");
                callbackState->listenerThreadId.store(0, std::memory_order_release);
                networkStatusChangedHandler.Reset();
                return false;
            }

            {
                std::lock_guard<std::mutex> lock(m_lock);
                if (stopRequested)
                {
                    startupState = StartupState::Failed;
                    cv.notify_all();
                    return false;
                }
                startupState = StartupState::Ready;
                cv.notify_all();
            }
            if (!eventDispatchState->Queue(GetNetworkCost()))
            {
                LOG_WARN("Unable to queue initial network status event.");
            }

            while (true)
            {
                const DWORD waitResult = MsgWaitForMultipleObjectsEx(
                    1,
                    &stopEvent,
                    INFINITE,
                    QS_ALLINPUT,
                    MWMO_INPUTAVAILABLE);
                if (waitResult == WAIT_OBJECT_0)
                {
                    break;
                }
                if (waitResult == WAIT_FAILED)
                {
                    LOG_ERROR("Unable to wait for network detector events.");
                    return false;
                }
                if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    continue;
                }
                if (msg.message == WM_QUIT)
                {
                    break;
                }

                switch (msg.message)
                {
                case NETDETECTOR_REFRESH:
                    GetCurrentNetworkCost();
                    break;
                default:
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            return true;
        }

        /// <summary>
        ///
        /// </summary>
        void NetworkDetector::Reset()
        {
            if (networkStatusCallbackState != nullptr)
            {
                networkStatusCallbackState->listenerThreadId.store(0, std::memory_order_release);
            }
            if (networkStatusChangedToken.value != 0 && networkInfoStats != nullptr)
            {
                const auto token = networkStatusChangedToken;
                networkStatusChangedToken.value = 0;
                networkInfoStats->remove_NetworkStatusChanged(token);
            }
            networkStatusChangedHandler.Reset();
            networkInfoStats.Reset();
        }

        /// <summary>
        /// Register for Windows Runtime events and block-wait in RegisterAndListen
        /// </summary>
#pragma warning(push)
#pragma warning(disable : 6320)
        void NetworkDetector::run()
        {
            bool isRoInitialized = false;

            __try
            {
                __try
                {
                    HRESULT hr = RoInitialize(RO_INIT_MULTITHREADED);
                    if (FAILED(hr))
                    {
                        LOG_ERROR("RoInitialize failed.");
                        return;
                    }

                    isRoInitialized = true;
                    if (GetNetworkInfoStats())
                    {
                        RefreshNetworkCost(networkInfoStats.Get(), *m_currentNetworkCost);
                        LOG_TRACE("start listening to events...");
                        RegisterAndListen();
                    }
                }
                __finally
                {
                    Reset();
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                LOG_ERROR("Handled exception in Windows Runtime network cost detection.");
            }

            if (isRoInitialized)
            {
                RoUninitialize();
            }
        }
#pragma warning(pop)

        /// <summary>
        /// Start network monitoring thread
        /// </summary>
        /// <returns>true - if start is successful, false - otherwise</returns>
        bool NetworkDetector::Start()
        {
            std::lock_guard<std::mutex> lifecycleLock(m_lifecycleLock);
            {
                std::unique_lock<std::mutex> lock(m_lock);
                if (startupState == StartupState::Starting)
                {
                    cv.wait(lock, [this]()
                            { return startupState != StartupState::Starting; });
                }
                if (startupState == StartupState::Ready)
                {
                    LOG_TRACE("NetworkDetector tid=%p is already running", m_listener_tid);
                    return true;
                }

                lock.unlock();
                if (netDetectThread.joinable())
                {
                    netDetectThread.join();
                }
                lock.lock();

                startupState = StartupState::Starting;
                stopRequested = false;
                networkStatusCallbackState = std::make_shared<CallbackState>();
                eventDispatchState = std::make_shared<EventDispatchState>();
                stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
                if (stopEvent == nullptr)
                {
                    LOG_ERROR("Unable to create the network detector stop event.");
                    startupState = StartupState::Failed;
                    networkStatusCallbackState.reset();
                    eventDispatchState.reset();
                    return false;
                }
                isRunning = true;
            }

            // Start a new thread. Notify waiters on exit.
            netDetectThread = std::thread([this]()
                                          {
                {
                    std::lock_guard<std::mutex> lk(m_lock);
                    m_listener_tid = GetCurrentThreadId();
                }
                run();
                LOG_TRACE("NetworkDetector tid=%p is shutting down..", m_listener_tid);
                {
                    std::lock_guard<std::mutex> lk(m_lock);
                    m_listener_tid = 0;
                    isRunning = false;
                    if (startupState == StartupState::Starting)
                    {
                        startupState = StartupState::Failed;
                    }
                    else if (startupState == StartupState::Ready)
                    {
                        startupState = StartupState::Stopped;
                    }
                    cv.notify_all();
                } });

            {
                LOG_TRACE("NetworkDetector is starting...");
                bool started;
                {
                    std::unique_lock<std::mutex> lock(m_lock);
                    cv.wait(lock, [this]()
                            { return startupState != StartupState::Starting; });
                    started = startupState == StartupState::Ready;
                    LOG_TRACE(
                        "NetworkDetector tid=%p running=%u",
                        m_listener_tid,
                        started);
                }

                if (!started && netDetectThread.joinable())
                {
                    netDetectThread.join();
                }
                if (!started)
                {
                    std::lock_guard<std::mutex> lock(m_lock);
                    CloseHandle(stopEvent);
                    stopEvent = nullptr;
                    networkStatusCallbackState.reset();
                    eventDispatchState->StopAndWait();
                    eventDispatchState.reset();
                }
                return started;
            }
        };

        /// <summary>
        /// Stop network monitoring thread
        /// </summary>
        void NetworkDetector::Stop()
        {
            std::lock_guard<std::mutex> lifecycleLock(m_lifecycleLock);
            if (netDetectThread.joinable())
            {
                {
                    std::lock_guard<std::mutex> lock(m_lock);
                    stopRequested = true;
                    if (networkStatusCallbackState != nullptr)
                    {
                        networkStatusCallbackState->listenerThreadId.store(0, std::memory_order_release);
                    }
                    if (!SetEvent(stopEvent))
                    {
                        LOG_ERROR("Unable to signal the network detector stop event.");
                    }
                }

                netDetectThread.join();
                eventDispatchState->StopAndWait();

                std::lock_guard<std::mutex> lock(m_lock);
                CloseHandle(stopEvent);
                stopEvent = nullptr;
                startupState = StartupState::Stopped;
                stopRequested = false;
                networkStatusCallbackState.reset();
                eventDispatchState.reset();
                LOG_TRACE("NetworkDetector tid=%p has stopped.", m_listener_tid);
            }
        };

        /// <summary>
        /// NetworkDetector destructor, performs Stop() if running
        /// </summary>
        /// <returns></returns>
        NetworkDetector::~NetworkDetector()
        {
            LOG_TRACE("NetworkDetector dtor tid=%p", m_listener_tid);
            Stop();
            LOG_TRACE("NetworkDetector done tid=%p", m_listener_tid);
        }

    }  // ::Windows

}
MAT_NS_END

#endif
