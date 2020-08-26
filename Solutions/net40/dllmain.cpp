#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN

// Windows Header Files:
#include <windows.h>
// #include <Objbase.h>

/// <summary>
/// Prevent unloading DLL until we finalized FlushAndTeardown
/// </summary>
/// <returns></returns>
HRESULT __stdcall DllCanUnloadNow(void)
{
    //   LogManager::FlushAndTeardown();
    return S_OK;
}

/// <summary>
/// Lock Microsoft Telemetry DLL in memory until process shutdown
/// </summary>
void LockInMemory()
{
#ifndef _WINRT_DLL
    HMODULE mstelModule;
    ::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN, L"ClientTelemetry.dll", &mstelModule);
#endif
}

/// <summary>
/// Executed on DLL load and unload
/// </summary>
/// <param name="hModule">The h module.</param>
/// <param name="ul_reason_for_call">The ul reason for call.</param>
/// <param name="lpReserved">The lp reserved.</param>
/// <returns></returns>
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    UNREFERENCED_PARAMETER(lpReserved);
    UNREFERENCED_PARAMETER(hModule);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        LockInMemory();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
