#include <collection.h>
#include "pal/PAL.hpp"
#include "pal/DeviceInformationImpl.hpp"
#include "pal/win32/WindowsEnvironmentInfo.h"
#include "PlatformHelpers.h"

#define LOG_MODULE DBG_PAL

#define DEFAULT_DEVICE_ID       "{deadbeef-fade-dead-c0de-cafebabefeed}"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace PAL
            {
                using namespace ::Windows::Networking::Connectivity;
                using namespace ::Windows::Devices::Input;

                using namespace Microsoft::Applications::Telemetry::Windows;
                using namespace ::Windows::Security::ExchangeActiveSyncProvisioning;

                /**
                * Returns the GUID of the 1st network adapter.
                */
                const char * getDeviceId()
                {
                    std::string deviceId;
                    try 
                    {
                        auto networkProfiles = NetworkInformation::GetConnectionProfiles();
                        if (networkProfiles->Size != 0)
                        {
                            // The first adapter is always LAN and cannot be removed. // TODO: Normalize the value using MD5 (per Root Tools).
                            auto  adapter = networkProfiles->GetAt(0)->NetworkAdapter;
                            deviceId = FromPlatformString(adapter->NetworkAdapterId.ToString());
                        }
                    }
                    catch (...)
                    {// Workaround for Windows OS bug VSO: 11314171 - sometimes NetworkInformation triggers exception
                        deviceId = DEFAULT_DEVICE_ID;
                    }
                    return deviceId.c_str();
                }

                // Helper functions.
                PowerSource GetCurrentPowerSource()
                {
#ifdef _WIN32_WINNT_WIN10
                    // Windows.System.Power was introduced in Windows 10.
                    switch (::Windows::System::Power::PowerManager::BatteryStatus)
                    {
                    case ::Windows::System::Power::BatteryStatus::Idle:
                    case ::Windows::System::Power::BatteryStatus::Charging:
                        return PowerSource_Charging;

                    case ::Windows::System::Power::BatteryStatus::Discharging:

                        return PowerSource_Battery;
                    }
#endif
                    /* Assume charging (AC) when not on battery */
                    return PowerSource_Charging;
                }

                std::string DeviceInformationImpl::GetDeviceTicket()
                {
                    return m_deviceTicket;
                }

                ///// IDeviceInformation API
                DeviceInformationImpl::DeviceInformationImpl() :
                    m_info_helper()
                {
                    m_os_architecture = WindowsEnvironmentInfo::GetProcessorArchitecture();

                    auto easClientDeviceInformation = ref new ::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
                    m_model = FromPlatformString(easClientDeviceInformation->SystemProductName);
                    m_manufacturer = FromPlatformString(easClientDeviceInformation->SystemManufacturer);
                    m_device_id = getDeviceId();

#ifdef _WIN32_WINNT_WIN10
#else // Windows 8.1 SDK
                    // TODO: check if we need to populate these two m_cpu* fields on WINRT?
                    // ExchangeActiveSyncProvisioning is currently unavailable on Windows Threshold.
                    auto deviceInformation = ref new  ::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
                    m_cpu_manufacturer = FromPlatformString(deviceInformation->SystemManufacturer);
                    m_cpu_model = FromPlatformString(deviceInformation->SystemProductName);
#endif
                    m_powerSource = GetCurrentPowerSource();
#ifdef _WIN32_WINNT_WIN10
                    // Windows.System.Power was introduced in Windows 10.
                    auto onPowerSourceChanged = ref new ::Windows::Foundation::EventHandler<Object^>([this](Object^ sender, Object^ args)
                    {
                        // No need to use WeakReference as this is not ref counted.
                        // See https://msdn.microsoft.com/en-us/library/hh699859.aspx for details.
                        auto powerSource = GetCurrentPowerSource();
                        ARIASDK_LOG_DETAIL("Power source changed to %d", powerSource);
                        // No need for the lock here - the event is called syncronously.
                        if (m_powerSource != powerSource)
                        {
                            m_powerSource = powerSource;
                            m_info_helper.OnChanged(POWER_SOURCE, std::to_string(powerSource));
                        }
                    });

                    ::Windows::System::Power::PowerManager::BatteryStatusChanged += onPowerSourceChanged;
                    ::Windows::System::Power::PowerManager::PowerSupplyStatusChanged += onPowerSourceChanged;
#endif
                }

				IDeviceInformation* DeviceInformationImpl::Create()
                {
                    return new DeviceInformationImpl();
                }		

				size_t DeviceInformationImpl::GetMemorySize() const
				{
					return 0;
				}
            }
        }
    }
}
