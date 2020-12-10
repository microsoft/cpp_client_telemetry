//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef NETWORKDETECTOR_HPP
#define NETWORKDETECTOR_HPP
#include "mat/config.h"
#ifdef HAVE_MAT_NETDETECT

#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>

//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN 1
//#endif

#pragma once

#include <Windows.h>

#include <wrl.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.networking.h>
#include <windows.networking.connectivity.h>

// ATL exceptions expose header incompatibilities with Edge's build, using libc++
#ifndef _ATL_NO_EXCEPTIONS
#define _ATL_NO_EXCEPTIONS
#endif

#include <atlbase.h>

#include <Netlistmgr.h>
#include <OCIdl.h>
#include <oaidl.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include <excpt.h>
#include <thread>
#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "Enums.hpp"

// #include <Wininet.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Networking;
using namespace ABI::Windows::Networking::Connectivity;

using namespace std;

namespace MAT_NS_BEGIN
{
            namespace Windows {

                /// <summary>
                /// Host name information structure
                /// </summary>
                struct HostNameInfo {
                    GUID            adapterId;
                    unsigned char   prefixLength;
                    std::string     address;
                };

                /// <summary>
                /// Convert HString to std::string
                /// </summary>
                /// <param name="name"></param>
                /// <returns></returns>
                std::string to_string(HString *name);

                /// <summary>
                /// Convert GUID to std::string
                /// </summary>
                /// <param name="guid"></param>
                /// <returns></returns>
                std::string to_string(GUID guid);

                class NetworkDetector: public INetworkEvents, INetworkConnectionEvents, INetworkListManagerEvents {

                private:

                    /// <summary>
                    /// Current network info stats
                    /// </summary>
                    ComPtr<INetworkInformationStatics>  networkInfoStats;


                    /// <summary>
                    /// Current connection profile
                    /// </summary>
                    ComPtr<IConnectionProfile>          m_connection_profile;

                    /// <summary>
                    /// COM INetworkListManager
                    /// </summary>
                    INetworkListManager*                pNlm;

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
                    bool                                isRunning = false;
                    bool                                isCoInitialized = false;
                    std::thread                         netDetectThread;

                    /// <summary>
                    /// 
                    /// </summary>
                    void run();

                    ComPtr<IUnknown>                    pSink;
                    ComPtr<IConnectionPointContainer>   pCpc;
                    ComPtr<IConnectionPoint>            m_pc1, m_pc2, m_pc3;

                    ULONG                               m_lRef;

                    DWORD                               m_dwCookie_INetworkEvents;
                    DWORD                               m_dwCookie_INetworkConnectionEvents;
                    DWORD                               m_dwCookie_INetworkListManagerEvents;
                    DWORD                               m_listener_tid = 0;

                    NLM_CONNECTIVITY                    m_connectivity;

                    /// <summary>
                    /// Register and listen to network state notifications
                    /// </summary>
                    /// <returns></returns>
                    bool RegisterAndListen();

                    /// <summary>
                    /// Reset network state listener to uninitialized state
                    /// </summary>
                    void Reset();

                    std::vector<std::string>            m_networks;
                    std::map<std::string, NLM_CONNECTIVITY> m_networks_connectivity;
                    std::map<std::string, NLM_CONNECTIVITY> m_connections_connectivity;
                    std::vector<HostNameInfo>           m_hostnames;
                    int                                 m_currentNetworkCost;
                    std::atomic<NetworkType>            m_currentNetworkType;

                public:

                    /// <summary>
                    /// 
                    /// </summary>
                    bool isUp() { return isRunning; };

                    /// <summary>
                    /// Createa network status listener
                    /// </summary>
                    NetworkDetector() :
                        pNlm(nullptr),
                        networkInfoStats(nullptr),
                        m_connection_profile(nullptr),
                        pSink(nullptr),
                        pCpc(nullptr),
                        m_pc1(nullptr),
                        m_pc2(nullptr),
                        m_pc3(nullptr),
                        isRunning(false),
                        isCoInitialized(false),
                        m_dwCookie_INetworkEvents(0),
                        m_dwCookie_INetworkConnectionEvents(0),
                        m_dwCookie_INetworkListManagerEvents(0),
                        m_currentNetworkCost(0),
                        m_currentNetworkType(NetworkType_Unknown),
                        m_listener_tid(0)
                    {};

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
                    virtual ~NetworkDetector();

                    /// <summary>
                    /// 
                    /// </summary>
                    /// <param name="type"></param>
                    /// <returns></returns>
                    const char *GetNetworkCostName(NetworkCostType type);

                    /// <summary>
                    /// Get current network cost
                    /// </summary>
                    /// <returns></returns>
                    int GetCurrentNetworkCost();

                    /// <summary>
                    /// Get last cached network cost
                    /// </summary>
                    /// <returns></returns>
                    NetworkCost const& GetNetworkCost();

                    /// <summary>
                    /// Get last cached network type
                    /// </summary>
                    /// <returns></returns>
                    NetworkType GetNetworkType();

                    /// <summary>
                    /// Get adapter ID for connection profile
                    /// </summary>
                    /// <param name="profile"></param>
                    /// <returns></returns>
                    std::string GetAdapterId(IConnectionProfile *profile);

                    int GetConnectivity();

                    const std::map<std::string, NLM_CONNECTIVITY>& GetNetworksConnectivity();
                    const std::map<std::string, NLM_CONNECTIVITY>& GetConnectionsConnectivity();

                    void GetNetworkDetails();

                    IConnectionProfile* GetCurrentConnectionProfile()
                    {
                        return m_connection_profile.Get();
                    }

                public:

                    // Inherited via INetworkListManagerEvents
                    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) noexcept override;
                    virtual ULONG   STDMETHODCALLTYPE AddRef(void) noexcept override;
                    virtual ULONG   STDMETHODCALLTYPE Release(void) noexcept override;
                    virtual HRESULT STDMETHODCALLTYPE ConnectivityChanged(NLM_CONNECTIVITY newConnectivity) override;

                    // Inherited via INetworkEvents
                    virtual HRESULT STDMETHODCALLTYPE NetworkAdded(GUID networkId) override;
                    virtual HRESULT STDMETHODCALLTYPE NetworkDeleted(GUID networkId) override;
                    virtual HRESULT STDMETHODCALLTYPE NetworkConnectivityChanged(GUID networkId, NLM_CONNECTIVITY newConnectivity) override;
                    virtual HRESULT STDMETHODCALLTYPE NetworkPropertyChanged(GUID networkId, NLM_NETWORK_PROPERTY_CHANGE flags) override;

                    // Inherited via INetworkConnectionEvents
                    virtual HRESULT STDMETHODCALLTYPE NetworkConnectionConnectivityChanged(GUID connectionId, NLM_CONNECTIVITY newConnectivity) override;
                    virtual HRESULT STDMETHODCALLTYPE NetworkConnectionPropertyChanged(GUID connectionId, NLM_CONNECTION_PROPERTY_CHANGE flags) override;
                };

            }
} MAT_NS_END

namespace MATW = MAT::Windows;

#endif

#endif
