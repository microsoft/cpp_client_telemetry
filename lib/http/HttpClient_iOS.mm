// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#import <Foundation/Foundation.h>
#import <CFNetwork/CFNetwork.h>

// Assume that if we are compiling with MSVC, then we prefer to use Windows HTTP stack,
// e.g. WinInet.dll or Win 10 HTTP client instead
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)

#include "Version.hpp"
#include "HttpClient.hpp"

namespace ARIASDK_NS_BEGIN {
   
    HttpClient::HttpClient()
    {
    }

    HttpClient::~HttpClient()
    {
    }

    IHttpRequest* HttpClient::CreateRequest()
    {
        return nullptr;
    }

    void HttpClient::SendRequestAsync(IHttpRequest*, IHttpResponseCallback*)
    {
    }

    void HttpClient::CancelRequestAsync(std::string const&)
    {
    }

} ARIASDK_NS_END

#endif
