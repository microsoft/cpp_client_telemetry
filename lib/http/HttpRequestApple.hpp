//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPREQUESTAPPLE_H
#define HTTPREQUESTAPPLE_H

#import <Foundation/Foundation.h>

#include "IHttpClient.hpp"
#include "HttpClient_Apple.hpp"

namespace MAT_NS_BEGIN {

    class HttpRequestApple : public SimpleHttpRequest {
    public:
        HttpRequestApple(HttpClient_Apple* parent);
        ~HttpRequestApple() noexcept;

        void SendAsync(IHttpResponseCallback* callback);
        void HandleResponse(NSData* data, NSURLResponse* response, NSError* error);
        void Cancel();

    private:
        HttpClient_Apple* m_parent = nullptr;
        IHttpResponseCallback* m_callback = nullptr;
        NSURLSessionDataTask* m_dataTask = nullptr;
        NSMutableURLRequest* m_urlRequest = nullptr;
        void (^m_completionMethod)(NSData* data, NSURLResponse* response, NSError* error);
    };

} MAT_NS_END

#endif // HTTPREQUESTAPPLE_H
