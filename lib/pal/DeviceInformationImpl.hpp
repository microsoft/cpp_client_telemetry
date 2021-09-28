//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef DEVICEINFORMATIONIMPL_HPP
#define DEVICEINFORMATIONIMPL_HPP

#include "pal/PAL.hpp"
#include "Enums.hpp"
#include "IDeviceInformation.hpp"
#include "InformationProviderImpl.hpp"

#include <string>

using namespace MAT;

namespace PAL_NS_BEGIN {

    class DeviceInformationImpl : public IDeviceInformation
    {
    public:
        static std::shared_ptr<IDeviceInformation> Create(MAT::IRuntimeConfig& configuration);

        virtual int RegisterInformationChangedCallback(PAL::IPropertyChangedCallback* pCallback) override
        {
            m_registeredCount++;
            return m_info_helper.RegisterInformationChangedCallback(pCallback);
        }

        virtual void UnRegisterInformationChangedCallback(int callbackToken) override
        {
            --m_registeredCount;
            m_info_helper.UnRegisterInformationChangedCallback(callbackToken);
        }

        // IDeviceInformation API
        virtual std::string const& GetDeviceId() const override { return m_device_id; }
        virtual std::string const& GetManufacturer() const override { return m_manufacturer; }
        virtual std::string const& GetModel() const override { return m_model; }
        virtual OsArchitectureType GetOsArchitectureType() const override { return m_os_architecture; }
        virtual PowerSource GetPowerSource() const override { return m_powerSource; }
        virtual std::string GetDeviceTicket() const override;

        DeviceInformationImpl(MAT::IRuntimeConfig& configuration);
        virtual ~DeviceInformationImpl();

        // Disable copy constructor and assignment operator.
        DeviceInformationImpl(DeviceInformationImpl const& other) = delete;
        DeviceInformationImpl& operator=(DeviceInformationImpl const& other) = delete;

    private:
        std::string m_deviceTicket;
        OsArchitectureType m_os_architecture;
    protected:
        PowerSource m_powerSource;
        InformatonProviderImpl m_info_helper;
        std::string m_device_id;
        std::string m_manufacturer;
        std::string m_model;
    private:
        size_t m_registeredCount;
    };

} PAL_NS_END

#endif

