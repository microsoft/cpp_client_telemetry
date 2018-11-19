#pragma once
#include "pal/UtcHelpers.hpp"
//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <winstring.h>
#include <Windows.Foundation.h>
#include <windows.system.diagnostics.telemetry.h>

#include <wrl.h>
#include <wrl\client.h>
#include <wrl\implements.h>

const int WINDOWS_MAJOR_VERSION = 10;
const int WINDOWS_BUILD_WITH_UTC_CHANGES = 15005;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::System::Diagnostics::Telemetry;

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace std;

namespace PAL_NS_BEGIN {

    bool IsUtcRegistrationEnabledinWindows()
    {
        HMODULE hNtDll = ::GetModuleHandle(TEXT("ntdll.dll"));
        typedef HRESULT NTSTATUS;
        typedef NTSTATUS(__stdcall * RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
        RtlGetVersion_t pRtlGetVersion = hNtDll ? reinterpret_cast<RtlGetVersion_t>(::GetProcAddress(hNtDll, "RtlGetVersion")) : nullptr;

        RTL_OSVERSIONINFOW rtlOsvi = { sizeof(rtlOsvi) };
        if (pRtlGetVersion && SUCCEEDED(pRtlGetVersion(&rtlOsvi)))
        {
            if (rtlOsvi.dwMajorVersion >= WINDOWS_MAJOR_VERSION && rtlOsvi.dwBuildNumber >= WINDOWS_BUILD_WITH_UTC_CHANGES)
            {
                return true;
            }
        }
        return false;
    }

    bool RegisterIkeyWithWindowsTelemetry(std::string const& ikeyin, int storageSize, int uploadQuotaSize)
    {
        // Initialize the Windows Runtime if it hasn't been initialized yet
        RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);

        // Get the activation factory for the IUriRuntimeClass interface.
        ComPtr<IPlatformTelemetryClientStatics> clientFactory;
        HRESULT hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_System_Diagnostics_Telemetry_PlatformTelemetryClient).Get(), &clientFactory);
        if (FAILED(hr))
        {
            return false;
        }

        int size = MultiByteToWideChar(CP_UTF8, 0, ikeyin.c_str(), (int)ikeyin.size(), NULL, 0);
        std::wstring wstrTo(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, ikeyin.c_str(), (int)ikeyin.size(), &wstrTo[0], size);
        HString ikey;
        hr = ikey.Set(wstrTo.c_str(), (DWORD)wstrTo.size());


        PlatformTelemetryRegistrationStatus status = PlatformTelemetryRegistrationStatus_UnknownFailure;

        ComPtr<IPlatformTelemetryRegistrationSettings> settings;
        hr = RoActivateInstance(HStringReference(RuntimeClass_Windows_System_Diagnostics_Telemetry_PlatformTelemetryRegistrationSettings).Get(), &settings);
        if (FAILED(hr))
        {// could not create settings, register without settings
            ComPtr<IPlatformTelemetryRegistrationResult> result;

            hr = clientFactory->Register(ikey.Get(), &result);
            if (FAILED(hr))
            {
                return false;
            }
            result->get_Status(&status);
        }
        else
        { //register with settings
            settings->put_StorageSize(storageSize);
            settings->put_UploadQuotaSize(uploadQuotaSize);
            ComPtr<IPlatformTelemetryRegistrationResult> result;

            hr = clientFactory->RegisterWithSettings(ikey.Get(), settings.Get(), &result);
            if (FAILED(hr))
            {
                return false;
            }
            result->get_Status(&status);
        }

        if (PlatformTelemetryRegistrationStatus_Success == status)
        {
            //		cout << "\n\n\n\nSuCCESS\\n\n\n" << endl;
            return true;
        }

        return false;
    }

} PAL_NS_END