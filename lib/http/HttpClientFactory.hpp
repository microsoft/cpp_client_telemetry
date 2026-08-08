//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPCLIENTFACTORY_HPP
#define HTTPCLIENTFACTORY_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

namespace MAT_NS_BEGIN {

class HttpClientFactory
{
public:
    static std::shared_ptr<IHttpClient> Create();

private:
    MATSDK_LOG_DECL_COMPONENT_CLASS();
};

} MAT_NS_END

// TODO: [maxgolov] - remove this once there is a better way to pass HTTP client configuration
#if defined(MATSDK_PAL_WIN32) && !defined(_WINRT_DLL)
  #if defined(HAVE_MAT_WININET_HTTP_CLIENT)
    #include "http/HttpClient_WinInet.hpp"
  #else
    // WinHTTP is the default Win32 desktop transport: unlike WinInet, it does
    // not depend on a logged-on interactive user or that user's Internet
    // Explorer settings, so it works in services and other non-interactive
    // processes without extra configuration. Define HAVE_MAT_WININET_HTTP_CLIENT
    // to opt back into WinInet (e.g. for IE-integrated proxy/cookie behavior).
    #define HAVE_MAT_WINHTTP_HTTP_CLIENT
    #include "http/HttpClient_WinHttp.hpp"
  #endif
#endif

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTFACTORY_HPP
