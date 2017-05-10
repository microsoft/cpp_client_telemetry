#pragma once

#if defined(ARIASDK_PLATFORM_WINDOWS)

#define ARIASDK_LIBABI_CDECL __cdecl

#if defined(ARIASDK_SHARED_LIB)
#define ARIASDK_LIBABI __declspec(dllexport)
#elif defined(ARIASDK_STATIC_LIB)
#define ARIASDK_LIBABI
#else // Header file included by client
#define ARIASDK_LIBABI __declspec(dllexport)
#endif

#else // non-windows platform

#define ARIASDK_LIBABI_CDECL

#define ARIASDK_LIBABI

#endif
