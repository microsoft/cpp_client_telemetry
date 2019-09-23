// Copyright (c) Microsoft Corporation. All rights reserved.
#define LOG_MODULE DBG_PAL
#include "pal/PAL.hpp"
#include "pal/NetworkInformationImpl.hpp"

#import <Reachability/Reachability.h>

namespace PAL_NS_BEGIN {

    NetworkInformationImpl::NetworkInformationImpl(bool isNetDetectEnabled) :
        m_info_helper(),
        m_isNetDetectEnabled(isNetDetectEnabled) {};

    NetworkInformationImpl::~NetworkInformationImpl() {};

    class NetworkInformation : public NetworkInformationImpl
    {
    public:
        /// <summary>
        ///
        /// </summary>
        /// <param name="isNetDetectEnabled"></param>
        NetworkInformation(bool isNetDetectEnabled);

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
            return m_type;
        }

        virtual NetworkCost GetNetworkCost()
        {
            m_cost = NetworkCost_Unmetered;
            return m_cost;
        }

    private:
        std::string m_network_provider;
    };

    NetworkInformation::NetworkInformation(bool isNetDetectEnabled) :
        NetworkInformationImpl(isNetDetectEnabled)
    {
        m_type = NetworkType_Wired;
        auto status = [Reachability currentReachabilityStatus];
        if(status == ReachableViaWifi)
        {
            m_type = NetworkType_Wifi;
        }
        else if (status == ReachableViaWWAN)
        {
            m_type = NetworkType_WWAN;
        }
        m_cost = NetworkCost_Unmetered;
    }

    NetworkInformation::~NetworkInformation()
    {
    }

    INetworkInformation* NetworkInformationImpl::Create(bool isNetDetectEnabled)
    {
        return new NetworkInformation(isNetDetectEnabled);
    }

} PAL_NS_END
