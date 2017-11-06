#pragma warning( disable : 4996 )
#include "Version.hpp"
#include <Windows.h>
#include "ISystemInformation.hpp"
#include "pal/SystemInformationImpl.hpp"
#include "WindowsEnvironmentInfo.h"

#include <string>

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace PAL
            {
                const std::string WindowsOSName = "Windows Desktop";

				ISystemInformation* SystemInformationImpl::Create()
                {
                    return  new SystemInformationImpl();
                }

                SystemInformationImpl::SystemInformationImpl()
                    : m_info_helper()
                {
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

                    HMODULE hNtDll = ::GetModuleHandle(TEXT("ntdll.dll"));
                    typedef HRESULT NTSTATUS;
                    typedef NTSTATUS(__stdcall * RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
                    RtlGetVersion_t pRtlGetVersion = hNtDll ? reinterpret_cast<RtlGetVersion_t>(::GetProcAddress(hNtDll, "RtlGetVersion")) : nullptr;

                    RTL_OSVERSIONINFOW rtlOsvi = { sizeof(rtlOsvi) };
                    if (pRtlGetVersion && SUCCEEDED(pRtlGetVersion(&rtlOsvi)))
                    {
                       m_os_major_version = std::to_string((long long)rtlOsvi.dwMajorVersion) + "." + std::to_string((long long)rtlOsvi.dwMinorVersion);
                       m_os_full_version = m_os_major_version + "." + std::to_string((long long)rtlOsvi.dwBuildNumber);
                    }                   

                    typedef HRESULT NTSTATUS;
                    typedef NTSTATUS(__stdcall * RtlConvertDeviceFamilyInfoToString_t)(unsigned long*,unsigned long*,PWSTR,PWSTR);
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
            }
        }
    }
}