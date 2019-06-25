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
    static IHttpClient* Create();

private:
    MATSDK_LOG_DECL_COMPONENT_CLASS();
};

} ARIASDK_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTFACTORY_HPP