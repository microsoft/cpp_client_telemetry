// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

namespace MAT_NS_BEGIN {


class HttpClientFactory
{
public:
    static IHttpClient* Create();

private:
    MATSDK_LOG_DECL_COMPONENT_CLASS();
};


} MAT_NS_END
