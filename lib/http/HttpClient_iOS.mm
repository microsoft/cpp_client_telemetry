// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

// Assume that if we are compiling with MSVC, then we prefer to use Windows HTTP stack,
// e.g. WinInet.dll or Win 10 HTTP client instead
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)

#import <Foundation/Foundation.h>
#import <CFNetwork/CFNetwork.h>

#include "HttpClient.hpp"
#include "Utils.hpp"

namespace ARIASDK_NS_BEGIN {

static std::string NextReqId()
{
    static std::atomic<uint64_t> seq(0);
    return std::string("REQ-") + std::to_string(seq.fetch_add(1));
}

static std::string NextRespId()
{
    static std::atomic<uint64_t> seq(0);
    return std::string("RESP-") + std::to_string(seq.fetch_add(1));
}

class HttpRequestIos : public SimpleHttpRequest
{
public:
    HttpRequestIos(HttpClient* parent) :
        SimpleHttpRequest(NextReqId()),
        m_parent(parent),
        m_response(NextRespId()),
        m_callback(nullptr),
        m_session(nullptr),
        m_dataTask(nullptr),
        m_urlRequest(nullptr)
    {
        m_parent->add(static_cast<IHttpRequest*>(this));
    }
    
    ~HttpRequestIos()
    {
        m_parent->erase(static_cast<IHttpRequest*>(this));
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

    void HandleResponse(NSData *data, NSURLResponse *response, NSError *error)
    {
        NSHTTPURLResponse *httpResp = static_cast<NSHTTPURLResponse*>(response);

        m_response.m_statusCode = httpResp.statusCode;

        NSDictionary *responseHeaders = [httpResp allHeaderFields];
        for (id key in responseHeaders)
        {
            m_response.m_headers.add([key UTF8String], [responseHeaders[key] UTF8String]);
        }

        if (error)
        {
            NSString* errorDomain = [error domain];
            long errorCode = [error code];

            if ([errorDomain isEqualToString:@"NSURLErrorDomain"] && (errorCode == NSURLErrorCancelled))
            {
                m_response.m_result = HttpResult_Aborted;
            }
            else
            {
                m_response.m_result = HttpResult_NetworkFailure;
            }
        }
        else
        {
            m_response.m_result = HttpResult_OK;
            auto body = static_cast<const uint8_t*>([data bytes]);
            m_response.m_body.reserve(data.length);
            std::copy(body, body + data.length, std::back_inserter(m_response.m_body));
        }
        m_callback->OnHttpResponse(&m_response);
    }

    void Cancel()
    {
        [m_dataTask cancel];
        [m_session getTasksWithCompletionHandler:^(NSArray *dataTasks, NSArray *uploadTasks, NSArray *downloadTasks)
        {
            for (NSURLSessionTask *_task in dataTasks)
            {
                [_task cancel];
            }

            for (NSURLSessionTask *_task in downloadTasks)
            {
                [_task cancel];
            }

            for (NSURLSessionTask *_task in uploadTasks)
            {
                [_task cancel];
            }
        }];
    }

private:
    HttpClient* m_parent;
    SimpleHttpResponse m_response;
    IHttpResponseCallback* m_callback;
    NSURLSession* m_session;
    NSURLSessionDataTask* m_dataTask;
    NSMutableURLRequest* m_urlRequest;
    void (^m_completionMethod)(NSData *data, NSURLResponse *response, NSError *error);
};

HttpClient::HttpClient()
{
    LOG_TRACE("Initializing HttpClient...");
}

HttpClient::~HttpClient()
{
    LOG_TRACE("Shutting down HttpClient...");
}

IHttpRequest* HttpClient::CreateRequest()
{
    auto request = new HttpRequestIos(this);
    LOG_TRACE("HTTP request=%p id=%s created", request, request->GetId().c_str());
    return request;
}

void HttpClient::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    auto requestIos = static_cast<HttpRequestIos*>(request);
    requestIos->SendAsync(callback);
    LOG_TRACE("HTTP request=%p callback=%p sent", request, callback);
}

void HttpClient::CancelRequestAsync(std::string const& id)
{
    HttpRequestIos* request = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_request_mtx);
        if (m_requests.find(id) != m_requests.cend())
        {
            request = static_cast<HttpRequestIos*>(m_requests[id]);
            if (request != nullptr)
            {
                LOG_TRACE("HTTP request=%p id=%s being aborted...", request, id.c_str());
                request->Cancel();
            }
            m_requests.erase(id);
        }
    }
}

} ARIASDK_NS_END

#endif
