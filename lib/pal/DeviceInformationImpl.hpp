#ifndef DEVICEINFORMATIONIMPL_HPP
#define DEVICEINFORMATIONIMPL_HPP

#include <pal/PAL.hpp>
#include "Enums.hpp"

#include "IDeviceInformation.hpp"
#include "InformationProviderImpl.hpp"

#include <string>

namespace PAL_NS_BEGIN {

    class DeviceInformationImpl : public IDeviceInformation
    {
    public:
        static IDeviceInformation* Create();

        // IInformationProvider API
        virtual int RegisterInformationChangedCallback(PAL::IPropertyChangedCallback* pCallback) override;
        virtual void UnRegisterInformationChangedCallback(int callbackToken) override;

        // IDeviceInformation API
        virtual std::string const& GetDeviceId() const { return m_device_id; }
        virtual std::string const& GetManufacturer() const { return m_manufacturer; }
        virtual std::string const& GetModel() const { return m_model; }
        virtual OsArchitectureType GetOsArchitectureType() const { return m_os_architecture; }
        virtual PowerSource GetPowerSource() const { return m_powerSource; }
        virtual std::string GetDeviceTicket();

    private:
        std::string m_device_id;
        std::string m_manufacturer;
        std::string m_model;
        std::string m_deviceTicket;
        OsArchitectureType m_os_architecture;
        PowerSource m_powerSource;
        InformatonProviderImpl m_info_helper;
        int m_registredCount;
        // Disable copy constructor and assignment operator.
        DeviceInformationImpl(DeviceInformationImpl const& other);
        DeviceInformationImpl& operator=(DeviceInformationImpl const& other);

        DeviceInformationImpl();
        virtual ~DeviceInformationImpl();
    };

} PAL_NS_END

#endif
