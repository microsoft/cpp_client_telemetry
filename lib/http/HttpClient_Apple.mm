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

#include <atomic>
#include <mutex>

namespace
{
    thread_local bool isAppleDelegateCallback = false;

    class AppleDelegateCallbackScope final
    {
    public:
        AppleDelegateCallbackScope() :
            m_previous(isAppleDelegateCallback)
        {
            isAppleDelegateCallback = true;
        }

        ~AppleDelegateCallbackScope() noexcept
        {
            isAppleDelegateCallback = m_previous;
        }

    private:
        bool m_previous;
    };
}

// Streams the response body in bounded chunks and enforces MAX_HTTP_RESPONSE_SIZE.
// The completionHandler-based NSURLSession APIs fully materialize the response body
// as an NSData before handing it over, so an attacker-controlled collector could force
// a large allocation. This delegate instead accumulates data incrementally in
// didReceiveData: and cancels the transfer as soon as the cap would be exceeded, so no
// more than the cap is ever buffered. Delegate callbacks may arrive on the session's
// delegate queue while a request thread registers a task, so shared state is guarded.
@interface MATStreamingSessionDelegate : NSObject <NSURLSessionDataDelegate>
- (BOOL)registerTask:(NSURLSessionTask*)task
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

- (BOOL)registerTask:(NSURLSessionTask*)task
             handler:(void (^)(NSData*, NSURLResponse*, NSError*))handler
{
    NSNumber* key = @(task.taskIdentifier);
    NSMutableData* buffer = [NSMutableData new];
    id copiedHandler = [handler copy];
    if (buffer == nil || copiedHandler == nil)
    {
        return NO;
    }
    @synchronized(self)
    {
        @try
        {
            _buffers[key] = buffer;
            _handlers[key] = copiedHandler;
            return YES;
        }
        @catch (NSException* exception)
        {
            (void)exception;
            [_buffers removeObjectForKey:key];
            [_handlers removeObjectForKey:key];
            return NO;
        }
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
    AppleDelegateCallbackScope callbackScope;
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
        bool cancelledBeforeSend = false;
        bool registered = false;
        NSURLSessionDataTask* task = nil;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_callback = callback;
            cancelledBeforeSend = m_cancelRequested;
        }
        if (cancelledBeforeSend)
        {
            // A Cancel() raced ahead of SendAsync and only set the flag (it never
            // completes on its own because there was no callback yet). Now that the
            // callback is published we own the single terminal Aborted.
            Complete(HttpResult_Aborted);
            return;
        }

        @try
        {
            @autoreleasepool
            {
                NSString* url = [[NSString alloc] initWithUTF8String:m_url.c_str()];
                NSURL* nsUrl = (url != nil) ? [NSURL URLWithString:url] : nil;
                if (nsUrl == nil || nsUrl.scheme == nil)
                {
                    Complete(HttpResult_LocalFailure);
                    return;
                }

                NSMutableURLRequest* urlRequest = [[NSMutableURLRequest alloc] initWithURL:nsUrl];
                if (urlRequest == nil)
                {
                    Complete(HttpResult_LocalFailure);
                    return;
                }

                for(const auto& header : m_headers)
                {
                    NSString* name = [[NSString alloc] initWithUTF8String:header.first.c_str()];
                    NSString* value = [[NSString alloc] initWithUTF8String:header.second.c_str()];
                    if (name == nil || value == nil)
                    {
                        Complete(HttpResult_LocalFailure);
                        return;
                    }
                    [urlRequest setValue:value forHTTPHeaderField:name];
                }

                m_completionMethod =
                    ^(NSData *data, NSURLResponse *response, NSError *error)
                    {
                        HandleResponse(data, response, error);
                    };

                if (session == nil || sessionDelegate == nil)
                {
                    Complete(HttpResult_NetworkFailure);
                    return;
                }

                if(equalsIgnoreCase(m_method, "get"))
                {
                    [urlRequest setHTTPMethod:@"GET"];
                    task = [session dataTaskWithRequest:urlRequest];
                }
                else
                {
                    [urlRequest setHTTPMethod:@"POST"];
                    NSData* postData = [NSData dataWithBytes:m_body.data() length:m_body.size()];
                    task = [session uploadTaskWithRequest:urlRequest fromData:postData];
                }

                if (task == nil || m_completionMethod == nil)
                {
                    Complete(HttpResult_LocalFailure);
                    return;
                }

                m_urlRequest = urlRequest;

                // Publish the task under the lock so a concurrent Cancel() can reach
                // and cancel it, and observe a cancel that raced with setup.
                bool cancelledDuringSetup = false;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_dataTask = task;
                    cancelledDuringSetup = m_cancelRequested;
                }
                if (cancelledDuringSetup)
                {
                    [task cancel];
                    Complete(HttpResult_Aborted);
                    return;
                }

                // Register before resume so the streaming delegate has the buffer and
                // completion handler in place before any response data arrives.
                registered = [sessionDelegate registerTask:task handler:m_completionMethod];
                if (!registered)
                {
                    bool cancelled = false;
                    {
                        std::lock_guard<std::mutex> lock(m_mutex);
                        cancelled = m_cancelRequested;
                    }
                    [task cancel];
                    Complete(cancelled ? HttpResult_Aborted : HttpResult_LocalFailure);
                    return;
                }

                bool cancelledAfterRegister = false;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    cancelledAfterRegister = m_cancelRequested;
                }
                if (cancelledAfterRegister)
                {
                    // The task is already registered, so let didCompleteWithError:
                    // be the sole terminal producer. Cancelling a suspended task is
                    // enough to drive that completion on Apple runtimes, so do not
                    // resume it here.
                    [task cancel];
                    return;
                }
                [task resume];
            }
        }
        @catch (NSException* exception)
        {
            LOG_WARN("HTTP request setup failed: %s", [[exception reason] UTF8String]);
            bool cancelled = false;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                cancelled = m_cancelRequested;
            }
            if (registered)
            {
                [task cancel];
                return;
            }
            if (task != nil)
            {
                [task cancel];
            }
            Complete(cancelled ? HttpResult_Aborted : HttpResult_LocalFailure);
        }
    }

    void HandleResponse(NSData* data, NSURLResponse* response, NSError* error)
    {
        IHttpResponseCallback* callback = nullptr;
        bool cancelRequested = false;
        HttpClient_Apple* parent = m_parent;
        IHttpRequest* self = static_cast<IHttpRequest*>(this);
        const std::string requestId = GetId();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_terminal)
            {
                return;
            }
            m_terminal = true;
            callback = m_callback;
            cancelRequested = m_cancelRequested;
        }

        @autoreleasepool
        {
            NSHTTPURLResponse *httpResp =
                [response isKindOfClass:[NSHTTPURLResponse class]]
                    ? static_cast<NSHTTPURLResponse*>(response)
                    : nil;
            auto simpleResponse = new SimpleHttpResponse { requestId };

            simpleResponse->m_statusCode =
                (httpResp != nil) ? static_cast<unsigned int>(httpResp.statusCode) : 0;

            if (httpResp != nil)
            {
                NSDictionary *responseHeaders = [httpResp allHeaderFields];
                for (id key in responseHeaders)
                {
                    const char* keyString = [key UTF8String];
                    const char* valueString = [responseHeaders[key] UTF8String];
                    if (keyString != nullptr && valueString != nullptr)
                    {
                        simpleResponse->m_headers.add(keyString, valueString);
                    }
                }
            }

            if (cancelRequested)
            {
                simpleResponse->m_result = HttpResult_Aborted;
            }
            else if (error)
            {
                NSString* errorDomain = [error domain];
                long errorCode = [error code];

                if ([errorDomain isEqualToString:@"NSURLErrorDomain"] &&
                    errorCode == NSURLErrorCancelled)
                {
                    simpleResponse->m_result = HttpResult_Aborted;
                }
                else if ([errorDomain isEqualToString:@"NSURLErrorDomain"] &&
                         (errorCode == NSURLErrorBadURL ||
                          errorCode == NSURLErrorUnsupportedURL))
                {
                    simpleResponse->m_result = HttpResult_LocalFailure;
                }
                else if (httpResp == nil)
                {
                    simpleResponse->m_result = HttpResult_NetworkFailure;
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
            if (parent != nullptr)
            {
                // Remove the request from the parent map before the callback runs.
                // A concurrent CancelRequestAsync that already holds the parent mutex
                // must finish first, keeping this raw request alive while it calls
                // Cancel(); later cancels will not find the request at all. The
                // callback may delete the request, so this erase must happen first.
                parent->Erase(self);
            }
            if (callback != nullptr)
            {
                callback->OnHttpResponse(simpleResponse);
            }
            else
            {
                delete simpleResponse;
            }
        }
        // Do not touch `this` after invoking the callback: it may delete the request.
    }

    bool Cancel()
    {
        // Only set the flag and cancel the in-flight task; never invoke the callback
        // here. A cancel before SendAsync has no callback yet, so completing from
        // Cancel would claim the terminal transition with no one to notify. SendAsync
        // (or the task's own delegate completion) delivers the single Aborted.
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cancelRequested = true;
        if (m_dataTask != nil)
        {
            [m_dataTask cancel];
        }
        return m_callback == nullptr && m_dataTask == nil;
    }

private:
    void Complete(HttpResult result)
    {
        IHttpResponseCallback* callback = nullptr;
        HttpClient_Apple* parent = m_parent;
        IHttpRequest* self = static_cast<IHttpRequest*>(this);
        const std::string requestId = GetId();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_terminal)
            {
                return;
            }
            m_terminal = true;
            callback = m_callback;
        }

        auto response = new SimpleHttpResponse { requestId };
        response->m_statusCode = 0;
        response->m_result = result;
        if (parent != nullptr)
        {
            // Same ordering rule as HandleResponse(): deregister before invoking
            // the callback because the callback may delete the request.
            parent->Erase(self);
        }
        if (callback != nullptr)
        {
            callback->OnHttpResponse(response);
        }
        else
        {
            delete response;
        }
        // Do not touch `this` after invoking the callback: it may delete the request.
    }

    HttpClient_Apple* m_parent = nullptr;
    IHttpResponseCallback* m_callback = nullptr;
    NSURLSessionDataTask* m_dataTask = nullptr;
    NSMutableURLRequest* m_urlRequest = nullptr;
    void (^m_completionMethod)(NSData* data, NSURLResponse* response, NSError* error);
    // Guards m_callback, m_cancelRequested, m_dataTask and m_terminal so setup,
    // cancellation and the single terminal completion observe a consistent view.
    // The callback is always invoked outside this lock.
    std::mutex m_mutex;
    bool m_cancelRequested = false;
    bool m_terminal = false;
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
    // Hold the requests mutex across Cancel(): Cancel() only flips the per-request
    // flag and cancels the NSURLSession task, and never completes synchronously.
    // That lets the mutex pin the raw request lifetime while we touch it. A request
    // that has never started has no callback capable of removing it, so retire it
    // here; a later SendAsync still observes its cancel flag and delivers Aborted.
    std::lock_guard<std::mutex> lock(m_requestsMtx);
    auto it = m_requests.find(id);
    if (it != m_requests.cend())
    {
        auto* request = static_cast<HttpRequestApple*>(it->second);
        if (request != nullptr)
        {
            LOG_TRACE("HTTP request=%p id=%s being aborted...", request, id.c_str());
            if (request->Cancel())
            {
                m_requests.erase(it);
            }
        }
    }
}

void HttpClient_Apple::CancelAllRequests()
{
    // NSURLSession serializes delegate callbacks when delegateQueue is nil. A
    // callback may cancel its peers, but it cannot wait for their callbacks to
    // drain without blocking the only queue that can deliver them.
    const bool waitForDrain = !isAppleDelegateCallback;
    for (;;)
    {
        std::vector<std::string> ids;
        {
            std::lock_guard<std::mutex> lock(m_requestsMtx);
            if (m_requests.empty())
            {
                return;
            }
            for (auto const& item : m_requests)
            {
                ids.push_back(item.first);
            }
        }
        for (const auto& id : ids)
        {
            CancelRequestAsync(id);
        }
        if (!waitForDrain)
        {
            return;
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
