//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <collection.h>
#include <windows.h>

#include "pal/PAL.hpp"
#include "pal/DeviceInformationImpl.hpp"
#include "pal/desktop/WindowsEnvironmentInfo.hpp"
#include "PlatformHelpers.h"

#include <exception>

#define LOG_MODULE DBG_PAL

#define DEFAULT_DEVICE_ID       "{deadbeef-fade-dead-c0de-cafebabefeed}"

namespace PAL_NS_BEGIN {

                using namespace ::Windows::Networking::Connectivity;
                using namespace ::Windows::Devices::Input;
                using namespace ::Windows::Foundation;

                using namespace Microsoft::Applications::Telemetry::Windows;
                using namespace ::Windows::Security::ExchangeActiveSyncProvisioning;
                EventRegistrationToken token1;
                EventRegistrationToken token2;

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
                DeviceInformationImpl::DeviceInformationImpl(MAT::IRuntimeConfig& configuration) :
                    m_registeredCount(0),
                    m_info_helper()
                {
                    m_os_architecture = WindowsEnvironmentInfo::GetProcessorArchitecture();

                    auto easClientDeviceInformation = ref new ::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
                    m_model = FromPlatformString(easClientDeviceInformation->SystemProductName);
                    m_manufacturer = FromPlatformString(easClientDeviceInformation->SystemManufacturer);

                    bool isNetDetectEnabled = configuration[CFG_BOOL_ENABLE_NET_DETECT];
                    m_device_id = DEFAULT_DEVICE_ID;

                    try {
                        // Note that if Network Detection is off, then we do not have acccess
                        // to NetworkInformation statics. Thus, the best available device id
                        // is the one populated by UTC (Diagnostic Tracking Service) from MSA
                        // Device Id claim.
                        if (isNetDetectEnabled)
                        {
                            auto networkProfiles = NetworkInformation::GetConnectionProfiles();
                            if (networkProfiles->Size != 0)
                            {
                                // The first adapter is always LAN and cannot be removed.
                                // TODO: Normalize the value using MD5 (per Root Tools).
                                auto adapter = networkProfiles->GetAt(0)->NetworkAdapter;
                                if (adapter != nullptr)
                                {
                                    m_device_id = FromPlatformString(adapter->NetworkAdapterId.ToString());
                                }
                            }
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
#endif
                    m_powerSource = GetCurrentPowerSource();
#ifdef _WIN32_WINNT_WIN10
                    // Windows.System.Power was introduced in Windows 10.
                    auto onPowerSourceChanged = ref new ::Windows::Foundation::EventHandler<Object^>([this](Object^ sender, Object^ args)
                    {
                        try
                        {
                            if (m_registeredCount > 0)
                            {
                                // No need to use WeakReference as this is not ref counted.
                                // See https://msdn.microsoft.com/en-us/library/hh699859.aspx for details.
                                auto powerSource = GetCurrentPowerSource();
                                //LOG_TRACE("Power source changed to %d", powerSource);
                                // No need for the lock here - the event is called synchronously.
                                if (m_powerSource != powerSource)
                                {
                                    m_powerSource = powerSource;
                                    m_info_helper.OnChanged(POWER_SOURCE, std::to_string(powerSource));
                                }
                            }
                        }
                        catch (...)
                        {

                        }
                    });

                    token1 = ::Windows::System::Power::PowerManager::BatteryStatusChanged += onPowerSourceChanged;
                    token2 = ::Windows::System::Power::PowerManager::PowerSupplyStatusChanged += onPowerSourceChanged;

#endif
                }

                std::string DeviceInformationImpl::GetDeviceTicket() const
                {
                    return m_deviceTicket;
                }

                DeviceInformationImpl::~DeviceInformationImpl()
                {
                     ::Windows::System::Power::PowerManager::BatteryStatusChanged -= token1;
                     ::Windows::System::Power::PowerManager::PowerSupplyStatusChanged -= token2;
                }

                std::shared_ptr<IDeviceInformation> DeviceInformationImpl::Create(IRuntimeConfig& configuration)
                {
                    return std::make_shared<DeviceInformationImpl>(configuration);
                }

} PAL_NS_END

