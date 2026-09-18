//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"
#include "mat/config.h"
#ifdef HAVE_MAT_NETDETECT

#pragma comment(lib, "runtimeobject.lib")

#include "NetworkDetector.hpp"

#include "ILogManager.hpp"
#include "DebugEvents.hpp"
#include "pal/PAL.hpp"

#define NETDETECTOR_STOP                WM_USER+1
#define NETDETECTOR_START_TIMEOUT_MS    1000

namespace MAT_NS_BEGIN
{
    namespace Windows {

        NetworkCost const& NetworkDetector::GetNetworkCost() {
            return m_currentNetworkCost;
        }

        /// <summary>
        /// Get current realtime network cost synchronously.
        /// This function provides an SEH handler for Windows Runtime failures.
        /// </summary>
        /// <returns></returns>
#pragma warning(push)
#pragma warning(disable: 6320)
        int NetworkDetector::GetCurrentNetworkCost()
        {
            m_currentNetworkCost = NetworkCost_Unknown;
            __try {
                m_currentNetworkCost = _GetCurrentNetworkCost();
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
                m_currentNetworkCost = NetworkCost_Unknown;
            }

            // Notify the app about current network cost change
            DebugEvent evt;
            evt.type = DebugEventType::EVT_NET_CHANGED;
            evt.param1 = m_currentNetworkCost;
            evt.param2 = false;
            ILogManager::DispatchEventBroadcast(evt);

            return m_currentNetworkCost;
        }
#pragma warning(pop)

        /// <summary>
        /// Internal implementation
        /// </summary>
        /// <returns></returns>
        NetworkCost NetworkDetector::_GetCurrentNetworkCost()
        {
            NetworkCost result = NetworkCost_Unknown;
            LOG_TRACE("get network cost...\n");

            if (networkInfoStats == nullptr) {
                LOG_WARN("Windows network information is unavailable!");
                return result;
            }

            ComPtr<IConnectionProfile> connectionProfile;
            HRESULT hr = networkInfoStats->GetInternetConnectionProfile(&connectionProfile);
            if (FAILED(hr) || connectionProfile == nullptr) {
                return result;
            }

            ComPtr<IConnectionCost> connectionCost;
            hr = connectionProfile->GetConnectionCost(&connectionCost);
            if (FAILED(hr) || connectionCost == nullptr) {
                return result;
            }

            boolean roaming = false;
            boolean overDataLimit = false;
            NetworkCostType costType = NetworkCostType_Unknown;
            if (FAILED(connectionCost->get_Roaming(&roaming)) ||
                FAILED(connectionCost->get_OverDataLimit(&overDataLimit)) ||
                FAILED(connectionCost->get_NetworkCostType(&costType))) {
                return result;
            }

            if (roaming || overDataLimit) {
                return NetworkCost_Roaming;
            }

            switch (costType) {
            case NetworkCostType_Unrestricted:
                result = NetworkCost_Unmetered;
                break;
            case NetworkCostType_Fixed:
            case NetworkCostType_Variable:
                result = NetworkCost_Metered;
                break;
            case NetworkCostType_Unknown:
            default:
                break;
            }

            return result;
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
            networkStatusChangedHandler = Callback<INetworkStatusChangedEventHandler>(
                [this](IInspectable*) -> HRESULT {
                    GetCurrentNetworkCost();
                    return S_OK;
                });
            if (networkStatusChangedHandler == nullptr) {
                LOG_ERROR("Unable to create network status handler.");
                return false;
            }

            HRESULT hr = networkInfoStats->add_NetworkStatusChanged(
                networkStatusChangedHandler.Get(),
                &networkStatusChangedToken);
            if (FAILED(hr)) {
                LOG_ERROR("Unable to subscribe to network status changes.");
                networkStatusChangedHandler.Reset();
                return false;
            }

            MSG msg;
            PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
            cv.notify_all();

            while (GetMessage(&msg, NULL, 0, 0) > 0)
            {
                switch (msg.message)
                {
                case NETDETECTOR_STOP:
                    PostQuitMessage(0);
                    break;
                default:
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            return true;
        }

        /// <summary>
        /// 
        /// </summary>
        void NetworkDetector::Reset()
        {
            if (networkStatusChangedToken.value != 0 && networkInfoStats != nullptr)
            {
                networkInfoStats->remove_NetworkStatusChanged(networkStatusChangedToken);
                networkStatusChangedToken.value = 0;
            }
            networkStatusChangedHandler.Reset();
            networkInfoStats.Reset();
        }

        /// <summary>
        /// Register for Windows Runtime events and block-wait in RegisterAndListen
        /// </summary>
#pragma warning( push )
#pragma warning(disable:6320)
        void NetworkDetector::run()
        {
            bool isCoInitialized = false;

            __try
            {
                HRESULT hr = CoInitialize(nullptr);
                if (FAILED(hr))
                {
                    LOG_ERROR("CoInitialize Failed.");
                    return;
                }

                isCoInitialized = true;
                if (GetNetworkInfoStats())
                {
                    GetCurrentNetworkCost();
                    LOG_TRACE("start listening to events...");
                    RegisterAndListen();
                    Reset();
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                LOG_ERROR("Handled exception in Windows Runtime network cost detection.");
            }

            if (isCoInitialized)
            {
                CoUninitialize();
            }

        }
#pragma warning( pop )

        /// <summary>
        /// Start network monitoring thread
        /// </summary>
        /// <returns>true - if start is successful, false - otherwise</returns>
        bool NetworkDetector::Start()
        {
            {
                std::lock_guard<std::mutex> lk(m_lock);
                if (isRunning)
                {
                    LOG_TRACE("NetworkDetector tid=%p is already running", m_listener_tid);
                    return true;
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
                    cv.notify_all();
                }
            });

            if (netDetectThread.joinable())
            {
                LOG_TRACE("NetworkDetector is starting...");
                {
                    std::unique_lock<std::mutex> lock(m_lock);
                    // Wait for up to NETDETECTOR_START_TIMEOUT_MS ms until:
                    // - the listener is subscribed; OR
                    // - Windows Runtime network information is unavailable
                    int retry = 1;
                    constexpr int max_retries = 2;
                    while (isRunning && cv.wait_for(lock, std::chrono::milliseconds(NETDETECTOR_START_TIMEOUT_MS))
                           == std::cv_status::timeout && (retry < max_retries))
                    {
                        LOG_TRACE("NetworkDetector starting up... [%u]", retry);
                        retry++;
                    }
                    LOG_TRACE("NetworkDetector tid=%p running=%u", m_listener_tid, isRunning);
                }
            }
            else
            {
                std::lock_guard<std::mutex> lk(m_lock);
                LOG_WARN("NetworkDetector thread can't be started!");
                isRunning = false;
            }

            return isRunning;
        };

        /// <summary>
        /// Stop network monitoring thread
        /// </summary>
        void NetworkDetector::Stop()
        {
            if (netDetectThread.joinable())
            {
                std::unique_lock<std::mutex> lk(m_lock);
                try {
                    if (!isRunning || m_listener_tid == 0 ||
                        !PostThreadMessage(m_listener_tid, NETDETECTOR_STOP, 0, NULL))
                    {
                        // Without detaching, we risk throwing an exception in the destructor.
                        // There is a chance that our code has finished, but the thread
                        // hasn't fully terminated, or the thread has already exited and
                        // isRunning is false. Alternatively, we may have never gotten
                        // a thread_id.
                        netDetectThread.detach();
                        LOG_WARN("NetworkDetector thread unable to be shut down.");
                    }
                    else
                    {
                        lk.unlock();
                        netDetectThread.join();
                        LOG_TRACE("NetworkDetector tid=%p has stopped.", m_listener_tid);
                    }
                }
                catch (std::system_error &ex)
                {
                    UNREFERENCED_PARAMETER(ex);
                    LOG_WARN("NetworkDetector tid=%p is already stopped.", m_listener_tid);
                }
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

    } // ::Windows

} MAT_NS_END

#endif
