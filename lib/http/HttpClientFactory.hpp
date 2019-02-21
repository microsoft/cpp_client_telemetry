// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

namespace ARIASDK_NS_BEGIN {


class HttpClientFactory
{
public:
    static IHttpClient* Create();

private:
    ARIASDK_LOG_DECL_COMPONENT_CLASS();
};


} ARIASDK_NS_END
