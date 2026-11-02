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

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTFACTORY_HPP
