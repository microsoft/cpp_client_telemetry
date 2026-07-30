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

// Streams the response body in bounded chunks and enforces MAX_HTTP_RESPONSE_SIZE.
// The completionHandler-based NSURLSession APIs fully materialize the response body
// as an NSData before handing it over, so an attacker-controlled collector could force
// a large allocation. This delegate instead accumulates data incrementally in
// didReceiveData: and cancels the transfer as soon as the cap would be exceeded, so no
// more than the cap is ever buffered. Delegate callbacks may arrive on the session's
// delegate queue while a request thread registers a task, so shared state is guarded.
@interface MATStreamingSessionDelegate : NSObject <NSURLSessionDataDelegate>
- (void)registerTask:(NSURLSessionTask*)task
             handler:(void (^)(NSData* data, NSURLResponse* response, NSError* error))handler;
@end

@implementation MATStreamingSessionDelegate {
    NSMutableDictionary<NSNumber*, NSMutableData*>* _buffers;
    NSMutableDictionary<NSNumber*, id>*             _handlers;
    NSMutableSet<NSNumber*>*                        _overCap;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        _buffers = [NSMutableDictionary new];
        _handlers = [NSMutableDictionary new];
        _overCap = [NSMutableSet new];
    }
    return self;
}

- (void)registerTask:(NSURLSessionTask*)task
             handler:(void (^)(NSData*, NSURLResponse*, NSError*))handler
{
    NSNumber* key = @(task.taskIdentifier);
    @synchronized(self)
    {
        _buffers[key] = [NSMutableData new];
        _handlers[key] = [handler copy];
    }
}

- (void)URLSession:(NSURLSession*)session
          dataTask:(NSURLSessionDataTask*)dataTask
    didReceiveData:(NSData*)data
{
    NSNumber* key = @(dataTask.taskIdentifier);
    @synchronized(self)
    {
        if ([_overCap containsObject:key])
        {
            return;
        }
        NSMutableData* buffer = _buffers[key];
        if (buffer == nil)
        {
            return;
        }
        if (buffer.length + data.length > MAT::MAX_HTTP_RESPONSE_SIZE)
        {
            // Refuse the over-large response: stop buffering and cancel the transfer.
            [_overCap addObject:key];
            [dataTask cancel];
            return;
        }
        [buffer appendData:data];
    }
}

- (void)URLSession:(NSURLSession*)session
              task:(NSURLSessionTask*)task
didCompleteWithError:(NSError*)error
{
    NSNumber* key = @(task.taskIdentifier);
    void (^handler)(NSData*, NSURLResponse*, NSError*) = nil;
    NSData* body = nil;
    BOOL overCap = NO;
    @synchronized(self)
    {
        handler = (void (^)(NSData*, NSURLResponse*, NSError*))_handlers[key];
        body = _buffers[key];
        overCap = [_overCap containsObject:key];
        [_handlers removeObjectForKey:key];
        [_buffers removeObjectForKey:key];
        [_overCap removeObject:key];
    }
    if (handler == nil)
    {
        return;
    }
    if (overCap)
    {
        // Surface a non-cancellation error so the request maps to NetworkFailure
        // (retried), not Aborted (which is reserved for caller-initiated cancels).
        NSError* capError = [NSError errorWithDomain:@"MATResponseCap"
                                                code:-1
                                            userInfo:@{ NSLocalizedDescriptionKey : @"HTTP response exceeds max buffered size" }];
        handler(nil, task.response, capError);
    }
    else
    {
        handler(body, task.response, error);
    }
}
@end

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

static dispatch_once_t once;
static NSURLSession* session;
static MATStreamingSessionDelegate* sessionDelegate;

class HttpRequestApple : public SimpleHttpRequest
{
public:
    HttpRequestApple(HttpClient_Apple* parent) :
        SimpleHttpRequest(NextReqId()),
        m_parent(parent)
    {
        m_parent->Add(static_cast<IHttpRequest*>(this));
        dispatch_once(&once, ^{
            NSURLSessionConfiguration* sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
            sessionDelegate = [MATStreamingSessionDelegate new];
            session = [NSURLSession sessionWithConfiguration:sessionConfig
                                                    delegate:sessionDelegate
                                               delegateQueue:nil];
        });
    }

    ~HttpRequestApple() noexcept
    {
        m_parent->Erase(static_cast<IHttpRequest*>(this));
    }

    void SendAsync(IHttpResponseCallback* callback)
    {
        @autoreleasepool
        {
            m_callback = callback;
            NSString* url = [[NSString alloc] initWithUTF8String:m_url.c_str()];
            m_urlRequest = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:url]];

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
                m_dataTask = [session dataTaskWithRequest:m_urlRequest];
            }
            else
            {
                [m_urlRequest setHTTPMethod:@"POST"];
                NSData* postData = [NSData dataWithBytes:m_body.data() length:m_body.size()];
                m_dataTask = [session uploadTaskWithRequest:m_urlRequest fromData:postData];
            }

            // Register before resume so the streaming delegate has the buffer and
            // completion handler in place before any response data arrives.
            [sessionDelegate registerTask:m_dataTask handler:m_completionMethod];
            [m_dataTask resume];
        }
    }

    void HandleResponse(NSData* data, NSURLResponse* response, NSError* error)
    {
        @autoreleasepool
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
                // The streaming delegate has already enforced MAX_HTTP_RESPONSE_SIZE
                // (an over-cap response arrives here as a cap error, handled above), so
                // data is bounded. Guard against a nil/empty body to avoid pointer
                // arithmetic on a null [data bytes].
                simpleResponse->m_result = HttpResult_OK;
                const size_t length = static_cast<size_t>(data.length);
                if (length > 0)
                {
                    auto body = static_cast<const uint8_t*>([data bytes]);
                    simpleResponse->m_body.reserve(length);
                    std::copy(body, body + length, std::back_inserter(simpleResponse->m_body));
                }
            }
            m_callback->OnHttpResponse(simpleResponse);
        }
    }

    void Cancel()
    {
        [m_dataTask cancel];
    }

private:
    HttpClient_Apple* m_parent = nullptr;
    IHttpResponseCallback* m_callback = nullptr;
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

    for (;;)
    {
        {
            std::lock_guard<std::mutex> lock(m_requestsMtx);
            if (m_requests.empty())
            {
                return;
            }
        }
        PAL::sleep(100);
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
