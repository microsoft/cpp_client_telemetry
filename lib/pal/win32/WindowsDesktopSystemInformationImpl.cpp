#pragma warning( disable : 4996 )
#pragma comment (lib, "Mincore.lib")

#include <pal/PAL.hpp>

#include <Windows.h>

#include "ISystemInformation.hpp"
#include "pal/SystemInformationImpl.hpp"

#include "WindowsEnvironmentInfo.h"

#include <string>

namespace PAL_NS_BEGIN {

    const std::string WindowsOSName = "Windows Desktop";

    ISystemInformation* SystemInformationImpl::Create()
    {
        return  new SystemInformationImpl();
    }

    /**
     * Get executable full path as wide string
     */
    std::wstring getAppFullPath()
    {
        DWORD maxBufferLength = 0x7fff;
        DWORD curBufferLength = MAX_PATH;
        bool getFileNameSucceeded = false;

        std::vector<TCHAR> curExeFullPathBuffer(curBufferLength);

        // Try increasing buffer size until it work or until we reach the maximum size of 0x7fff (32 Kb)
        do
        {
            curExeFullPathBuffer.resize(curBufferLength);

            DWORD result = GetModuleFileName(GetModuleHandle(NULL), &curExeFullPathBuffer[0], curBufferLength);

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
            return std::wstring(curExeFullPathBuffer.begin(), curExeFullPathBuffer.end());
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
        RegGetValueA(HKEY_LOCAL_MACHINE, c_currentVersion_Key, c_buildLabEx_ValueName, RRF_RT_REG_SZ, NULL, (char*)buff, &size);
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

        RTL_OSVERSIONINFOW rtlOsvi = { sizeof(rtlOsvi) };
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
        if (GetModuleFileNameA(GetModuleHandle(NULL), &buff[0], MAX_PATH) > 0)
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
        return appId;
    }

    SystemInformationImpl::SystemInformationImpl()
        : m_info_helper()
    {
#if 1
        m_user_timezone = WindowsEnvironmentInfo::GetTimeZone();
        m_os_name = WindowsOSName;
        m_app_id = getAppId();
        m_app_version = getAppVersion();

        getOsVersion(m_os_major_version, m_os_full_version);

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
                std::string str(temp.begin(), temp.end());
                m_device_family = str;
            }
        }
#else   // FIXME: [MG] - figure out which implementation is better
        m_user_timezone = WindowsEnvironmentInfo::GetTimeZone();
        m_os_name = WindowsOSName;

        // Auto-detect the app name - executable name without .exe suffix
        char buff[MAX_PATH] = { 0 };
        if (GetModuleFileNameA(GetModuleHandle(NULL), &buff[0], MAX_PATH) > 0)
        {
            //std::wstring app_name_w(buff);
            std::string  app_name(buff);// app_name_w.begin(), app_name_w.end());
            size_t pos_dot = app_name.rfind(".");
            size_t pos_slash = app_name.rfind("\\");
            if ((pos_dot != std::string::npos) && (pos_slash != std::string::npos) && (pos_dot > pos_slash))
            {
                app_name = app_name.substr(pos_slash + 1, (pos_dot - pos_slash) - 1);
            }
            m_app_id = app_name;
        }

        OSVERSIONINFO osvi = { 0 };
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        // FIXME: [MG] - C4996 'GetVersionExW': was declared deprecated
        GetVersionEx(&osvi);

        m_os_major_version = std::to_string((long long)osvi.dwMajorVersion) + "." + std::to_string((long long)osvi.dwMinorVersion);
        m_os_full_version = m_os_major_version + "." + std::to_string((long long)osvi.dwBuildNumber);
#endif
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
