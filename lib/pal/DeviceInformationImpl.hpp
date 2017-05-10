#pragma once
#include "aria/Enums.hpp"
#include "aria/IDeviceInformation.hpp"
#include "InformationProviderImpl.hpp"

namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

    class DeviceInformationImpl : public IDeviceInformation
    {
    public:
        static IDeviceInformation* Create();

        // IInformationProvider API
        virtual int RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback);
        virtual void UnRegisterInformationChangedCallback(int callbackToken);

        // IDeviceInformation API
        virtual std::string const& GetDeviceId() const { return m_device_id; }
        virtual std::string const& GetManufacturer() const { return m_manufacturer; }
        virtual std::string const& GetModel() const { return m_model; }

        virtual OsArchitectureType GetOsArchitectureType() const { return m_os_architecture; }
        virtual std::string const& GetCpuMake() const { return m_cpu_manufacturer; }
        virtual std::string const& GetCpuModel() const { return m_cpu_model; }

        virtual size_t GetMemorySize() const;
        virtual size_t GetStorageSize() const;

        virtual size_t GetScreenCount()const;

        virtual double GetScreenHeightDPI() const;
        virtual double GetScreenWidthDPI() const;

        virtual double GetScreenWidthInInches() const;
        virtual double GetScreenHeightInInches() const;

        virtual size_t GetScreenHeightInPixels() const;
        virtual size_t GetScreenWidthInPixels() const;

        virtual bool IsDigitizerAvailable() const { return m_digitizerCount > 0;  }
        virtual bool IsTouchAvailable() const { return m_touchCount > 0; }

        virtual bool IsKeyboardAvailable() const { return m_keyboardCount > 0; }
        virtual bool IsMouseAvailable() const { return m_mouseCount > 0; }

        virtual bool IsFrontCameraAvailable() const;
        virtual bool IsRearCameraAvailable() const;

        virtual PowerSource GetPowerSource() const { return m_powerSource; }

    private:
        std::string m_device_id;
        std::string m_manufacturer;
        std::string m_model;

        std::string m_cpu_manufacturer;
        std::string m_cpu_model;
        OsArchitectureType m_os_architecture;

        int m_digitizerCount;
        int m_touchCount;
        int m_mouseCount;
        int m_keyboardCount;
        PowerSource m_powerSource;

        InformatonProviderImpl m_info_helper;

        // Disable copy constructor and assignment operator.
        DeviceInformationImpl(DeviceInformationImpl const& other);
        DeviceInformationImpl& operator=(DeviceInformationImpl const& other);

        DeviceInformationImpl();
        ~DeviceInformationImpl() {}
    };

} // PAL
}}}