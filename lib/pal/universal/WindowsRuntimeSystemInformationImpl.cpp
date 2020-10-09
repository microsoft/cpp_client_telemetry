//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "pal/PAL.hpp"

#include <collection.h>

#include "ISystemInformation.hpp"
#include "pal/SystemInformationImpl.hpp"
#include "pal/desktop/WindowsEnvironmentInfo.hpp"
#include "PlatformHelpers.h"

using namespace std;
using namespace ::Windows::ApplicationModel;
using namespace ::Windows::System::UserProfile;
using namespace ::Windows::System::Profile;
using namespace ::Windows::Globalization;

using namespace Microsoft::Applications::Telemetry::Windows;

namespace PAL_NS_BEGIN {

    const string WindowsOSName = "Windows";
    const string WindowsPhoneOSName = "Windows for Phones";
    const string DeviceFamily_Mobile = "Windows.Mobile";

    std::shared_ptr<ISystemInformation> SystemInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<SystemInformationImpl>(configuration);
    }

    SystemInformationImpl::SystemInformationImpl(IRuntimeConfig& /*configuration*/)
        : m_info_helper()
    {
        auto version = Package::Current->Id->Version;

        m_app_id = FromPlatformString(Package::Current->Id->Name);
        m_app_version = std::to_string(version.Major) + "." + std::to_string(version.Minor)
            + "." + std::to_string(version.Build) + "." + std::to_string(version.Revision);

        m_user_language = "en";
        try {
            if (GlobalizationPreferences::Languages->Size)
                m_user_language = FromPlatformString(GlobalizationPreferences::Languages->GetAt(0));
            m_user_timezone = WindowsEnvironmentInfo::GetTimeZone();
        }
        catch (AccessDeniedException^)
        {
            // Windows 10 RS4+ OS bug: sometimes access to GlobalizationPreferences::Languages fails
            // with "Access Denied" error. It is not certain if it's because the list is empty or
            // because the app is not allowed to access the list of languages.
            LOG_WARN("Unable to obtain user Language!");
        }

        try
        {
            m_user_advertising_id = FromPlatformString(AdvertisingManager::AdvertisingId);
        }
        catch (Exception^)
        {
            // This throws FileNotFoundException on Windows 10 v10033/UAP v22623.
        }


#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
        // Only works for Windows Phone 8.1. A run-time check (below) is required in Threshold.
        m_os_name = WindowsPhoneOSName;
#else
        m_os_name = WindowsOSName;
#endif

#ifdef _WIN32_WINNT_WIN10

        // The DeviceFamilyVersion is a decimalized form of the ULONGLONG hex form. For example:
        // 2814750430068736 = 000A000027840000 = 10.0.10116.0
        auto versionDec = std::stoull(AnalyticsInfo::VersionInfo->DeviceFamilyVersion->Data());
        if (versionDec != 0ull)
        {
            m_os_major_version = std::to_string(versionDec >> 16 * 3) + "." + std::to_string(versionDec >> 16 * 2 & 0xFFFF);
            m_os_full_version = m_os_major_version +
                "." + std::to_string(versionDec >> 16 & 0xFFFF) + "." + std::to_string(versionDec & 0xFFFF);
        }
        else
        {
            m_os_major_version = "10.0";
        }

        if (FromPlatformString(AnalyticsInfo::VersionInfo->DeviceFamily) == DeviceFamily_Mobile)
        {
            m_os_name = WindowsPhoneOSName;
        }

#else // Windows 8.1 SDK
        m_os_major_version = "8.1";
        // There is no API to get full version in Windows 8.1.
#endif
        String ^primaryLanguage = "en";
        try {
            if (ApplicationLanguages::Languages->Size) {
                primaryLanguage = ApplicationLanguages::Languages->GetAt(0);
            }
        }
        catch (...) {
            //CTDEBUGLOG("Language detection may not be reliable!");
        }
        m_app_language = FromPlatformString(primaryLanguage);
        // CTDEBUGLOG("m_app_language=%s", m_app_language.c_str());
    }

    SystemInformationImpl::~SystemInformationImpl()
    {
    }

    int SystemInformationImpl::RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback)
    {
        return m_info_helper.RegisterInformationChangedCallback(pCallback);
    }

    void SystemInformationImpl::UnRegisterInformationChangedCallback(int callbackToken)
    {
        m_info_helper.UnRegisterInformationChangedCallback(callbackToken);
    }

} PAL_NS_END

