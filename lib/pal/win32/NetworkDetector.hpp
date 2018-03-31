#ifndef NETWORKDETECTOR_HPP
#define NETWORKDETECTOR_HPP

#ifndef NO_ROAM_SUP

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

#include <Netlistmgr.h>
#include <OCIdl.h>
#include <atlbase.h>
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

#include "Enums.hpp"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Networking;
using namespace ABI::Windows::Networking::Connectivity;

using namespace std;

namespace ARIASDK_NS_BEGIN
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
        std::string to_string(const HString *name);

        /// <summary>
        /// Convert GUID to std::string
        /// </summary>
        /// <param name="guid"></param>
        /// <returns></returns>
        std::string to_string(GUID guid);

        class NetworkDetector : public INetworkEvents, INetworkConnectionEvents, INetworkListManagerEvents {

        private:
            /// <summary>
            /// Obtain network cost RO. This function does not handle potential exceptions and must only be called from GetNetworkCost()
            /// </summary>
            /// <returns></returns>
            NetworkCost _GetCurrentNetworkCost();

            /// <summary>
            /// runs the main message loop
            /// </summary>
            void run();

            /// <summary>
            /// Register and listen to network state notifications
            /// </summary>
            /// <returns></returns>
            bool RegisterAndListen();

            bool IsWindows8orLater();

            std::vector<std::string>            m_networks;
            std::map<std::string, NLM_CONNECTIVITY> m_networks_connectivity;
            std::map<std::string, NLM_CONNECTIVITY> m_connections_connectivity;
            std::vector<HostNameInfo>           m_hostnames;
            int                                 m_currentNetworkCost;
            ULONG                               m_lRef;
            DWORD                               m_listener_tid;
            NLM_CONNECTIVITY                    m_connectivity;
            /// COM INetworkListManager
            INetworkListManager*                pNlm;
            std::mutex                          m_lock;
            std::condition_variable             cv;
            bool                                isRunning;
            std::thread                         netDetectThread;
            ComPtr<IUnknown>                    pSink;
            HANDLE                              m_syncEvent;


        public:

            /// <summary>
            /// 
            /// </summary>
            bool isUp() const { return isRunning; };

            /// <summary>
            /// Createa network status listener
            /// </summary>
            NetworkDetector() :
                pNlm(NULL),
                isRunning(false),
                m_currentNetworkCost(0),
                m_lRef(0),
                m_syncEvent(::CreateEvent(NULL, FALSE, TRUE, NULL))
            {}

            virtual ~NetworkDetector();
            bool Start();
            void Stop();

            /// <summary>
            /// Get current network cost
            /// </summary>
            /// <returns></returns>
            int NetworkDetector::GetCurrentNetworkCost();

            /// <summary>
            /// Get last cached network cost
            /// </summary>
            /// <returns></returns>
            NetworkCost const& NetworkDetector::GetNetworkCost() const;


            int NetworkDetector::GetConnectivity() const;

            const std::map<std::string, NLM_CONNECTIVITY>& GetNetworksConnectivity() const;
            const std::map<std::string, NLM_CONNECTIVITY>& GetConnectionsConnectivity() const;

        public:

            // Inherited via INetworkListManagerEvents
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) override;
            virtual ULONG   STDMETHODCALLTYPE AddRef(void) override;
            virtual ULONG   STDMETHODCALLTYPE Release(void) override;
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
} ARIASDK_NS_END

namespace MATW = MAT::Windows;

#endif

#endif