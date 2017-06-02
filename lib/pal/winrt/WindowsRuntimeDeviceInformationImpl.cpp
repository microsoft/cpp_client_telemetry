#include <collection.h>
#include "PAL.hpp"
#include "DeviceInformationImpl.hpp"
#include "win32\WindowsEnvironmentInfo.h"
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

                ///// IDeviceInformation API
                DeviceInformationImpl::DeviceInformationImpl() :
                    m_info_helper()
                {
                    m_os_architecture = WindowsEnvironmentInfo::GetProcessorArchitecture();

                    auto easClientDeviceInformation = ref new ::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
                    m_model = FromPlatformString(easClientDeviceInformation->SystemProductName);
                    m_manufacturer = FromPlatformString(easClientDeviceInformation->SystemManufacturer);

                    try {
                        auto networkProfiles = NetworkInformation::GetConnectionProfiles();
                        if (networkProfiles->Size != 0)
                        {
                            // The first adapter is always LAN and cannot be removed.
                            // TODO: Normalize the value using MD5 (per Root Tools).
                            auto  adapter = networkProfiles->GetAt(0)->NetworkAdapter;
                            m_device_id = FromPlatformString(adapter->NetworkAdapterId.ToString());
                        }
                    }
                    catch (...)
                    {
                        // Workaround for Windows OS bug VSO: 11314171 - sometimes NetworkInformation triggers exception
                        m_device_id = DEFAULT_DEVICE_ID;
                    }

#ifdef _WIN32_WINNT_WIN10
#else // Windows 8.1 SDK
                    // TODO: check if we need to populate these two m_cpu* fields on WINRT?
                    // ExchangeActiveSyncProvisioning is currently unavailable on Windows Threshold.
                    auto deviceInformation = ref new  ::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
                    m_cpu_manufacturer = FromPlatformString(deviceInformation->SystemManufacturer);
                    m_cpu_model = FromPlatformString(deviceInformation->SystemProductName);
#endif
                    auto mouseCapabilities = ref new MouseCapabilities();
                    auto keyboardCapabilities = ref new KeyboardCapabilities();
                    auto touchCapabilities = ref new TouchCapabilities();

                    m_mouseCount = mouseCapabilities->MousePresent;
                    m_keyboardCount = keyboardCapabilities->KeyboardPresent;
                    m_touchCount = touchCapabilities->TouchPresent;

                    m_digitizerCount = 0;
                    IVectorView<PointerDevice^>^ pointerDeviceList = PointerDevice::GetPointerDevices();
                    for (auto device : pointerDeviceList)
                    {
                        if (device->PointerDeviceType == PointerDeviceType::Pen)
                        {
                            m_digitizerCount++;
                        }
                    }

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
