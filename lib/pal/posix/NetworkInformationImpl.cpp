//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#define LOG_MODULE DBG_PAL
#include "pal/PAL.hpp"
#include "pal/NetworkInformationImpl.hpp"

namespace PAL_NS_BEGIN {

    NetworkInformationImpl::NetworkInformationImpl(IRuntimeConfig& configuration) :
        m_info_helper(),
        m_isNetDetectEnabled(configuration[CFG_BOOL_ENABLE_NET_DETECT]) {};

    NetworkInformationImpl::~NetworkInformationImpl() {};

    class NetworkInformation : public NetworkInformationImpl
    {
        std::string m_network_provider;

    public:
        /// <summary>
        ///
        /// </summary>
        /// <param name="isNetDetectEnabled"></param>
        NetworkInformation(IRuntimeConfig& configuration);

        /// <summary>
        ///
        /// </summary>
        virtual ~NetworkInformation();

        /// <summary>
        /// Gets the current network provider for the device
        /// </summary>
        /// <returns>The current network provider for the device</returns>
        virtual std::string const& GetNetworkProvider()
        {
            return m_network_provider;
        }

        /// <summary>
        /// Gets the current network type for the device
        /// E.g. Wifi, 3G, Ethernet
        /// </summary>
        /// <returns>The current network type for the device</returns>
        virtual NetworkType GetNetworkType()
        {
            m_type = NetworkType_Wired;
            return m_type;
        }

        /// <summary>
        /// Gets the current network cost for the device:
        /// OVER_DATA_LIMIT
        /// METERED
        /// UNMETERED
        /// </summary>
        /// <returns>The current network cost for the device</returns>
        virtual NetworkCost GetNetworkCost()
        {
            m_cost = NetworkCost_Unmetered;
            return m_cost;
        }
    };

    NetworkInformation::NetworkInformation(IRuntimeConfig& configuration) :
        NetworkInformationImpl(configuration)
    {
        m_type = NetworkType_Wired;
        m_cost = NetworkCost_Unmetered;
    }

    NetworkInformation::~NetworkInformation()
    {
    }

    std::shared_ptr<INetworkInformation> NetworkInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<NetworkInformation>(configuration);
    }

} PAL_NS_END

