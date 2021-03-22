//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "ctmacros.hpp"
#include "mat/config.h"
#ifdef HAVE_MAT_NETDETECT

#pragma comment(lib, "runtimeobject.lib")

// This macro is required for DEFINE_GUID below to declare a local instance of IID_INetworkCostManager GUID
#define INITGUID

#include "NetworkDetector.hpp"
#include <algorithm>

#include "ILogManager.hpp"
#include "DebugEvents.hpp"
#include "utils/Utils.hpp"
#include "pal/PAL.hpp"

// Define a GUID that is only available in Windows 8.x+ SDK . We are using Windows 7.1A SDK for Win32 SDK build,
// so we cannot easily add an extra dependency on Windows 8 or later functionality project-wide. It'd be error-prone,
// because when we have all Windows 8+ features - we might fall into temptation of using that features that would
// break Windows 7.1 compatibility. We cannot afford breaking Windows 7.1 compatibility at this time.
DEFINE_GUID(IID_INetworkCostManager2, 0xdcb00008, 0x570f, 0x4a9b, 0x8d, 0x69, 0x19, 0x9f, 0xdb, 0xa5, 0x72, 0x3b);

#define NETDETECTOR_START           WM_USER+1
#define NETDETECTOR_STOP            WM_USER+2

#define NETDETECTOR_COM_SETTLE_MS   1000

namespace MAT_NS_BEGIN
{
    namespace Windows {

        // Malwarebytes have been detected
        static bool mbDetected = false;

        /// <summary>
        /// Convert HString to std::string
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        std::string to_string(HString *name)
        {
            UINT32 length;
            PCWSTR rawString = name->GetRawBuffer(&length);
            std::wstring wide(rawString);
            return to_utf8_string(wide);
        }

        /// <summary>
        /// Convert GUID to std::string
        /// </summary>
        /// <param name="guid"></param>
        /// <returns></returns>
        std::string to_string(GUID guid) {
            std::string result;
            char buff[40] = { 0 }; // Maximum hyphenated GUID length with braces is 38 + null terminator
            sprintf_s(buff, sizeof(buff),
                "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
                guid.Data1, guid.Data2, guid.Data3,
                guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
                guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
            result = buff;
            return result;
        }

        NetworkCost const& NetworkDetector::GetNetworkCost() {
            return (NetworkCost const &)m_currentNetworkCost;
        }

        NetworkType NetworkDetector::GetNetworkType()
        {
            return m_currentNetworkType.load();
        }

        /// <summary>
        /// Get current realtime network cost synchronously.
        /// This function can be called on any Windows release and it provides a SEH handler.
        /// </summary>
        /// <returns></returns>
#pragma warning(push)
#pragma warning(disable: 6320)
        int NetworkDetector::GetCurrentNetworkCost()
        {
#if 0
            // We don't know the cost of something that is not there
            if (m_connectivity == NLM_CONNECTIVITY_DISCONNECTED)
            {
                TRACE("Disconnected!");
                m_currentNetworkCost = NetworkCost_Unknown;
            }
#endif
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
            evt.param2 = mbDetected;
            ILogManager::DispatchEventBroadcast(evt);

            return m_currentNetworkCost;
        }
#pragma warning(pop)

        /// <summary>
        /// Get current network connectivity state
        /// </summary>
        /// <returns>Value of enum NLM_CONNECTIVITY</returns>
        int NetworkDetector::GetConnectivity()
        {
            return m_connectivity;
        }

        /// <summary>
        /// Internal implementation
        /// </summary>
        /// <returns></returns>
        NetworkCost NetworkDetector::_GetCurrentNetworkCost()
        {
            NetworkCost result = NetworkCost_Unknown;
            LOG_TRACE("get network cost...\n");

            if (pNlm == NULL) {
                LOG_WARN("INetworkCostManager is unavailable!");
                return result;
            }

            HRESULT hr;

            DWORD dwCost = NLM_CONNECTION_COST_UNKNOWN;
            INetworkCostManager* pNetworkCostManager = NULL;

            hr = pNlm->QueryInterface(IID_INetworkCostManager2, (void**)&pNetworkCostManager);
            if (hr != S_OK) {
                return result;
            }

            hr = pNetworkCostManager->GetCost(&dwCost, NULL);
            if (hr == S_OK) {
                switch (dwCost) {
                case NLM_CONNECTION_COST_UNRESTRICTED:  // The connection is unlimited and is considered to be unrestricted of usage charges and capacity constraints.
                    result = NetworkCost_Unmetered;
                    break;
                case NLM_CONNECTION_COST_FIXED:         // The use of this connection is unrestricted up to a specific data transfer limit.
                case NLM_CONNECTION_COST_VARIABLE:      // This connection is regulated on a per byte basis.
                    result = NetworkCost_Metered;
                    break;
                case NLM_CONNECTION_COST_OVERDATALIMIT: // The connection is currently in an OverDataLimit state as it has exceeded the carrier specified data transfer limit.
                case NLM_CONNECTION_COST_CONGESTED:     // The network is experiencing high traffic load and is congested.
                case NLM_CONNECTION_COST_ROAMING:       // The connection is roaming outside the network and affiliates of the home provider.
                case NLM_CONNECTION_COST_APPROACHINGDATALIMIT:  // The connection is approaching the data limit specified by the carrier.
                    result = NetworkCost_Roaming;
                    break;
                case NLM_CONNECTION_COST_UNKNOWN:
                default:
                    result = NetworkCost_Unknown;       // The cost is unknown.
                    break;
                }
            }

            return result;
}

        /// <summary>
        /// Get adapter id for IConnectionProfile
        /// </summary>
        /// <param name="profile"></param>
        /// <returns></returns>
        std::string NetworkDetector::GetAdapterId(IConnectionProfile *profile)
        {
            if (!profile)
            {
                LOG_ERROR("Invalid profile pointer!");
                return "";  // Invalid interface ptr
            }

#if 0 /* FIXME: do we return none if connectivity level is none? */
            NetworkConnectivityLevel connectivityLevel;
            HRESULT hr = profile->GetNetworkConnectivityLevel(&connectivityLevel);
            if (connectivityLevel != NetworkConnectivityLevel_None)
            {

            }
#endif

            ComPtr<INetworkAdapter> adapter;
            HRESULT hr = profile->get_NetworkAdapter(&adapter);
            if (hr == E_INVALIDARG)
            {
                // No interfaces - device is in airplane mode
                LOG_TRACE("No network interfaces - device is in airplane mode");
                return "";
            }

            GUID id;
            hr = adapter->get_NetworkAdapterId(&id);
            if (!SUCCEEDED(hr))
            {
                // Unable to obtain Network Adapter GUID
                LOG_TRACE("Unable to obtain interface GUID");
                return "";
            }

            return to_string(id);
        }

        /// <summary>
        /// COM thread interfaces supported by this class
        /// </summary>
        /// <param name="riid"></param>
        /// <param name="ppv"></param>
        /// <returns></returns>
        HRESULT NetworkDetector::QueryInterface(REFIID riid, void ** ppv) noexcept
        {
            static const QITAB rgqit[] =
            {
                QITABENT(NetworkDetector, INetworkEvents),
                QITABENT(NetworkDetector, INetworkConnectionEvents),
                QITABENT(NetworkDetector, INetworkListManagerEvents),
                { nullptr,0 }
            };
            return QISearch(this, rgqit, riid, ppv);
        }

        ULONG NetworkDetector::AddRef(void) noexcept
        {
            return InterlockedIncrement((LONG *)&m_lRef);
        }

        ULONG NetworkDetector::Release(void) noexcept
        {
            ULONG ulNewRef = (ULONG)InterlockedDecrement((LONG *)&m_lRef);
            if (ulNewRef == 0)
            {
                // NetworkDetector is destroyed from FlushAndTeardown.
                // If customer forgets to call it, then it is destroyed from atexit(...)
                LOG_TRACE("NetworkDetector last instance released (this=%p)", this);
            }
            return ulNewRef;
        }

        HRESULT NetworkDetector::ConnectivityChanged(NLM_CONNECTIVITY newConnectivity)
        {
            LOG_TRACE("Connectivity changed: %d", newConnectivity);
            m_connectivity = newConnectivity;
            GetCurrentNetworkCost();
            return RPC_S_OK;
        }

        HRESULT NetworkDetector::NetworkAdded(GUID networkId)
        {
            LOG_TRACE("NetworkAdded: %s", to_string(networkId).c_str());
            m_networks.push_back(to_string(networkId));
            return RPC_S_OK;
        }

        HRESULT NetworkDetector::NetworkDeleted(GUID networkId)
        {
            LOG_TRACE("NetworkDeleted: %s", to_string(networkId).c_str());
            auto &v = m_networks;
            const std::string &item = to_string(networkId);
            v.erase(std::remove(v.begin(), v.end(), item), v.end());
            return RPC_S_OK;
        }

        HRESULT NetworkDetector::NetworkConnectivityChanged(GUID networkId, NLM_CONNECTIVITY newConnectivity)
        {
            LOG_TRACE("NetworkConnectivityChanged: %s, %d", to_string(networkId).c_str(), newConnectivity);
            m_networks_connectivity[to_string(networkId)] = newConnectivity;
            return RPC_S_OK;
        }

        HRESULT NetworkDetector::NetworkPropertyChanged(GUID networkId, NLM_NETWORK_PROPERTY_CHANGE flags)
        {
            UNREFERENCED_PARAMETER(networkId);
            UNREFERENCED_PARAMETER(flags);
            LOG_TRACE("NetworkPropertyChanged: %s, %d", to_string(networkId).c_str(), flags);
            GetCurrentNetworkCost();
            return RPC_S_OK;
        }

        HRESULT NetworkDetector::NetworkConnectionConnectivityChanged(GUID connectionId, NLM_CONNECTIVITY newConnectivity)
        {
            LOG_TRACE("NetworkConnectionConnectivityChanged: %s, %d", to_string(connectionId).c_str(), newConnectivity);
            m_connections_connectivity[to_string(connectionId)] = newConnectivity;
            return RPC_S_OK;
        }

        HRESULT NetworkDetector::NetworkConnectionPropertyChanged(GUID connectionId, NLM_CONNECTION_PROPERTY_CHANGE flags)
        {
            UNREFERENCED_PARAMETER(connectionId);
            UNREFERENCED_PARAMETER(flags);
            LOG_TRACE("NetworkConnectionPropertyChanged: %s, %d", to_string(connectionId).c_str(), flags);
            return RPC_S_OK;
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

        bool NetworkDetector::RegisterAndListen()
        {
            // ???
            HRESULT hr = pNlm->QueryInterface(IID_IUnknown, (void**)&pSink);
            if (FAILED(hr))
            {
                LOG_ERROR("cannot query IID_IUnknown!!!");
                return false;
            }

            pSink = (INetworkEvents*)this;

            hr = pNlm->QueryInterface(IID_IConnectionPointContainer, (void**)&pCpc);
            if (FAILED(hr))
            {
                LOG_ERROR("Unable to QueryInterface IID_IConnectionPointContainer!");
                return false;
            }

            hr = pCpc->FindConnectionPoint(IID_INetworkConnectionEvents, &m_pc1);
            if (SUCCEEDED(hr))
            {
                hr = m_pc1->Advise(
                    pSink.Get(),
                    &m_dwCookie_INetworkConnectionEvents);
                LOG_INFO("listening to INetworkConnectionEvents... %s",
                    (SUCCEEDED(hr)) ? "OK" : "FAILED");
            }

            hr = pCpc->FindConnectionPoint(IID_INetworkEvents, &m_pc2);
            if (SUCCEEDED(hr))
            {
                hr = m_pc2->Advise(
                    pSink.Get(),
                    &m_dwCookie_INetworkEvents);
                LOG_INFO("listening to INetworkEvents... %s",
                    (SUCCEEDED(hr)) ? "OK" : "FAILED");
            }

            hr = pCpc->FindConnectionPoint(IID_INetworkListManagerEvents, &m_pc3);
            if (SUCCEEDED(hr))
            {
                hr = m_pc3->Advise(
                    pSink.Get(),
                    &m_dwCookie_INetworkListManagerEvents);
                LOG_INFO("listening to INetworkListManagerEvents... %s",
                    (SUCCEEDED(hr)) ? "OK" : "FAILED");
            }

            MSG msg;
            PostThreadMessage(m_listener_tid, NETDETECTOR_START, 0, 0);
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
            if (m_pc1 != nullptr)
            {
                m_pc1->Unadvise(m_dwCookie_INetworkConnectionEvents);
                m_pc1 = nullptr;
            }

            if (m_pc2 != nullptr)
            {
                m_pc2->Unadvise(m_dwCookie_INetworkEvents);
                m_pc2 = nullptr;
            }

            if (m_pc3 != nullptr)
            {
                m_pc3->Unadvise(m_dwCookie_INetworkListManagerEvents);
                m_pc3 = nullptr;
            }

            m_connection_profile.Reset();
            pSink.Reset();
            pCpc.Reset();

            networkInfoStats.Reset();
            if (pNlm != nullptr)
            {
                LOG_TRACE("release network list manager...");
                pNlm->Release();
                pNlm = nullptr;
            };
        }

        /// <summary>
        /// Register for COM events and block-wait in RegisterAndListen
        /// </summary>
#pragma warning( push )
#pragma warning(disable:28159)
#pragma warning(disable:4996)
#pragma warning(disable:6320)
// We must use GetVersionEx to retain backwards compat with Win 7 SP1
        void NetworkDetector::run()
        {
            // Check Windows version and if below Windows 8, then avoid running Network cost detection logic
            OSVERSIONINFO osvi;
            BOOL bIsWindows8orLater;
            ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&osvi);
            bIsWindows8orLater = ((osvi.dwMajorVersion >= 6) && (osvi.dwMinorVersion >= 2)) || (osvi.dwMajorVersion > 6);
            // Applications not manifested for Windows 8.1 or Windows 10 will return the Windows 8 OS version value (6.2)
            if (!bIsWindows8orLater)
            {
                LOG_INFO("Running on Windows %d.%d without network detector...", osvi.dwMajorVersion, osvi.dwMinorVersion);
                return;
            }

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
                    LOG_INFO("create network list manager...");
                    hr = CoCreateInstance(
                        CLSID_NetworkListManager,
                        nullptr,
                        CLSCTX_ALL,
                        IID_INetworkListManager,
                        (void**)&pNlm);
                    if (FAILED(hr))
                    {
                        LOG_ERROR("Unable to CoCreateInstance for CLSID_NetworkListManager!");
                    }
                    else
                    {
                        GetCurrentNetworkCost();
                        LOG_TRACE("start listening to events...");
                        RegisterAndListen(); // we block here to process COM events
                    }
                    // Once we are done OR cannot init NLM, we must perform the clean-up
                    Reset();
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                LOG_ERROR("Handled exception in network cost detection (Windows 7?)");
            }

            if (isCoInitialized)
            {
                CoUninitialize();
                isCoInitialized = false;
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
                    // Wait for up to NETDETECTOR_COM_SETTLE_MS ms until:
                    // - COM object is ready; OR
                    // - COM object can't be started (pre-Win 8 scenario)
                    int retry = 1;
                    constexpr int max_retries = 2;
                    while (isRunning && cv.wait_for(lock, std::chrono::milliseconds(NETDETECTOR_COM_SETTLE_MS))
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

        /// <summary>
        /// Get network cost name
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        const char* NetworkDetector::GetNetworkCostName(NetworkCostType type)
        {
            switch (type) {
            case NetworkCostType_Unrestricted:
                return "Unrestricted";
            case NetworkCostType_Fixed:
                return "Fixed";
            case NetworkCostType_Variable:
                return "Variable";
            case NetworkCostType_Unknown:
            default:
                return "Unknown";
            }
        }

        const std::map<std::string, NLM_CONNECTIVITY>& NetworkDetector::GetNetworksConnectivity()
        {
            return m_networks_connectivity;
        }

        const std::map<std::string, NLM_CONNECTIVITY>& NetworkDetector::GetConnectionsConnectivity()
        {
            return m_connections_connectivity;
        }

        /// <summary>
        /// Obtain various details about network stack
        /// </summary>
        void NetworkDetector::GetNetworkDetails()
        {
            LOG_TRACE("Getting network details...");
            ComPtr<IVectorView<HostName *>> hostNames;
            HRESULT hr = networkInfoStats->GetHostNames(&hostNames);
            if ((!SUCCEEDED(hr))||(!hostNames))
                return;

            m_hostnames.clear();
            unsigned int hostNameCount;
            hr = hostNames->get_Size(&hostNameCount);
            if (!SUCCEEDED(hr))
                return;
            for (unsigned i = 0; i < hostNameCount; ++i) {
                MATW::HostNameInfo hostInfo;
                ComPtr<IHostName> hostName;
                hr = hostNames->GetAt(i, &hostName);
                if (!SUCCEEDED(hr))
                    continue;
                HString rawName;
                hostName->get_RawName(rawName.GetAddressOf());
                LOG_TRACE("RawName: %s", to_string(&rawName).c_str());

                HostNameType type;
                hr = hostName->get_Type(&type);
                if (!SUCCEEDED(hr))
                    continue;
                LOG_TRACE("HostNameType: %d", type);

                if (type == HostNameType_DomainName)
                    continue;

                ComPtr<IIPInformation> ipInformation;
                hr = hostName->get_IPInformation(&ipInformation);
                if (!SUCCEEDED(hr))
                    continue;

                ComPtr<INetworkAdapter> currentAdapter;
                hr = ipInformation->get_NetworkAdapter(&currentAdapter);
                if (!SUCCEEDED(hr))
                    continue;
                hr = currentAdapter->get_NetworkAdapterId(&hostInfo.adapterId);
                if (!SUCCEEDED(hr))
                    continue;
                LOG_TRACE("CurrentAdapterId: %s", to_string(hostInfo.adapterId).c_str());

                ComPtr<IReference<unsigned char>> prefixLengthReference;
                hr = ipInformation->get_PrefixLength(&prefixLengthReference);
                if (!SUCCEEDED(hr))
                    continue;
                hr = prefixLengthReference->get_Value(&hostInfo.prefixLength);
                if (!SUCCEEDED(hr))
                    continue;
                LOG_TRACE("PrefixLength: %d", hostInfo.prefixLength);

                // invalid prefixes
                if ((type == HostNameType_Ipv4 && hostInfo.prefixLength > 32)
                    || (type == HostNameType_Ipv6 && hostInfo.prefixLength > 128))
                    continue;

                HString name;
                hr = hostName->get_CanonicalName(name.GetAddressOf());
                if (!SUCCEEDED(hr))
                    continue;
                hostInfo.address = to_string(&name);
                LOG_TRACE("CanonicalName: %s", hostInfo.address.c_str());

                m_hostnames.push_back(hostInfo);
            }

            // hr = networkInfoStats->GetInternetConnectionProfile(&m_connection_profile);
            // auto profile0 = m_connection_profile.Get();

            ComPtr<IVectorView<ConnectionProfile *>> m_connection_profiles;
            hr = networkInfoStats->GetConnectionProfiles(&m_connection_profiles);
            if (!SUCCEEDED(hr))
                return;

            unsigned int size;
            hr = m_connection_profiles->get_Size(&size);
            if (!SUCCEEDED(hr))
                return;

            for (unsigned int i = 0; i < size; ++i) {
                ComPtr<IConnectionProfile> profile;
                hr = m_connection_profiles->GetAt(i, &profile);
                if (!SUCCEEDED(hr))
                    continue;
                auto prof = profile.Get();
                HString name;
                hr = prof->get_ProfileName(name.GetAddressOf());
                if (!SUCCEEDED(hr))
                    continue;
                LOG_TRACE("Profile[%d]: name = %s", i, to_string(&name).c_str());
                LOG_TRACE("Profile[%d]: guid = %s", i, GetAdapterId(prof).c_str());
            }
        }

    } // ::Windows

} MAT_NS_END

#endif
