//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef INETWORKINFORMATION_HPP
#define INETWORKINFORMATION_HPP

#include "pal/PAL.hpp"
#include "IInformationProvider.hpp"

#include <string>

#define ETHERNET_AVAILABLE "EthernetAvailable"
#define WIFI_AVAILABLE "WifiAvailable"
#define WWAN_AVAILABLE "WwanAvailable"

// PropertyName
#define NETWORK_PROVIDER "NetworkProvider"
#define NETWORK_COST "NetworkCost"
#define NETWORK_TYPE "NetworkType"

using namespace MAT;

namespace PAL_NS_BEGIN {

    class INetworkInformation : public IInformationProvider
    {
    public:

        virtual ~INetworkInformation() {};

        /// <summary>
        /// Gets the current network provider for the device
        /// </summary>
        /// <returns>The current network provider for the device</returns>
        virtual std::string const& GetNetworkProvider() = 0;

        /// <summary>
        /// Gets the current network type for the device
        /// E.g. Wifi, 3G, Ethernet
        /// </summary>
        /// <returns>The current network type for the device</returns>
        virtual NetworkType GetNetworkType() = 0;

        /// <summary>
        /// Gets the current network cost for the device:
        /// OVER_DATA_LIMIT
        /// METERED
        /// UNMETERED
        /// </summary>
        /// <returns>The current network cost for the device</returns>
        virtual NetworkCost GetNetworkCost() = 0;

        /// <summary>
        /// Gets the current network Ethernet availability
        /// </summary>
        /// <returns>Ethernet availability</returns>
        virtual bool IsEthernetAvailable() = 0;

        /// <summary>
        /// Gets the current network wifi availability
        /// </summary>
        /// <returns>Wifi availability</returns>
        virtual bool IsWifiAvailable() = 0;

        /// <summary>
        /// Gets the current network Wwan availability
        /// </summary>
        /// <returns>Wwan availability</returns>
        virtual bool IsWwanAvailable() = 0;
    };

} PAL_NS_END
#endif

