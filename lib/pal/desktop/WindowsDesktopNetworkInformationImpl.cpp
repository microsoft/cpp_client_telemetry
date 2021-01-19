//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifdef _WIN32
#define LOG_MODULE DBG_PAL
#include "pal/NetworkInformationImpl.hpp"

#include <Windows.h>
#include <Wininet.h>

#include "NetworkDetector.hpp"

using namespace MAT;

namespace PAL_NS_BEGIN {

    NetworkInformationImpl::NetworkInformationImpl(IRuntimeConfig& configuration) :
        m_cost(NetworkCost_Unmetered),
        m_info_helper(),
        m_registeredCount(0),
        m_isNetDetectEnabled(configuration[CFG_BOOL_ENABLE_NET_DETECT])
    { };
    NetworkInformationImpl::~NetworkInformationImpl() { };

    class Win32NetworkInformation : public NetworkInformationImpl
    {
#ifdef HAVE_MAT_NETDETECT
        std::unique_ptr<MATW::NetworkDetector> networkDetector;
#endif
        std::string m_network_provider;

    public:
        /// <summary>
        ///
        /// </summary>
        /// <param name="isNetDetectEnabled"></param>
        Win32NetworkInformation(IRuntimeConfig& configuration);

        /// <summary>
        ///
        /// </summary>
        virtual ~Win32NetworkInformation();

        /// <summary>
        /// Gets the current network provider for the device
        /// </summary>
        /// <returns>The current network provider for the device</returns>
        virtual std::string const& GetNetworkProvider() override
        {
            return m_network_provider;
        }

        /// <summary>
        /// Gets the current network type for the device
        /// E.g. Wifi, 3G, Ethernet
        /// </summary>
        /// <returns>The current network type for the device</returns>
        virtual NetworkType GetNetworkType() override
        {
            m_type = NetworkType_Unknown;
            return m_type;
        }

        /// <summary>
        /// Gets the current network cost for the device:
        /// OVER_DATA_LIMIT
        /// METERED
        /// UNMETERED
        /// </summary>
        /// <returns>The current network cost for the device</returns>
        virtual NetworkCost GetNetworkCost() override
        {
            m_cost = NetworkCost_Unmetered;
#ifdef HAVE_MAT_NETDETECT
            if (m_isNetDetectEnabled) {
                m_cost = networkDetector->GetNetworkCost();
            }
#endif
            return m_cost;
        }
    };

    Win32NetworkInformation::Win32NetworkInformation(IRuntimeConfig& configuration) :
                            NetworkInformationImpl(configuration)
    {
        m_type = NetworkType_Unknown;
        m_cost = NetworkCost_Unknown;
#ifdef HAVE_MAT_NETDETECT
        if (m_isNetDetectEnabled) {
            networkDetector = std::unique_ptr<MATW::NetworkDetector>(new MATW::NetworkDetector());
            networkDetector->AddRef();
            networkDetector->Start();
        }
#endif
    }

    Win32NetworkInformation::~Win32NetworkInformation()
    {
        //LOG_TRACE("Win32NetworkInformation::~Win32NetworkInformation dtor");
#ifdef HAVE_MAT_NETDETECT
        if (m_isNetDetectEnabled) {
            networkDetector->Stop();
            networkDetector->Release();
        }
#endif
    }

    std::shared_ptr<INetworkInformation> NetworkInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<Win32NetworkInformation>(configuration);
    }
} PAL_NS_END
#endif

