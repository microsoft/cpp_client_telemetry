//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

// Assume that if we are compiling with MSVC, then we prefer to use Windows HTTP stack,
// e.g. WinInet.dll or Win 10 HTTP client instead
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)

#import <Foundation/Foundation.h>
#import <CFNetwork/CFNetwork.h>

#include "HttpClient_Apple.hpp"
#include "utils/StringUtils.hpp"
#include "utils/Utils.hpp"

namespace MAT_NS_BEGIN {

static std::string NextReqId()
{
    static std::atomic<uint64_t> seq;
    return std::string("REQ-") + std::to_string(seq.fetch_add(1));
}

static std::string NextRespId()
{
    static std::atomic<uint64_t> seq;
    return std::string("RESP-") + std::to_string(seq.fetch_add(1));
}

class HttpRequestApple : public SimpleHttpRequest
{
public:
    HttpRequestApple(HttpClient_Apple* parent) :
        SimpleHttpRequest(NextReqId()),
        m_parent(parent)
    {
        m_parent->Add(static_cast<IHttpRequest*>(this));
    }

    ~HttpRequestApple() noexcept
    {
        m_parent->Erase(static_cast<IHttpRequest*>(this));
    }

    void SendAsync(IHttpResponseCallback* callback)
    {
        m_callback = callback;
        NSString* url = [[NSString alloc] initWithUTF8String:m_url.c_str()];
        NSURLSessionConfiguration* sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
        m_urlRequest = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:url]];
        m_session = [NSURLSession sessionWithConfiguration:sessionConfig];
        m_session.sessionDescription = url;

        for(const auto& header : m_headers)
        {
            NSString* name = [[NSString alloc] initWithUTF8String:header.first.c_str()];
            NSString* value = [[NSString alloc] initWithUTF8String:header.second.c_str()];
            [m_urlRequest setValue:value forHTTPHeaderField:name];
        }

        m_completionMethod =
            ^(NSData *data, NSURLResponse *response, NSError *error)
            {
                HandleResponse(data, response, error);
            };

        if(equalsIgnoreCase(m_method, "get"))
        {
            [m_urlRequest setHTTPMethod:@"GET"];
            m_dataTask = [m_session dataTaskWithRequest:m_urlRequest completionHandler:m_completionMethod];
        }
        else
        {
            [m_urlRequest setHTTPMethod:@"POST"];
            NSData* postData = [NSData dataWithBytes:m_body.data() length:m_body.size()];
            m_dataTask = [m_session uploadTaskWithRequest:m_urlRequest fromData:postData completionHandler:m_completionMethod];
        }

        [m_dataTask resume];
    }

    void HandleResponse(NSData* data, NSURLResponse* response, NSError* error)
    {
        NSHTTPURLResponse *httpResp = static_cast<NSHTTPURLResponse*>(response);
        auto simpleResponse = new SimpleHttpResponse { NextRespId() };

        simpleResponse->m_statusCode = httpResp.statusCode;

        NSDictionary *responseHeaders = [httpResp allHeaderFields];
        for (id key in responseHeaders)
        {
            simpleResponse->m_headers.add([key UTF8String], [responseHeaders[key] UTF8String]);
        }

        if (error)
        {
            NSString* errorDomain = [error domain];
            long errorCode = [error code];

            if ([errorDomain isEqualToString:@"NSURLErrorDomain"] && (errorCode == NSURLErrorCancelled))
            {
                simpleResponse->m_result = HttpResult_Aborted;
            }
            else
            {
                LOG_TRACE("HTTP response error code: %li", errorCode);
                simpleResponse->m_result = HttpResult_NetworkFailure;
            }
        }
        else
        {
            simpleResponse->m_result = HttpResult_OK;
            auto body = static_cast<const uint8_t*>([data bytes]);
            simpleResponse->m_body.reserve(data.length);
            std::copy(body, body + data.length, std::back_inserter(simpleResponse->m_body));
        }
        m_callback->OnHttpResponse(simpleResponse);
    }

    void Cancel()
    {
        [m_dataTask cancel];
        [m_session getTasksWithCompletionHandler:^(NSArray* dataTasks, NSArray* uploadTasks, NSArray* downloadTasks)
        {
            for (NSURLSessionTask* _task in dataTasks)
            {
                [_task cancel];
            }

            for (NSURLSessionTask* _task in downloadTasks)
            {
                [_task cancel];
            }

            for (NSURLSessionTask* _task in uploadTasks)
            {
                [_task cancel];
            }
        }];
    }

private:
    HttpClient_Apple* m_parent = nullptr;
    IHttpResponseCallback* m_callback = nullptr;
    NSURLSession* m_session = nullptr;
    NSURLSessionDataTask* m_dataTask = nullptr;
    NSMutableURLRequest* m_urlRequest = nullptr;
    void (^m_completionMethod)(NSData* data, NSURLResponse* response, NSError* error);
};

HttpClient_Apple::HttpClient_Apple()
{
    LOG_TRACE("Initializing HttpClient_Apple...");
}

HttpClient_Apple::~HttpClient_Apple() noexcept
{
    LOG_TRACE("Shutting down HttpClient_Apple...");
}

IHttpRequest* HttpClient_Apple::CreateRequest()
{
    auto request = new HttpRequestApple(this);
    LOG_TRACE("HTTP request=%p id=%s created", request, request->GetId().c_str());
    return request;
}

void HttpClient_Apple::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    auto requestApple = static_cast<HttpRequestApple*>(request);
    requestApple->SendAsync(callback);
    LOG_TRACE("HTTP request=%p callback=%p sent", request, callback);
}

void HttpClient_Apple::CancelRequestAsync(const std::string& id)
{
    HttpRequestApple* request = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_requestsMtx);
        if (m_requests.find(id) != m_requests.cend())
        {
            request = static_cast<HttpRequestApple*>(m_requests[id]);
            if (request != nullptr)
            {
                LOG_TRACE("HTTP request=%p id=%s being aborted...", request, id.c_str());
                request->Cancel();
            }
            m_requests.erase(id);
        }
    }
}

void HttpClient_Apple::CancelAllRequests()
{
    std::vector<std::string> ids;
    {
        std::lock_guard<std::mutex> lock(m_requestsMtx);
        for (auto const& item : m_requests) {
            ids.push_back(item.first);
        }
    }

    for (const auto &id : ids)
        CancelRequestAsync(id);

    while (!m_requests.empty())
    {
        PAL::sleep(100);
        std::this_thread::yield();
    }
}

void HttpClient_Apple::Erase(IHttpRequest* req)
{
    std::lock_guard<std::mutex> lock(m_requestsMtx);
    m_requests.erase(req->GetId());
}

void HttpClient_Apple::Add(IHttpRequest* req)
{
    std::lock_guard<std::mutex> lock(m_requestsMtx);
    m_requests[req->GetId()] = req;
}

} MAT_NS_END

#endif

