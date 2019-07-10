// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "HttpClientFactory.hpp"
#include "pal/PAL.hpp"

#if defined(MATSDK_PAL_WIN32)
  #ifdef _WINRT_DLL
    #include "http/HttpClient_WinRt.hpp"
  #else
    #include "http/HttpClient_WinInet.hpp"
  #endif
#elif defined(MATSDK_PAL_CPP11)
  #include "http/HttpClient.hpp"
#else
  #error The library cannot work without an HTTP client implementation.
#endif

namespace ARIASDK_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(HttpClientFactory, "EventsSDK.HttpClientFactory", "Events telemetry client - HttpClientFactory class");

#if defined(MATSDK_PAL_WIN32)
#ifdef _WINRT_DLL
    IHttpClient* HttpClientFactory::Create() {
        LOG_TRACE("Creating HttpClient_WinRt");
        return new HttpClient_WinRt();
    }

#else
    IHttpClient* HttpClientFactory::Create() {
        LOG_TRACE("Creating HttpClient_WinInet");
        return new HttpClient_WinInet();
    }

#endif
#elif defined(MATSDK_PAL_CPP11)
    IHttpClient* HttpClientFactory::Create() {
        LOG_TRACE("Creating generic HttpClient");
        return new HttpClient();
    }

#else
#error The library cannot work without an HTTP client implementation.
#endif

} ARIASDK_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
