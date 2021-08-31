//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#define LOG_MODULE DBG_PAL
#include "pal/PAL.hpp"
#include "pal/NetworkInformationImpl.hpp"

#import <Network/Network.h>
#import "ODWReachability.h"

namespace PAL_NS_BEGIN {

    NetworkInformationImpl::NetworkInformationImpl(IRuntimeConfig& configuration) :
        m_info_helper(),
        m_isNetDetectEnabled(configuration[CFG_BOOL_ENABLE_NET_DETECT]) {};

    NetworkInformationImpl::~NetworkInformationImpl() {};

    class NetworkInformation : public NetworkInformationImpl,
                               public std::enable_shared_from_this<NetworkInformation>
    {
    public:
        /// <summary>
        ///
        /// </summary>
        /// <param name="isNetDetectEnabled"></param>
        NetworkInformation(IRuntimeConfig& configuration);

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

        /// <summary>
        /// Setup initial network information and start net monitor if requested.
        /// This cannot be put in constructor because we need to use shared_from_this.
        /// </summary>
        void SetupNetDetect();

    private:
        void UpdateType(NetworkType type) noexcept;
        void UpdateCost(NetworkCost cost) noexcept;
        std::string m_network_provider {};

        // iOS 12 and newer
        nw_path_monitor_t m_monitor = nil;

        // iOS 11 and older
        ODWReachability* m_reach = nil;
        id m_notificationId = nil;
    };

    NetworkInformation::NetworkInformation(IRuntimeConfig& configuration) :
        NetworkInformationImpl(configuration)
    {
        m_type = NetworkType_Unknown;
        m_cost = NetworkCost_Unknown;
    }

    NetworkInformation::~NetworkInformation() noexcept
    {
        if (@available(macOS 10.14, iOS 12.0, *))
        {
            if (m_isNetDetectEnabled)
            {
                nw_path_monitor_cancel(m_monitor);
            }
        }
        else
        {
            if (m_isNetDetectEnabled)
            {
                [[NSNotificationCenter defaultCenter] removeObserver:m_notificationId];
                [m_reach stopNotifier];
            }
        }
    }

    void NetworkInformation::SetupNetDetect()
    {
        auto weak_this = std::weak_ptr<NetworkInformation>(shared_from_this());

        m_reach = [ODWReachability reachabilityForInternetConnection];
        void (^block)(NSNotification*) = ^(NSNotification*)
        {
            auto strong_this = weak_this.lock();
            if (!strong_this)
            {
                return;
            }

            // NetworkCost information is not available until iOS 12.
            // Just make the best guess here.
            switch (m_reach.currentReachabilityStatus)
            {
                case NotReachable:
                    strong_this->UpdateType(NetworkType_Unknown);
                    strong_this->UpdateCost(NetworkCost_Unknown);
                    break;
                case ReachableViaWiFi:
                    strong_this->UpdateType(NetworkType_Wifi);
                    strong_this->UpdateCost(NetworkCost_Unmetered);
                    break;
                case ReachableViaWWAN:
                    strong_this->UpdateType(NetworkType_WWAN);
                    strong_this->UpdateCost(NetworkCost_Metered);
                    break;
            }
        };
        block(nil); // Update the initial status.
        
        if (@available(macOS 10.14, iOS 12.0, *))
        {
            m_monitor = nw_path_monitor_create();
            nw_path_monitor_set_queue(m_monitor, dispatch_get_global_queue(QOS_CLASS_BACKGROUND, 0));
            nw_path_monitor_set_update_handler(m_monitor, ^(nw_path_t path)
            {
                auto strong_this = weak_this.lock();
                if (!strong_this)
                {
                    return;
                }

                NetworkType type = NetworkType_Unknown;
                NetworkCost cost = NetworkCost_Unknown;
                nw_path_status_t status = nw_path_get_status(path);
                bool connected = status == nw_path_status_satisfied || status == nw_path_status_satisfiable;
                if (connected)
                {
                    if (nw_path_uses_interface_type(path, nw_interface_type_wifi))
                    {
                        type = NetworkType_Wifi;
                    }
                    else if (nw_path_uses_interface_type(path, nw_interface_type_cellular))
                    {
                        type = NetworkType_WWAN;
                    }
                    else if (nw_path_uses_interface_type(path, nw_interface_type_wired))
                    {
                        type = NetworkType_Wired;
                    }
                    cost = nw_path_is_expensive(path) ? NetworkCost_Metered : NetworkCost_Unmetered;
                    if (@available(macOS 10.15, iOS 13.0, *))
                    {
                        if (nw_path_is_constrained(path))
                        {
                            cost = NetworkCost_Roaming;
                        }
                    }
                }
                strong_this->UpdateType(type);
                strong_this->UpdateCost(cost);
            });
            nw_path_monitor_start(m_monitor);

            // nw_path_monitor_start will invoke the callback for once. So if
            // we don't want to listen for changes, we can just start the
            // monitor and stop it right away.
            if (!m_isNetDetectEnabled)
            {
                nw_path_monitor_cancel(m_monitor);
            }
        }
        else if (m_isNetDetectEnabled)
        {
            m_notificationId =
                [[NSNotificationCenter defaultCenter]
                addObserverForName: kNetworkReachabilityChangedNotification
                object: nil
                queue: nil
                usingBlock: block];
            [m_reach startNotifier];
        }
    }

    void NetworkInformation::UpdateType(NetworkType type) noexcept
    {
        if (type != m_type)
        {
            m_type = type;
            m_info_helper.OnChanged(NETWORK_TYPE, std::to_string(type));
        }
    }

    void NetworkInformation::UpdateCost(NetworkCost cost) noexcept
    {
        if (cost != m_cost)
        {
            m_cost = cost;
            m_info_helper.OnChanged(NETWORK_COST, std::to_string(cost));
        }
    }

    std::shared_ptr<INetworkInformation> NetworkInformationImpl::Create(IRuntimeConfig& configuration)
    {
        auto networkInformation = std::make_shared<NetworkInformation>(configuration);
        networkInformation->SetupNetDetect();
        return networkInformation;
    }

} PAL_NS_END

