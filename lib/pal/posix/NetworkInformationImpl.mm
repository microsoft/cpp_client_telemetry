// Copyright (c) Microsoft Corporation. All rights reserved.
#define LOG_MODULE DBG_PAL
#include "pal/PAL.hpp"
#include "pal/NetworkInformationImpl.hpp"

#import "Reachability.h"

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
        virtual ~NetworkInformation() noexcept;

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
        /// E.g. Wifi, WWAN
        /// </summary>
        /// <returns>The current network type for the device</returns>
        virtual NetworkType GetNetworkType()
        {
            return m_type;
        }

        virtual NetworkCost GetNetworkCost()
        {
            return m_cost;
        }

    private:
        NetworkType GetNetworkTypeInternal() noexcept;
        std::string m_network_provider {};
        Reachability* m_reach = [Reachability reachabilityForInternetConnection];
        id m_notificationId = nil;
    };

    NetworkInformation::NetworkInformation(bool isNetDetectEnabled) :
        NetworkInformationImpl(isNetDetectEnabled)
    {
        m_type = GetNetworkTypeInternal();
        m_cost = NetworkCost_Unknown;

        if (isNetDetectEnabled)
        {
            m_notificationId =
                [[NSNotificationCenter defaultCenter]
                 addObserverForName: kReachabilityChangedNotification
                 object: nil
                 queue: nil
                 usingBlock: ^(NSNotification*){
                     auto type = GetNetworkTypeInternal();
                     if (type != m_type)
                     {
                         m_type = type;
                         m_info_helper.OnChanged(NETWORK_TYPE, std::to_string(type));
                     }
                }];

            [m_reach startNotifier];
        }
    }

    NetworkInformation::~NetworkInformation() noexcept
    {
        if (m_isNetDetectEnabled)
        {
            [[NSNotificationCenter defaultCenter] removeObserver:m_notificationId];
            [m_reach stopNotifier];
        }
    }

    NetworkType NetworkInformation::GetNetworkTypeInternal() noexcept
    {
        auto status = [m_reach currentReachabilityStatus];
        if(status == ReachableViaWiFi)
        {
            return NetworkType_Wifi;
        }
        else if (status == ReachableViaWWAN)
        {
            return NetworkType_WWAN;
        }
        
        return NetworkType_Unknown;
    }

    INetworkInformation* NetworkInformationImpl::Create(bool isNetDetectEnabled)
    {
        return new NetworkInformation(isNetDetectEnabled);
    }

} PAL_NS_END
