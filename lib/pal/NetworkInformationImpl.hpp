//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef NETWORKINFORMATIONIMPL_HPP
#define NETWORKINFORMATIONIMPL_HPP

#include "pal/PAL.hpp"
#include "Enums.hpp"
#include "INetworkInformation.hpp"
#include "InformationProviderImpl.hpp"

#include "IPropertyChangedCallback.hpp"

#include <string>

using namespace MAT;

namespace PAL_NS_BEGIN {

    class NetworkInformationImpl : public INetworkInformation
    {
    public:
        static std::shared_ptr<INetworkInformation> Create(MAT::IRuntimeConfig& configuration);

        // IInformationProvider API
        virtual int  RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback) { m_registeredCount++; return m_info_helper.RegisterInformationChangedCallback(pCallback); }
        virtual void UnRegisterInformationChangedCallback(int callbackToken) { --m_registeredCount; m_info_helper.UnRegisterInformationChangedCallback(callbackToken); }

        // INetworkInformation API
        virtual std::string const& GetNetworkProvider() { return m_provider; };
        virtual NetworkType GetNetworkType() { return m_type; }
        virtual NetworkCost GetNetworkCost() { return m_cost; }

        virtual bool IsEthernetAvailable() { return false; }
        virtual bool IsWifiAvailable() { return false; }
        virtual bool IsWwanAvailable() { return false; }

        NetworkInformationImpl(MAT::IRuntimeConfig& configuration);
        virtual ~NetworkInformationImpl();

        // Disable copy constructor and assignment operator.
        NetworkInformationImpl(NetworkInformationImpl const& other) = delete;
        NetworkInformationImpl& operator=(NetworkInformationImpl const& other) = delete;

    protected:
        std::string m_provider;
        NetworkType m_type;
        NetworkCost m_cost;

        InformatonProviderImpl m_info_helper;
        int m_registeredCount;
        bool m_isNetDetectEnabled;
    };

} PAL_NS_END
#endif

