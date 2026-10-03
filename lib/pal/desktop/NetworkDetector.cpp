//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"
#include "mat/config.h"
#ifdef HAVE_MAT_NETDETECT

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ole32.lib")

#include <winsock2.h>
#include <ws2ipdef.h>
#include "NetworkDetector.hpp"
#include <iphlpapi.h>
#include <wrl/implements.h>
#include <exception>

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
            mutable std::mutex mutex;
            DWORD listenerThreadId = 0;

            void SetListenerThreadId(DWORD threadId)
            {
                std::lock_guard<std::mutex> lock(mutex);
                listenerThreadId = threadId;
            }

            bool QueueRefresh() const
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (listenerThreadId == 0)
                {
                    return false;
                }
                if (!PostThreadMessage(listenerThreadId, NETDETECTOR_REFRESH, 0, NULL))
                {
                    LOG_ERROR("Unable to post a network cost refresh: %lu.", GetLastError());
                    return false;
                }
                return true;
            }
        };

        struct NetworkDetector::EventDispatchState : std::enable_shared_from_this<NetworkDetector::EventDispatchState>
        {
            ~EventDispatchState()
            {
                if (work != nullptr)
                {
                    CloseThreadpoolWork(work);
                }
            }

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

                if (work == nullptr)
                {
                    work = CreateThreadpoolWork(DispatchPendingEvents, this, nullptr);
                    if (work == nullptr)
                    {
                        LOG_ERROR("Unable to create network callback work: %lu.", GetLastError());
                        return false;
                    }
                }
                workerScheduled = true;
                SubmitThreadpoolWork(work);
                return true;
            }

            void Stop()
            {
                std::lock_guard<std::mutex> lock(mutex);
                acceptEvents = false;
                eventPending = false;
            }

            void Wait()
            {
                if (currentNetworkEventDispatch == this)
                {
                    return;
                }
                if (work != nullptr)
                {
                    WaitForThreadpoolWorkCallbacks(work, FALSE);
                }
            }

           private:
            static void CALLBACK DispatchPendingEvents(PTP_CALLBACK_INSTANCE instance, void* context, PTP_WORK)
            {
                std::shared_ptr<EventDispatchState> state =
                    static_cast<EventDispatchState*>(context)->shared_from_this();
                HMODULE module = nullptr;
                const auto address = reinterpret_cast<LPCWSTR>(&DispatchPendingEvents);
                if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                       address, &module) ||
                    (module != GetModuleHandleW(nullptr) &&
                     !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, address, &module)))
                {
                    LOG_ERROR("Unable to retain the network callback module: %lu.", GetLastError());
                    std::lock_guard<std::mutex> lock(state->mutex);
                    state->workerScheduled = false;
                    state->eventPending = false;
                    return;
                }
                if (module != GetModuleHandleW(nullptr))
                {
                    // Reentrant Stop can return on this callback; release the DLL reference after return.
                    FreeLibraryWhenCallbackReturns(instance, module);
                }
                currentNetworkEventDispatch = state.get();

                while (true)
                {
                    NetworkCost cost;
                    {
                        std::lock_guard<std::mutex> lock(state->mutex);
                        if (!state->acceptEvents || !state->eventPending)
                        {
                            state->workerScheduled = false;
                            currentNetworkEventDispatch = nullptr;
                            return;
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
            PTP_WORK work = nullptr;
            NetworkCost latestCost = NetworkCost_Unknown;
            bool acceptEvents = true;
            bool eventPending = false;
            bool workerScheduled = false;
        };

        NetworkCost MapNetworkCost(
            NL_NETWORK_CONNECTIVITY_COST_HINT costType,
            bool roaming,
            bool overDataLimit,
            bool approachingDataLimit)
        {
            if (roaming || overDataLimit || approachingDataLimit)
            {
                return NetworkCost_Roaming;
            }

            switch (costType)
            {
            case NetworkConnectivityCostHintUnrestricted:
                return NetworkCost_Unmetered;
            case NetworkConnectivityCostHintFixed:
            case NetworkConnectivityCostHintVariable:
                return NetworkCost_Metered;
            case NetworkConnectivityCostHintUnknown:
            default:
                return NetworkCost_Unknown;
            }
        }

        NetworkCost MapLegacyNetworkCost(DWORD cost)
        {
            if ((cost & (NLM_CONNECTION_COST_ROAMING | NLM_CONNECTION_COST_OVERDATALIMIT |
                         NLM_CONNECTION_COST_APPROACHINGDATALIMIT | NLM_CONNECTION_COST_CONGESTED)) != 0)
            {
                return NetworkCost_Roaming;
            }
            if ((cost & NLM_CONNECTION_COST_UNRESTRICTED) != 0)
            {
                return NetworkCost_Unmetered;
            }
            if ((cost & (NLM_CONNECTION_COST_FIXED | NLM_CONNECTION_COST_VARIABLE)) != 0)
            {
                return NetworkCost_Metered;
            }
            return NetworkCost_Unknown;
        }

        NetworkDetector::NetworkDetector()
        {
            const auto module = GetModuleHandleW(L"iphlpapi.dll");
            getConnectivityHint = reinterpret_cast<GetConnectivityHint>(
                GetProcAddress(module, "GetNetworkConnectivityHint"));
            notifyConnectivityHint = reinterpret_cast<NotifyConnectivityHint>(
                GetProcAddress(module, "NotifyNetworkConnectivityHintChange"));
            if (getConnectivityHint == nullptr || notifyConnectivityHint == nullptr)
            {
                getConnectivityHint = nullptr;
                notifyConnectivityHint = nullptr;
            }
        }

        NetworkCost NetworkDetector::QueryNetworkCost()
        {
            if (getConnectivityHint != nullptr)
            {
                NL_NETWORK_CONNECTIVITY_HINT hint {};
                const auto error = getConnectivityHint(&hint);
                if (error != NO_ERROR)
                {
                    LOG_ERROR("Unable to query network connectivity cost: %lu.", error);
                    return NetworkCost_Unknown;
                }
                return MapNetworkCost(hint.ConnectivityCost, hint.Roaming != FALSE,
                                      hint.OverDataLimit != FALSE, hint.ApproachingDataLimit != FALSE);
            }
            DWORD cost = NLM_CONNECTION_COST_UNKNOWN;
            const auto hr = networkCostManager->GetCost(&cost, nullptr);
            if (FAILED(hr))
            {
                LOG_ERROR("Unable to query legacy network cost: 0x%08lx.", hr);
                return NetworkCost_Unknown;
            }
            return MapLegacyNetworkCost(cost);
        }

        struct NetworkDetector::NetworkStatusChangedSink :
            RuntimeClass<RuntimeClassFlags<ClassicCom>, INetworkCostManagerEvents, INetworkListManagerEvents>
        {
            explicit NetworkStatusChangedSink(std::shared_ptr<CallbackState> state) : state(std::move(state))
            {
            }
            HRESULT STDMETHODCALLTYPE CostChanged(DWORD, NLM_SOCKADDR*) override
            {
                state->QueueRefresh();
                return S_OK;
            }
            HRESULT STDMETHODCALLTYPE DataPlanStatusChanged(NLM_SOCKADDR*) override
            {
                state->QueueRefresh();
                return S_OK;
            }
            HRESULT STDMETHODCALLTYPE ConnectivityChanged(NLM_CONNECTIVITY) override
            {
                state->QueueRefresh();
                return S_OK;
            }
            std::shared_ptr<CallbackState> state;
        };

        void WINAPI NetworkDetector::NetworkHintChanged(void* context, NL_NETWORK_CONNECTIVITY_HINT)
        {
            static_cast<CallbackState*>(context)->QueueRefresh();
        }
        NetworkCost NetworkDetector::GetNetworkCost()
        {
            return m_currentNetworkCost->load(std::memory_order_relaxed);
        }

        int NetworkDetector::GetCurrentNetworkCost()
        {
            {
                std::unique_lock<std::mutex> lock(m_lock);
                if (m_listener_tid != GetCurrentThreadId())
                {
                    if (startupState != StartupState::Ready || stopRequested)
                    {
                        return GetNetworkCost();
                    }
                    const auto sequence = refreshSequence;
                    const auto callbackState = networkStatusCallbackState;
                    if (!callbackState->QueueRefresh())
                    {
                        LOG_ERROR("Unable to request a network cost refresh.");
                        return GetNetworkCost();
                    }
                    cv.wait(lock, [this, sequence, callbackState]() {
                        return refreshSequence != sequence || stopRequested ||
                               startupState != StartupState::Ready ||
                               networkStatusCallbackState != callbackState;
                    });
                    return GetNetworkCost();
                }
            }
            const auto currentNetworkCost = QueryNetworkCost();
            m_currentNetworkCost->store(currentNetworkCost, std::memory_order_relaxed);
            std::shared_ptr<EventDispatchState> dispatchState;
            {
                std::lock_guard<std::mutex> lock(m_lock);
                dispatchState = eventDispatchState;
                ++refreshSequence;
                cv.notify_all();
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
        /// Initialize the network cost backend on its owning thread
        /// </summary>
        /// <returns></returns>
        bool NetworkDetector::InitializeNetworkCost()
        {
            if (getConnectivityHint != nullptr)
            {
                return true;
            }
            const auto hr = CoCreateInstance(CLSID_NetworkListManager, nullptr, CLSCTX_INPROC_SERVER,
                                             IID_PPV_ARGS(networkCostManager.GetAddressOf()));
            if (FAILED(hr))
            {
                LOG_ERROR("Unable to initialize the legacy network cost manager: 0x%08lx.", hr);
                return false;
            }
            return true;
        }

        bool NetworkDetector::RegisterAndListen() noexcept
        {
            MSG msg;
            PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

            const auto callbackState = networkStatusCallbackState;
            callbackState->SetListenerThreadId(GetCurrentThreadId());
            if (notifyConnectivityHint != nullptr)
            {
                const auto error = notifyConnectivityHint(
                    NetworkHintChanged, callbackState.get(), FALSE, &networkStatusNotification);
                if (error != NO_ERROR)
                {
                    LOG_ERROR("Unable to subscribe to network connectivity changes: %lu.", error);
                    return false;
                }
            }
            else
            {
                auto sink = Make<NetworkStatusChangedSink>(callbackState);
                if (sink == nullptr)
                {
                    LOG_ERROR("Unable to create a legacy network status handler.");
                    return false;
                }
                auto hr = sink.As(&networkStatusChangedHandler);
                ComPtr<IConnectionPointContainer> container;
                if (SUCCEEDED(hr))
                {
                    hr = networkCostManager.As(&container);
                }
                if (SUCCEEDED(hr))
                {
                    hr = container->FindConnectionPoint(__uuidof(INetworkCostManagerEvents), &costConnectionPoint);
                }
                if (SUCCEEDED(hr))
                {
                    hr = costConnectionPoint->Advise(networkStatusChangedHandler.Get(), &costCookie);
                    costSubscribed = SUCCEEDED(hr);
                }
                if (SUCCEEDED(hr))
                {
                    hr = container->FindConnectionPoint(__uuidof(INetworkListManagerEvents), &connectivityConnectionPoint);
                }
                if (SUCCEEDED(hr))
                {
                    hr = connectivityConnectionPoint->Advise(networkStatusChangedHandler.Get(), &connectivityCookie);
                    connectivitySubscribed = SUCCEEDED(hr);
                }
                if (FAILED(hr))
                {
                    LOG_ERROR("Unable to subscribe to legacy network changes: 0x%08lx.", hr);
                    return false;
                }
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
                networkStatusCallbackState->SetListenerThreadId(0);
            }
            if (networkStatusNotification != nullptr)
            {
                const auto error = CancelMibChangeNotify2(networkStatusNotification);
                if (error != NO_ERROR)
                {
                    LOG_ERROR("Unable to cancel network connectivity notifications: %lu.", error);
                    std::terminate();
                }
                networkStatusNotification = nullptr;
            }
            if (costSubscribed)
            {
                const auto hr = costConnectionPoint->Unadvise(costCookie);
                if (FAILED(hr))
                {
                    LOG_ERROR("Unable to unsubscribe from legacy network cost changes: 0x%08lx.", hr);
                }
                costSubscribed = false;
            }
            if (connectivitySubscribed)
            {
                const auto hr = connectivityConnectionPoint->Unadvise(connectivityCookie);
                if (FAILED(hr))
                {
                    LOG_ERROR("Unable to unsubscribe from legacy network connectivity changes: 0x%08lx.", hr);
                }
                connectivitySubscribed = false;
            }
            if (networkStatusChangedHandler != nullptr)
            {
                const auto hr = CoDisconnectObject(networkStatusChangedHandler.Get(), 0);
                if (FAILED(hr))
                {
                    LOG_ERROR("Unable to disconnect the legacy network handler: 0x%08lx.", hr);
                    std::terminate();
                }
            }
            networkStatusChangedHandler.Reset();
            costConnectionPoint.Reset();
            connectivityConnectionPoint.Reset();
            networkCostManager.Reset();
        }

        /// <summary>
        /// Own the network backend and notifications for the listener thread
        /// </summary>
        void NetworkDetector::run()
        {
            const bool useCom = getConnectivityHint == nullptr;
            if (useCom)
            {
                const auto hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
                if (FAILED(hr))
                {
                    LOG_ERROR("Unable to initialize the legacy network COM apartment: 0x%08lx.", hr);
                    return;
                }
            }
            struct Cleanup
            {
                NetworkDetector& detector;
                bool useCom;
                ~Cleanup()
                {
                    detector.Reset();
                    if (useCom)
                    {
                        CoUninitialize();
                    }
                }
            } cleanup { *this, useCom };
            if (InitializeNetworkCost())
            {
                m_currentNetworkCost->store(QueryNetworkCost(), std::memory_order_relaxed);
                LOG_TRACE("start listening to events...");
                RegisterAndListen();
            }
        }
        /// <summary>
        /// Start network monitoring thread
        /// </summary>
        /// <returns>true - if start is successful, false - otherwise</returns>
        bool NetworkDetector::Start()
        {
            std::unique_lock<std::mutex> lifecycleLock(m_lifecycleLock);
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

                while (eventDispatchState != nullptr)
                {
                    const auto previousDispatch = eventDispatchState;
                    lock.unlock();
                    lifecycleLock.unlock();
                    previousDispatch->Wait();
                    lifecycleLock.lock();
                    lock.lock();
                    if (startupState == StartupState::Ready)
                    {
                        return true;
                    }
                    if (eventDispatchState == previousDispatch)
                    {
                        break;
                    }
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
                    std::shared_ptr<EventDispatchState> dispatchState;
                    {
                        std::lock_guard<std::mutex> lock(m_lock);
                        CloseHandle(stopEvent);
                        stopEvent = nullptr;
                        networkStatusCallbackState.reset();
                        dispatchState = std::move(eventDispatchState);
                    }
                    dispatchState->Stop();
                    lifecycleLock.unlock();
                    dispatchState->Wait();
                }
                return started;
            }
        };

        /// <summary>
        /// Stop network monitoring thread
        /// </summary>
        void NetworkDetector::Stop()
        {
            std::unique_lock<std::mutex> lifecycleLock(m_lifecycleLock);
            if (netDetectThread.joinable())
            {
                {
                    std::lock_guard<std::mutex> lock(m_lock);
                    stopRequested = true;
                    cv.notify_all();
                    eventDispatchState->Stop();
                    if (networkStatusCallbackState != nullptr)
                    {
                        networkStatusCallbackState->SetListenerThreadId(0);
                    }
                    if (!SetEvent(stopEvent))
                    {
                        LOG_ERROR("Unable to signal the network detector stop event.");
                    }
                }

                netDetectThread.join();
                {
                    std::lock_guard<std::mutex> lock(m_lock);
                    CloseHandle(stopEvent);
                    stopEvent = nullptr;
                    startupState = StartupState::Stopped;
                    stopRequested = false;
                    networkStatusCallbackState.reset();
                }
            }
            const auto dispatchState = eventDispatchState;
            lifecycleLock.unlock();
            if (dispatchState != nullptr)
            {
                dispatchState->Stop();
                dispatchState->Wait();
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
