//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <windows.h>
#include "pal/NetworkInformationImpl.hpp"
#include <exception>

using namespace ::Windows::Networking::Connectivity;

namespace PAL_NS_BEGIN {

                Windows::Foundation::EventRegistrationToken token;

                // Helper method. Retrieves network type based on the specified connection profile.
                NetworkType GetNetworkTypeForProfile(ConnectionProfile^ profile)
                {
                    if (profile != nullptr)
                    {
                        if (profile->IsWlanConnectionProfile)
                        {
                            return NetworkType_Wifi;
                        }
                        else if (profile->IsWwanConnectionProfile)
                        {
                            return NetworkType_WWAN;
                        }
                        else
                        {
                            auto IanaInterfaceType = profile->NetworkAdapter->IanaInterfaceType;
                            if (IanaInterfaceType == 6)
                            {
                                return NetworkType_Wired;
                            }
                        }
                    }

                    return NetworkType_Unknown;
                }

                // Helper method. Retrieves network cost based on the specified connection profile.
                NetworkCost GetNetworkCostForProfile(ConnectionProfile^ profile)
                {
                    if (profile != nullptr)
                    {
                        auto cost = profile->GetConnectionCost();

                        if (cost->Roaming || cost->OverDataLimit || cost->NetworkCostType == NetworkCostType::Variable)
                        {
                            return NetworkCost_OverDataLimit;
                        }

                        if (cost->NetworkCostType == NetworkCostType::Fixed)
                        {
                            return NetworkCost_Metered;
                        }

                        if (cost->NetworkCostType == NetworkCostType::Unrestricted)
                        {
                            return NetworkCost_Unmetered;
                        }
                    }

                    return NetworkCost_Unknown;
                }

                NetworkInformationImpl::NetworkInformationImpl(IRuntimeConfig& configuration) :
                    m_info_helper(),
                    m_registeredCount(0),
                    m_isNetDetectEnabled(configuration[CFG_BOOL_ENABLE_NET_DETECT])
                {
                    m_type = NetworkType_Unknown;
                    m_cost = NetworkCost_Unknown;

                    token.Value = 0;

                    // If Network Detector is turned off, then no need to obtain NetworkInformation.
                    if (!m_isNetDetectEnabled)
                    {
                        return;
                    }

                    // NetworkInformation::GetInternetConnectionProfile() may fail under
                    // some unknown scenarios on some Windows versions (Windows API bug).
                    // In those cases we assume the network type and cost both as Unknown.
                    try {
                        auto profile = NetworkInformation::GetInternetConnectionProfile();
                        if (profile == nullptr)
                            return;
                        m_type = GetNetworkTypeForProfile(profile);
                        m_cost = GetNetworkCostForProfile(profile);
                    }
                    catch (...)
                    {
                        m_type = NetworkType_Unknown;
                        m_cost = NetworkCost_Unknown;
                        return;
                    }

                    token = NetworkInformation::NetworkStatusChanged +=
                        ref new NetworkStatusChangedEventHandler([this](Platform::Object^ sender)
                    {
                        // No need to use WeakReference as this is not ref counted.
                        // See https://msdn.microsoft.com/en-us/library/hh699859.aspx for details.
                        try {
                            if (m_registeredCount > 0)
                            {
                                auto profile = NetworkInformation::GetInternetConnectionProfile();
                                NetworkType networkType = NetworkType_Unknown;
                                NetworkCost networkCost = NetworkCost_Unknown;
                                if (profile != nullptr)
                                {
                                    networkType = GetNetworkTypeForProfile(profile);
                                    networkCost = GetNetworkCostForProfile(profile);
                                }
                                // No need for the lock here - the event is called synchronously.
                                if (m_type != networkType)
                                {
                                    m_type = networkType;
                                    m_info_helper.OnChanged(NETWORK_TYPE, std::to_string(networkType));
                                }

                                if (m_cost != networkCost)
                                {
                                    m_cost = networkCost;
                                    m_info_helper.OnChanged(NETWORK_COST, std::to_string(networkCost));
                                }
                            }
                        }
                        catch (...) {
                            // Let's not fire network state change callback here because something went
                            // really bad with Windows Networking API. If device is offline, then TPM
                            // timer would fire and we re-check if internet is connected. If not, then
                            // we would not perform the upload.
                            m_type = NetworkType_Unknown;
                            m_cost = NetworkCost_Unknown;
                        };
                    });

                }

                NetworkInformationImpl::~NetworkInformationImpl()
                {
                    if (token.Value != 0)
                    {
                        NetworkInformation::NetworkStatusChanged -= token;
                    }
                };

                std::shared_ptr<INetworkInformation> NetworkInformationImpl::Create(IRuntimeConfig& configuration)
                {
                    return std::make_shared<NetworkInformationImpl>(configuration);
                }

} PAL_NS_END

