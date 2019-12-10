// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "HttpClientFactory.hpp"
#include "pal/PAL.hpp"

#if defined(MATSDK_PAL_WIN32)
  #ifdef _WINRT_DLL
    #include "http/HttpClient_WinRt.hpp"
  #elif defined(HAVE_MAT_WININET_HTTP_CLIENT)
    #include "http/HttpClient_WinInet.hpp"
  #endif
#elif defined(MATSDK_PAL_CPP11)
  #if defined(IOS_HTTP)
    #include "http/HttpClient_iOS.hpp"
  #else
    #include "http/HttpClient_Curl.hpp"
  #endif
#else
  #error The library cannot work without an HTTP client implementation.
#endif

namespace ARIASDK_NS_BEGIN {

    MATSDK_LOG_INST_COMPONENT_CLASS(HttpClientFactory, "EventsSDK.HttpClientFactory", "Events telemetry client - HttpClientFactory class");

#if defined(MATSDK_PAL_WIN32)
#ifdef _WINRT_DLL
    /* Win 10 HTTP client */
    std::shared_ptr<IHttpClient> HttpClientFactory::Create() {
        LOG_TRACE("Creating HttpClient_WinRt");
        return std::make_shared<HttpClient_WinRt>();
    }
#elif defined(HAVE_MAT_WININET_HTTP_CLIENT)
    /* Win32 WinInet HTTP client */
    std::shared_ptr<IHttpClient> HttpClientFactory::Create() {
        LOG_TRACE("Creating HttpClient_WinInet");
        return std::make_shared<HttpClient_WinInet>();
    }

#endif
#elif defined(MATSDK_PAL_CPP11)
#if defined(IOS_HTTP)
    std::shared_ptr<IHttpClient> HttpClientFactory::Create() {
        LOG_TRACE("Creating HttpClient_iOS");
        return std::make_shared<HttpClient_iOS>();
    }
#else
    std::shared_ptr<IHttpClient> HttpClientFactory::Create() {
        LOG_TRACE("Creating HttpClient_Curl");
        return std::make_shared<HttpClient_Curl>();
    }
#endif
#else
#error The library cannot work without an HTTP client implementation.
#endif

} ARIASDK_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
