// Copyright (c) Microsoft. All rights reserved.
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
        static IDeviceInformation* Create();

        virtual int RegisterInformationChangedCallback(PAL::IPropertyChangedCallback* pCallback) override
        {
            m_registredCount++;
            return m_info_helper.RegisterInformationChangedCallback(pCallback);
        }

        virtual void UnRegisterInformationChangedCallback(int callbackToken) override
        {
            --m_registredCount;
            m_info_helper.UnRegisterInformationChangedCallback(callbackToken);
        }

        // IDeviceInformation API
        virtual std::string const& GetDeviceId() const override { return m_device_id; }
        virtual std::string const& GetManufacturer() const override { return m_manufacturer; }
        virtual std::string const& GetModel() const override { return m_model; }
        virtual OsArchitectureType GetOsArchitectureType() const override { return m_os_architecture; }
        virtual PowerSource GetPowerSource() const override { return m_powerSource; }
        virtual std::string GetDeviceTicket() const override;

    private:
        std::string m_device_id;
        std::string m_manufacturer;
        std::string m_model;
        std::string m_deviceTicket;
        OsArchitectureType m_os_architecture;
    protected:
        PowerSource m_powerSource;
        InformatonProviderImpl m_info_helper;
    private:
        int m_registredCount;
        // Disable copy constructor and assignment operator.
        DeviceInformationImpl(DeviceInformationImpl const& other);
        DeviceInformationImpl& operator=(DeviceInformationImpl const& other);

        protected:
        DeviceInformationImpl();
        virtual ~DeviceInformationImpl();
    };

} PAL_NS_END

#endif
