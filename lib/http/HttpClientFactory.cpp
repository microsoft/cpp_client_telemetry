// Copyright (c) Microsoft. All rights reserved.
#include "HttpClientFactory.hpp"

#include "pal/PAL.hpp"

#if ARIASDK_PAL_SKYPE
#include "http/HttpClient_HttpStack.hpp"
#elif ARIASDK_PAL_WIN32
#  ifdef _WINRT_DLL
#include "http/HttpClient_WinRt.hpp"
#  else
#include "http/HttpClient_WinInet.hpp"
#  endif
#elif ARIASDK_PAL_CPP11
#include "http/HttpClient.hpp"
#else
#error The library cannot work without an HTTP client implementation.
#endif

namespace ARIASDK_NS_BEGIN {

    ARIASDK_LOG_INST_COMPONENT_CLASS(HttpClientFactory, "EventsSDK.HttpClientFactory", "Events telemetry client - HttpClientFactory class");

#if ARIASDK_PAL_SKYPE
    // HttpStack client requires a pointer to skype's http_stack instance. This used to be a property on 'LogConfiguration',
    // but it was removed long ago in bd162b75f15867e93c8d8e8f7f09a26e140c2346 (8/14/2017), so this hasn't successfully
    // compiled for quite some time.
    IHttpClient* HttpClientFactory::Create() {
        LOG_TRACE("Creating HttpClient_HttpStack: Skype HTTP Stack (provided IHttpStack=%p)", configuration.skypeHttpStack);
        return new HttpClient_HttpStack(configuration.skypeHttpStack);
    }

#elif ARIASDK_PAL_WIN32
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
#elif ARIASDK_PAL_CPP11
    IHttpClient* HttpClientFactory::Create() {
        LOG_TRACE("Creating generic HttpClient");
        return new HttpClient();
    }

#else
#error The library cannot work without an HTTP client implementation.
#endif

} ARIASDK_NS_END
