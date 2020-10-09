//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#define LOG_MODULE DBG_DLLLOAD
// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"
#include "mat/config.h"

#ifdef HAVE_MAT_ZLIB
#pragma comment(lib, "zlib.lib") 
#endif

#ifdef HAVE_MAT_STORAGE
#pragma comment(lib, "sqlite-uwp.lib")
#endif

unsigned thread_count = 0;

BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        //TRACE("DLL_PROCESS_ATTACH");
        break;
    case DLL_THREAD_ATTACH:
        thread_count++;
        //TRACE("DLL_THREAD_ATTACH [%u], tid=0x%x", thread_count, GetCurrentThreadId() );
        break;
    case DLL_THREAD_DETACH:
        thread_count--;
        //TRACE("DLL_THREAD_DETACH [%u], tid=0x%x", thread_count, GetCurrentThreadId() );
        break;
    case DLL_PROCESS_DETACH:
        //TRACE("DLL_PROCESS_DETACH");
        break;
    }
    return TRUE;
}
