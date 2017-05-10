#pragma warning( disable : 4996 )
#include "..\..\include\aria\Version.hpp"
#include <Windows.h>
#include "..\..\lib\include\aria\ISystemInformation.hpp"
#include "..\..\lib\pal\SystemInformationImpl.hpp"
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

                    OSVERSIONINFO osvi = { 0 };
                    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                    // FIXME: [MG] - C4996 'GetVersionExW': was declared deprecated
                    GetVersionEx(&osvi);

                    m_os_major_version = std::to_string((long long)osvi.dwMajorVersion) + "." + std::to_string((long long)osvi.dwMinorVersion);
                    m_os_full_version = m_os_major_version + "." + std::to_string((long long)osvi.dwBuildNumber);
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