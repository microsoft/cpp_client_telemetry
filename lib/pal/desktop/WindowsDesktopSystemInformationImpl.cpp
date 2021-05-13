//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

#pragma warning( disable : 4996 )

// Windows 7.1 SDK module:
#pragma comment (lib, "Version.Lib")

/* Ref. https://docs.microsoft.com/en-us/windows/desktop/apiindex/windows-8-api-sets
 *
 * Compatibility with Windows 7, Windows Server 2008 R2 and older operating systems:
 * Binaries that link to MinCore.lib or MinCore_Downlevel.lib are not designed to work
 * on Windows 7, Windows Server 2008 R2 or earlier. Binaries that need to run on earlier
 * versions of Windows or Windows Server must not use either MinCore.lib or MinCore_Downlevel.lib.
 */

#include "pal/PAL.hpp"
#include "utils/Utils.hpp"

#include <Windows.h>

#include "ISystemInformation.hpp"
#include "pal/SystemInformationImpl.hpp"

#include "WindowsEnvironmentInfo.hpp"

#include <string>

// This define is only available for TH1+
// Issue #61 now tracks a fix that should work for win7+
#ifndef RRF_SUBKEY_WOW6464KEY
#define RRF_SUBKEY_WOW6464KEY  0x00010000  // when opening the subkey (if provided) force open from the 64bit location (only one SUBKEY_WOW64* flag can be set or RegGetValue will fail with ERROR_INVALID_PARAMETER)
#endif

using namespace MAT;

namespace PAL_NS_BEGIN {

    const std::string WindowsOSName = "Windows Desktop";

    std::shared_ptr<ISystemInformation> SystemInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<SystemInformationImpl>(configuration);
    }

    /**
     * Get executable full path as wide string
     */
    std::wstring getAppFullPath()
    {
        DWORD maxBufferLength = 0x7fff;
        DWORD curBufferLength = MAX_PATH;
        bool getFileNameSucceeded = false;

        std::vector<wchar_t> curExeFullPathBuffer(curBufferLength);

        // Try increasing buffer size until it work or until we reach the maximum size of 0x7fff (32 Kb)
        do
        {
            curExeFullPathBuffer.resize(curBufferLength);
            HMODULE handle = nullptr;
            if (::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &handle) == FALSE)
            {
                LOG_ERROR("Failed to get module handle with error: %d", ::GetLastError());
                return {};
            }
            DWORD result = ::GetModuleFileName(handle, &curExeFullPathBuffer[0], curBufferLength);

            if (result == curBufferLength)
            {
                // insufficient buffer, increase it
                curBufferLength = (curBufferLength < maxBufferLength ? (2 * curBufferLength) % (maxBufferLength + 1) : maxBufferLength + 1);
            }
            else if (result > 0)
            {
                // call succeeded
                curExeFullPathBuffer.resize(result + 1);
                getFileNameSucceeded = true;
                break;
            }
            else
            {
                // call failed for other reason
                break;
            }
        } while (curBufferLength <= maxBufferLength);

        if (getFileNameSucceeded)
        {
            return std::wstring{curExeFullPathBuffer.cbegin(), curExeFullPathBuffer.cend()};
        }

        return{};
    }

    /**
     * Get app module version
     */
    std::string getAppVersion()
    {
        DWORD   dwVersionInfoSize;
        DWORD   dwUnused;
        UINT    nUnused;
        std::vector<BYTE> buffer;
        VS_FIXEDFILEINFO* pffi;

        // Get the filename of the current process
        std::wstring applicationFullPath = getAppFullPath();

        // Get the product version information from the current process filename
        dwVersionInfoSize = GetFileVersionInfoSize(applicationFullPath.data(), &dwUnused);

        if (dwVersionInfoSize == 0)
        {
            return{};
        }

        buffer.resize(dwVersionInfoSize);

        if (GetFileVersionInfo(applicationFullPath.data(), 0, dwVersionInfoSize, &buffer[0]) == 0)
        {
            return{};
        }

        if (VerQueryValue(&buffer[0], L"\\", reinterpret_cast<LPVOID*> (&pffi), &nUnused) == 0)
        {
            return{};
        }

        return
            std::to_string(static_cast<int>(HIWORD(pffi->dwProductVersionMS))) + "." +
            std::to_string(static_cast<int>(LOWORD(pffi->dwProductVersionMS))) + "." +
            std::to_string(static_cast<int>(HIWORD(pffi->dwProductVersionLS))) + "." +
            std::to_string(static_cast<int>(LOWORD(pffi->dwProductVersionLS)));
    }

    /**
     * Get OS BuildLabEx string
     */
    std::string getOsBuildLabEx()
    {
        char buff[MAX_PATH] = { 0 };
        const PCSTR c_currentVersion_Key = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
        const PCSTR c_buildLabEx_ValueName = "BuildLabEx";
        DWORD size = sizeof(buff);
        RegGetValueA(HKEY_LOCAL_MACHINE, c_currentVersion_Key, c_buildLabEx_ValueName, RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, NULL, (char*)buff, &size);
        return buff;
    }

    /**
     * Get CommercialId from registry
     */
    std::string getCommercialId()
    {
        char buff[MAX_PATH] = { 0 };
        const PCSTR c_groupPolicyDataCollection_Key = "SOFTWARE\\Policies\\Microsoft\\Windows\\DataCollection";
        const PCSTR c_dataCollection_Key = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\DataCollection";
        const PCSTR c_commercialId = "CommercialId";
        DWORD size = sizeof(buff);
        if (RegGetValueA(HKEY_LOCAL_MACHINE, c_groupPolicyDataCollection_Key, c_commercialId, RRF_RT_REG_SZ, NULL, static_cast<char*>(buff), &size) != ERROR_SUCCESS)
        {
            size = sizeof(buff);
            RegGetValueA(HKEY_LOCAL_MACHINE, c_dataCollection_Key, c_commercialId, RRF_RT_REG_SZ, NULL, static_cast<char*>(buff), &size);
        }
        return buff;
    }

    /**
     * Get OS major and full version strings
     */
    void getOsVersion(std::string& osMajorVersion, std::string& osFullVersion)
    {
        std::string buildLabEx = getOsBuildLabEx();

        HMODULE hNtDll = ::GetModuleHandle(TEXT("ntdll.dll"));
        typedef HRESULT NTSTATUS;
        typedef NTSTATUS(__stdcall * RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
        RtlGetVersion_t pRtlGetVersion = hNtDll ? reinterpret_cast<RtlGetVersion_t>(::GetProcAddress(hNtDll, "RtlGetVersion")) : nullptr;

        RTL_OSVERSIONINFOW rtlOsvi = { sizeof(rtlOsvi), 0, 0, 0, 0, {0} };
        if (pRtlGetVersion && SUCCEEDED(pRtlGetVersion(&rtlOsvi)))
        {
            osMajorVersion = std::to_string((long long)rtlOsvi.dwMajorVersion) + "." + std::to_string((long long)rtlOsvi.dwMinorVersion);
            osFullVersion = osMajorVersion + "." + std::to_string((long long)rtlOsvi.dwBuildNumber);
            osFullVersion = osMajorVersion + "." + buildLabEx;
        }
    }

    /**
     * Get App Id
     */
    std::string getAppId()
    {
        // Auto-detect the app name - executable name without .exe suffix
        char buff[MAX_PATH] = { 0 };
        std::string appId;
        HMODULE handle = nullptr;
        if (::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, nullptr, &handle) != FALSE)
        {
            if (::GetModuleFileNameA(handle, &buff[0], MAX_PATH) > 0)
            {
                std::string  app_name(buff);
                size_t pos_dot = app_name.rfind(".");
                size_t pos_slash = app_name.rfind("\\");
                if ((pos_dot != std::string::npos) && (pos_slash != std::string::npos) && (pos_dot > pos_slash))
                {
                    app_name = app_name.substr(pos_slash + 1, (pos_dot - pos_slash) - 1);
                }
                appId = app_name;
            }
        }
        else
        {
            LOG_ERROR("Failed to get module handle with error: %d", ::GetLastError());
        }
        return appId;
    }

    SystemInformationImpl::SystemInformationImpl(IRuntimeConfig& /*configuration*/)
        : m_info_helper()
    {

        m_user_timezone = WindowsEnvironmentInfo::GetTimeZone();
        m_os_name = WindowsOSName;
        m_app_id = getAppId();
        m_app_version = getAppVersion();

        getOsVersion(m_os_major_version, m_os_full_version);
#ifdef WIN_DESKTOP
        m_device_class = "Windows";
#else
        HMODULE hNtDll = ::GetModuleHandle(TEXT("ntdll.dll"));
        typedef HRESULT NTSTATUS;
        typedef NTSTATUS(__stdcall * RtlConvertDeviceFamilyInfoToString_t)(unsigned long*, unsigned long*, PWSTR, PWSTR);
        RtlConvertDeviceFamilyInfoToString_t pRtlConvertDeviceFamilyInfoToString = hNtDll ? reinterpret_cast<RtlConvertDeviceFamilyInfoToString_t>(::GetProcAddress(hNtDll, "RtlConvertDeviceFamilyInfoToString")) : nullptr;

        if (pRtlConvertDeviceFamilyInfoToString)
        {
            unsigned long platformBufferSize = 0;
            unsigned long deviceClassBufferSize = 0;

            unsigned long status = pRtlConvertDeviceFamilyInfoToString(&platformBufferSize, &deviceClassBufferSize, nullptr, nullptr);
            if (status == 0xC0000023L) // #define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
            {
                auto platformString = std::make_unique<wchar_t[]>(platformBufferSize);
                auto deviceString = std::make_unique<wchar_t[]>(deviceClassBufferSize);
                pRtlConvertDeviceFamilyInfoToString(&platformBufferSize, &deviceClassBufferSize, platformString.get(), deviceString.get());
                std::wstring temp;
                temp.assign(platformString.get());
                m_device_class = to_utf8_string(temp);
            }
        }
#endif

        m_commercial_id = getCommercialId();
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

