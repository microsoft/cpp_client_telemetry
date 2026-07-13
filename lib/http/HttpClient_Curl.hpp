//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPCLIENTCURL_HPP
#define HTTPCLIENTCURL_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string.h>
#include <regex>

#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>

#include <algorithm>
#include <numeric>
#include <limits>
#include <atomic>
#include <thread>
#include <memory>
#include <utility>
#include <exception>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <poll.h>
#include <curl/curl.h>

#include <unistd.h>

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

#ifdef HAVE_ONEDS_BOUNDCHECK_METHODS
#include "utils/annex_k.hpp"
#endif

#define HTTP_CONN_TIMEOUT       5L
#define HTTP_STATUS_REGEXP		"HTTP\\/\\d\\.\\d (\\d+)\\ .*"
#define HTTP_HEADER_REGEXP      "(.*)\\: (.*)\\n*"

#undef TRACE
#define TRACE(...)	// printf

namespace MAT_NS_BEGIN {

// Tracks the number of in-flight CurlHttpOperations so ~HttpClient_Curl can wait for
// their detached-worker curl_easy_cleanup to finish before it runs
// curl_global_cleanup (the two must not run concurrently). If shutdown times
// out, abandonCallbacks tells late workers to skip callback/log dispatch.
struct CurlOperationTracker {
    std::mutex mtx;
    std::condition_variable cv;
    int inFlight = 0;
    std::atomic<bool> abandonCallbacks { false };
};

// State shared with detached curl worker callbacks. A worker can outlive
// HttpClient_Curl if shutdown's bounded drain times out, so callbacks must only
// touch this shared state and never capture/dereference the parent client.
struct CurlClientSharedState {
    CurlClientSharedState() :
        activeOps(std::make_shared<CurlOperationTracker>())
    {
    }

    std::mutex requestsMtx;
    std::map<std::string, IHttpRequest*> requests;
    std::string sslCaInfo;
    std::shared_ptr<CurlOperationTracker> activeOps;
};

/**
 * Curl-based HTTP client
 */
class HttpClient_Curl : public IHttpClient {
public:
    HttpClient_Curl();
    virtual ~HttpClient_Curl();

    virtual IHttpRequest* CreateRequest() override;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
    virtual void CancelRequestAsync(std::string const& id) override;

    virtual void ApplySettings(ILogConfiguration& config) override;
    void SetSslVerification(bool sslVerify, const std::string& caInfo = "");

private:
    void AddRequest(IHttpRequest* request);

    std::shared_ptr<CurlClientSharedState> m_state { std::make_shared<CurlClientSharedState>() };
    std::atomic<bool> m_sslVerify { true };
};

class CurlHttpOperation : public std::enable_shared_from_this<CurlHttpOperation> {
public:

    void DispatchEvent(HttpStateEvent type)
    {
        if (m_tracker && m_tracker->abandonCallbacks.load(std::memory_order_acquire))
        {
            return;
        }
        if (m_callback != nullptr)
        {
            m_callback->OnHttpStateEvent(type, static_cast<void*>(curl), 0);
        }
    }

    std::atomic<bool> isAborted { false };      // Set to 'true' when async callback is aborted

    // Set once the completion callback has run. After that point the externally
    // owned IHttpResponseCallback (m_callback) may already be destroyed, so it must
    // not be dispatched to again (see ~CurlHttpOperation).
    std::atomic<bool> m_completed { false };

    /**
     * Create local CURL instance for url and body
     *
     * @param url
     * @param body
     * @param httpConnTimeout   HTTP connection timeout in seconds
     * @param httpReadTimeout   HTTP read timeout in seconds
     */
    CurlHttpOperation(
            std::string method,
            std::string url,
            IHttpResponseCallback* callback,
            // requestHeaders is copied into the curl_slist during construction and
            // need not outlive this operation. requestBody is taken by value and
            // owned by this operation: the detached worker in SendAsync can outlive
            // the caller's request, so a reference into it could dangle during
            // Send().
            const std::map<std::string, std::string>& requestHeaders,
            std::vector<uint8_t> requestBody,
            // Default connectivity and response size options
            bool rawResponse                                         = false,
            size_t httpConnTimeout                                   = HTTP_CONN_TIMEOUT,
            // SSL certificate verification options
            bool sslVerify                                           = true,
            const std::string& sslCaInfo                             = "") :

            // Optional connection params
            rawResponse(rawResponse),
            httpConnTimeout(httpConnTimeout),

            m_callback(callback),
            m_method(method),
            m_url(url),
            m_sslCaInfo(sslCaInfo),

            // Local vars
            requestBody(std::move(requestBody))
    {
        TRACE("--------------------------------------------------------------------------------------------------\n");
        response.memory = nullptr;
        response.size = 0;

        /* get a curl handle */
        curl = curl_easy_init();
        if(!curl)
        {
            TRACE("libcurl failed to init!\n");
            res = CURLE_FAILED_INIT;
            DispatchEvent(OnCreateFailed);
            return;
        }

#if 0
        // Be verbose
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#else
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
#endif

        // Specify target URL
        curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, sslVerify ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, sslVerify ? 2L : 0L);
        if (!m_sslCaInfo.empty()) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, m_sslCaInfo.c_str());
        }
        // HTTP/2 please, fallback to HTTP/1.1 if not supported
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

        // Headers are copied into m_headersChunk during construction and the
        // curl_slist is kept alive until destruction, so the original map does
        // not need operation-lifetime storage.
        for (const auto& kv : requestHeaders)
        {
            std::string header = kv.first + ": " + kv.second;
            m_headersChunk = curl_slist_append(m_headersChunk, header.c_str());
        }

        if(m_headersChunk != nullptr)
        {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, m_headersChunk);
        }
        TRACE("method=%s, url=%s\n", this->m_method.c_str(), this->m_url.c_str());

        DispatchEvent(OnCreated);
    }

    /**
     * Destroy CURL instance
     */
    virtual ~CurlHttpOperation()
    {
        // When Send() ran asynchronously, it was on a detached worker that held a
        // shared_ptr to this operation (see SendAsync), so this destructor runs only
        // after that worker finished and released its reference; the curl handle,
        // response buffer and owned request body are then no longer in use. It can also
        // run without any async worker: for an operation that was never sent, or when
        // SendAsync fell back to a synchronous run on the caller's thread. There is no
        // future to join in any case, so destruction is safe on any thread -- including
        // the worker thread itself, which is where it happens when the callback drops
        // the last other reference.
        // OnDestroy is dispatched only when this operation is destroyed without its send
        // ever having run -- i.e. SendAsync was never called. Once RunSendAndCallback
        // runs it sets m_completed regardless of the result (even when Send() fails
        // immediately, e.g. curl_easy_init returns an error), and once the completion
        // callback has run m_callback may already be freed: synchronous-handler builds
        // delete the IHttpResponseCallback inside onHttpResponse, called from the
        // completion callback. Dispatching through it then would be a use-after-free, so
        // it is suppressed. (Consequently the curl client does not emit OnDestroy for a
        // request whose send was attempted.)
        if (!m_completed.load(std::memory_order_acquire))
        {
            DispatchEvent(OnDestroy);
        }
        res = CURLE_OK;
        curl_easy_cleanup(curl);
        curl_slist_free_all(m_headersChunk);
        ReleaseResponse();

        // Signal HttpClient_Curl that this operation's curl_easy_cleanup is done, so
        // its destructor can safely run curl_global_cleanup once all operations end.
        if (m_tracker)
        {
            std::lock_guard<std::mutex> lock(m_tracker->mtx);
            if (--m_tracker->inFlight == 0)
            {
                m_tracker->cv.notify_all();
            }
        }
    }

    // Associate this operation with HttpClient_Curl's in-flight tracker so its
    // lifetime (through the curl_easy_cleanup in the destructor above) is awaited
    // before curl_global_cleanup. Called once, before the async send starts.
    void trackWith(std::shared_ptr<CurlOperationTracker> tracker)
    {
        m_tracker = std::move(tracker);
        if (m_tracker)
        {
            std::lock_guard<std::mutex> lock(m_tracker->mtx);
            ++m_tracker->inFlight;
        }
    }

    /**
     * Send request synchronously
     */
    long Send()
    {
        TRACE("method=%s\n", this->m_method.c_str());

        ReleaseResponse();
        // Request buffer
        const void *request  = requestBody.empty() ? nullptr : requestBody.data();
        const size_t reqSize = requestBody.size();

        if(!curl)
        {
            res = CURLE_FAILED_INIT;
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }

        // TODO: should we control what local source port we use?
        // curl_easy_setopt(curl, CURLOPT_LOCALPORT, dcf_port);

        // Perform initial connect, handling the timeout if needed
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
        DispatchEvent(OnConnecting);
        res = curl_easy_perform(curl);
        if(CURLE_OK != res)
        {
            DispatchEvent(OnConnectFailed);     // couldn't connect - stage 1
            TRACE("Error #1: %s\n", curl_easy_strerror(res));
            goto cleanup;
        }

        /* Extract the socket from the curl handle - we'll need it for waiting.
         * Note that this API takes a pointer to a 'long' while we use
         * curl_socket_t for sockets otherwise.
         */

#if LIBCURL_VERSION_NUM >= 0x072D00 // Version 7.45.00
        res = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockextr);
#else
        res = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &sockextr);
#endif

        if(CURLE_OK != res)
        {
            DispatchEvent(OnConnectFailed);     // couldn't connect - stage 2
            TRACE("Error #2: %s\n", curl_easy_strerror(res));
            goto cleanup;
        }

        /* wait for the socket to become ready for sending */
        sockfd = sockextr;
        if( !WaitOnSocket(sockfd, 0, HTTP_CONN_TIMEOUT * 1000L) || isAborted)
        {
            TRACE("Error #3: timeout, aborted=%u\n", isAborted.load() );
            res = CURLE_OPERATION_TIMEDOUT;
            DispatchEvent(OnConnectFailed);     // couldn't connect - stage 3
            goto cleanup;
        }

        // once connection is there - switch back to easy perform for HTTP post
        curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 0);

        // send all data to our callback function
        if (rawResponse)
        {
            curl_easy_setopt(curl, CURLOPT_HEADER,        true);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void *)&WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,     (void *)&response);
        } else {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void *)&WriteVectorCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA,    (void *)&respHeaders);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,     (void *)&respBody);
        }

        // TODO: only two methods supported for now - POST and GET
        if (m_method.compare("POST") == 0)
        {
            // POST
            curl_easy_setopt(curl, CURLOPT_POST, true);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, static_cast<const char*>(request));
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, reqSize);
        } else
        if (m_method.compare("GET") == 0)
        {
            // GET
        } else
        {
            TRACE("Error #4: unsupported method %s\n", m_method.c_str());
            res = CURLE_UNSUPPORTED_PROTOCOL;
            goto cleanup;
        }

        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 4096);
        DispatchEvent(OnSending);
        res = curl_easy_perform(curl);
        if(CURLE_OK != res)
        {
            DispatchEvent(OnSendFailed);
            TRACE("Error: %s\n", curl_easy_strerror(res));
            goto cleanup;
        }

        /* Code snippet to parse raw HTTP response. This might come in handy
         * if we ever consider to handle the raw upload instead of curl_easy_perform
       ...
       std::string resp((const char *)response);
       std::regex http_status_regex(HTTP_STATUS_REGEXP);
       std::smatch match;
       if(std::regex_search(resp, match, http_status_regex))
         http_code = std::stol(match[1]);
       ...
         */

        /* libcurl is nice enough to parse the response code itself: */
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res);
        // We got some response from server. Dump the contents.
        TRACE("HTTP response code %d\n", res);
        DispatchEvent(OnResponse);

cleanup:

        // This function returns:
        // - on success: HTTP status code.
        // - on failure: CURL error code.
        // The two sets of enums (CURLE, HTTP codes) - do not intersect, so we collapse them in one set.
        return res;
    }

    // Runs the blocking Send() and then the callback, guaranteeing the callback is
    // invoked exactly once and that no exception escapes (a detached worker must not
    // let one escape -> std::terminate; std::async previously captured exceptions in
    // its never-observed future). Shared by the detached worker and the synchronous
    // fallbacks in SendAsync().
    void RunSendAndCallback(const std::function<void(CurlHttpOperation &)>& callback) {
        try
        {
            Send();
        }
        catch (const std::exception& e)
        {
            TRACE("CurlHttpOperation Send() failed by exception: %s\n", e.what());
            res = CURLE_FAILED_INIT;   // report a failure result to the callback
        }
        catch (...)
        {
            TRACE("CurlHttpOperation Send() failed by unknown exception\n");
            res = CURLE_FAILED_INIT;
        }
        // Invoke the callback even if Send() threw, so the operation is always
        // completed (with the failure result set above) and the request is never
        // left outstanding. Guard it so a throwing callback cannot escape either.
        if (callback != nullptr)
        {
            try
            {
                callback(*this);
            }
            catch (const std::exception& e)
            {
                TRACE("CurlHttpOperation callback threw: %s\n", e.what());
            }
            catch (...)
            {
                TRACE("CurlHttpOperation callback threw unknown exception\n");
            }
        }
        // The send has completed. The completion callback (if any) may have destroyed
        // the IHttpResponseCallback -- synchronous-handler builds run onHttpResponse,
        // which deletes it -- so m_callback must not be dispatched to after this point.
        // Set completion regardless of whether a callback was provided: a request that
        // was actually sent must never emit OnDestroy from the destructor.
        m_completed.store(true, std::memory_order_release);
    }

    void SendAsync(std::function<void(CurlHttpOperation &)> callback = nullptr) {
        // Run the blocking Send() on a detached worker that keeps this operation
        // alive for the duration by holding a shared_ptr to itself. This replaces
        // std::async, whose returned future joins its worker thread on destruction:
        // when the callback below caused this operation to be destroyed on the
        // async thread (OnHttpResponse -> EventsUploadContext::clear()), that join
        // was a self-join and raised std::system_error("Resource deadlock avoided")
        // out of the noexcept destructor, aborting the process. With
        // the self-keepalive there is no future and no join: the worker simply
        // exits, releasing the last reference, and ~CurlHttpOperation runs
        // trivially on whichever thread drops it.
        std::shared_ptr<CurlHttpOperation> self;
        try
        {
            self = shared_from_this();
        }
        catch (const std::bad_weak_ptr&)
        {
            // The detached-worker self-keepalive requires this operation to be owned
            // by a std::shared_ptr (it always is in practice -- created via
            // make_shared in HttpClient_Curl.cpp). If a future caller ever constructs
            // one outside a shared_ptr (stack / unique_ptr), shared_from_this() throws;
            // fall back to a synchronous run on the caller's thread rather than letting
            // std::bad_weak_ptr escape SendAsync(). The caller owns the object for the
            // duration and the callback is still invoked.
            RunSendAndCallback(callback);
            return;
        }
        try
        {
            // Constructing the worker lambda copies `callback` (a std::function,
            // which can throw std::bad_alloc), and std::thread construction can throw
            // std::system_error / std::bad_alloc -- both are inside this try. The
            // worker holds `self`, keeping this operation alive for the detached run.
            std::thread([self, callback]() { self->RunSendAndCallback(callback); }).detach();
        }
        catch (const std::exception& e)
        {
            // Building the callable or starting the worker thread failed. Run the
            // operation synchronously as a fallback so the IHttpClient callback is
            // still always invoked and the exception does not escape SendAsync().
            // `self` keeps this operation alive for the duration of the run.
            TRACE("CurlHttpOperation could not start worker thread: %s; running synchronously\n", e.what());
            RunSendAndCallback(callback);
        }
    }

    /**
     * Get HTTP response code. This function returns CURL error code if HTTP response code is invalid.
     */
    long GetResponseCode()
    {
        return res;
    }

    /**
     * Get whether or not response was programmatically aborted
     */
    bool WasAborted()
    {
        return isAborted.load();
    }

    /**
     * Return a copy of response headers
     *
     * @return
     */
    std::map<std::string, std::string> GetResponseHeaders()
    {
        std::map<std::string, std::string> result;
        if (respHeaders.empty())
        {
            return result;
        }

        std::stringstream ss;
        std::string headers(reinterpret_cast<const char*>(respHeaders.data()), respHeaders.size());
        ss.str(headers);

        std::string header;
        while (std::getline(ss, header, '\n')) {
            std::smatch match;
            std::regex http_headers_regex(HTTP_HEADER_REGEXP);
            if (std::regex_search(header, match, http_headers_regex))
                result[match[1]] = match[2];    // Key: value
        }
        return result;
    }

    /**
     * Return a copy of response body
     *
     * @return
     */
    std::vector<uint8_t> GetResponseBody()
    {
        return respBody;
    }

    /**
     * Return a raw copy of response headers+body
     *
     * @return
     */
    std::vector<uint8_t> GetRawResponse()
    {
        std::vector<uint8_t> result;
        if ((response.memory != nullptr) && (response.size != 0))
        {
            const auto* begin = reinterpret_cast<const uint8_t*>(response.memory);
            result.insert(result.end(), begin, begin + response.size);
        }
        return result;
    }

    /**
     * Release memory allocated for response
     */
    void ReleaseResponse()
    {
        if (response.memory != nullptr) {
            free(response.memory);
            response.memory = nullptr;
            response.size = 0;
        }
        respHeaders.clear();
        respBody.clear();
    }

    /**
     * Abort request in connecting or reading state.
     */
    void Abort()
    {
        isAborted = true;
        if (curl!=nullptr)
        {
            // Simply close the socket - connection reset by peer.. Ha-ha-ha-ha-ha!
            if (sockfd) {
                ::close(sockfd);
                sockfd = 0;
            }
        }
    }

    CURL *GetHandle()
    {
        return curl;
    }

protected:
    const bool   rawResponse;       // Do not split response headers from response body
    const size_t httpConnTimeout;   // Timeout for connect.  Default: 5s

    CURL *curl;                     // Local curl instance
    CURLcode res = CURLE_OK;        // Curl result OR HTTP status code if successful

    IHttpResponseCallback* m_callback = nullptr;

    // In-flight tracker shared with HttpClient_Curl; decremented in the destructor.
    std::shared_ptr<CurlOperationTracker> m_tracker;

    // Request values
    std::string m_method;
    std::string m_url;
    std::string m_sslCaInfo;
    // Owned copy of the request body, read by Send(). Owned (not a reference into
    // the caller's IHttpRequest) because the detached worker in SendAsync can
    // outlive that request, so a reference could dangle mid-send.
    std::vector<uint8_t> requestBody;
    struct curl_slist *m_headersChunk = nullptr;

    // Processed response headers and body
    std::vector<uint8_t>        respHeaders;
    std::vector<uint8_t>        respBody;

    // Socket parameters
    curl_socket_t sockfd = 0;

    long sockextr   = 0;

    curl_off_t nread = 0;
    size_t sendlen   = 0;        // # bytes sent by client
    size_t acklen    = 0;        // # bytes ack by server

    /**
     * Helper routine to wait for data on socket
     *
     * @param sockfd
     * @param for_recv
     * @param timeout_ms
     * @return
     */
    static int WaitOnSocket(curl_socket_t sockfd, int for_recv, long timeout_ms)
    {
        struct pollfd pfd;
        pfd.fd = sockfd;
        pfd.events = for_recv ? POLLIN : POLLOUT;
        // Cap timeout to max int value to avoid overflow in poll()
        auto timeout = std::min(timeout_ms, static_cast<long>(std::numeric_limits<int>::max()));   
        return poll(&pfd, 1, static_cast<int>(timeout));
    }

    // Raw response buffer
    struct MemoryStruct {
      char *memory;
      size_t size;
    } response;

    /**
     * Old-school memory allocator
     *
     * @param contents
     * @param size
     * @param nmemb
     * @param userp
     * @return
     */
    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *)userp;

        auto* memory = static_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));
        if(memory == nullptr) {
          /* out of memory! */
          TRACE("not enough memory (realloc returned NULL)\n");
          return 0;
        }
        mem->memory = memory;
#ifdef HAVE_ONEDS_BOUNDCHECK_METHODS
        BoundCheckFunctions::oneds_memcpy_s(&(mem->memory[mem->size]), realsize, contents, realsize);
#else
        memcpy(&(mem->memory[mem->size]), contents, realsize);
#endif
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
    }

    /**
     * C++ STL std::string allocator
     *
     * @param ptr
     * @param size
     * @param nmemb
     * @param data
     * @return
     */
    static size_t WriteVectorCallback(void *ptr, size_t size, size_t nmemb, std::vector<uint8_t>* data)
    {
        if (data != nullptr) {
            const auto* begin = static_cast<const uint8_t*>(ptr);
            const auto* end   = begin + size * nmemb;
            data->insert( data->end(), begin, end);
        }
        return size * nmemb;
    }

};

} MAT_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTCURL_HPP
