// Copyright (c) Microsoft. All rights reserved.
#ifndef HTTPCLIENTFACTORY_HPP
#define HTTPCLIENTFACTORY_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

namespace ARIASDK_NS_BEGIN {

class HttpClientFactory
{
public:
    static std::shared_ptr<IHttpClient> Create();

private:
    MATSDK_LOG_DECL_COMPONENT_CLASS();
};

} ARIASDK_NS_END

// TODO: [maxgolov] - remove this once there is a better way to pass HTTP client configuration
#if defined(MATSDK_PAL_WIN32) && !defined(_WINRT_DLL)
#define HAVE_MAT_WININET_HTTP_CLIENT
#include "http/HttpClient_WinInet.hpp"
#endif

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTFACTORY_HPP