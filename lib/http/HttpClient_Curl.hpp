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
#include <mutex>
#include <stdexcept>

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
    void EraseRequest(std::string const& id);
    void AddRequest(IHttpRequest* request);

    std::mutex m_requestsMtx;
    std::map<std::string, IHttpRequest*> m_requests;
    std::atomic<bool> m_sslVerify { true };
    std::string m_sslCaInfo;
};

class CurlHttpOperation {
public:

    void DispatchEvent(HttpStateEvent type)
    {
        if (m_callback != nullptr)
        {
            m_callback->OnHttpStateEvent(type, static_cast<void*>(curl), 0);
        }
    }

    std::atomic<bool> isAborted { false };      // Set to 'true' when async callback is aborted
    bool m_optionFailure { false };

    /**
     * Create local CURL instance for url and body
     *
     * @param url
     * @param body
     * @param httpConnTimeout   HTTP connection timeout in seconds
     * @param httpReadTimeout   HTTP read timeout in seconds
     */
    // Selects HTTP/2 only when the libcurl we are actually linked against was
    // built with HTTP/2 support. Setting CURLOPT_HTTP_VERSION to
    // CURL_HTTP_VERSION_2_0 against a libcurl without HTTP/2 does not silently
    // downgrade -- it fails the transfer with CURLE_UNSUPPORTED_PROTOCOL -- so
    // the version has to be probed at runtime rather than assumed.
    static long GetPreferredHttpVersion() noexcept
    {
        const curl_version_info_data* versionInfo = curl_version_info(CURLVERSION_NOW);
        if (versionInfo != nullptr && (versionInfo->features & CURL_VERSION_HTTP2) != 0)
        {
            return CURL_HTTP_VERSION_2_0;
        }
        return CURL_HTTP_VERSION_1_1;
    }

    CurlHttpOperation(
            std::string method,
            std::string url,
            IHttpResponseCallback* callback,
            // requestHeaders is copied into the curl_slist during construction and
            // need not outlive this operation. requestBody is stored by reference;
            // CurlHttpRequest destroys this operation (which joins the worker) before
            // destroying its inherited request-body storage.
            const std::map<std::string, std::string>& requestHeaders,
            const std::vector<uint8_t>& requestBody,
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
            requestBody(requestBody)
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

        if (!SetOption(CURLOPT_VERBOSE, 0L) ||
            !SetOption(CURLOPT_URL, m_url.c_str()) ||
            !SetOption(CURLOPT_SSL_VERIFYPEER, sslVerify ? 1L : 0L) ||
            !SetOption(CURLOPT_SSL_VERIFYHOST, sslVerify ? 2L : 0L) ||
            (!m_sslCaInfo.empty() && !SetOption(CURLOPT_CAINFO, m_sslCaInfo.c_str())) ||
            // HTTP/2 when the linked libcurl supports it, otherwise HTTP/1.1
            !SetOption(CURLOPT_HTTP_VERSION, GetPreferredHttpVersion()))
        {
            DispatchEvent(OnCreateFailed);
            return;
        }

        // Headers are copied into m_headersChunk during construction and the
        // curl_slist is kept alive until destruction, so the original map does
        // not need operation-lifetime storage.
        for (const auto& kv : requestHeaders)
        {
            std::string header = kv.first + ": " + kv.second;
            curl_slist* appendedHeaders = curl_slist_append(m_headersChunk, header.c_str());
            if (appendedHeaders == nullptr)
            {
                res = CURLE_OUT_OF_MEMORY;
                m_optionFailure = true;
                DispatchEvent(OnCreateFailed);
                return;
            }
            m_headersChunk = appendedHeaders;
        }

        if (m_headersChunk != nullptr && !SetOption(CURLOPT_HTTPHEADER, m_headersChunk))
        {
            DispatchEvent(OnCreateFailed);
            return;
        }
        TRACE("method=%s, url=%s\n", this->m_method.c_str(), this->m_url.c_str());

        DispatchEvent(OnCreated);
    }

    /**
     * Destroy CURL instance
     */
    virtual ~CurlHttpOperation()
    {
        if (m_worker.joinable())
        {
            if (m_worker.get_id() == std::this_thread::get_id())
            {
                // The completion callback can release the owning request on this
                // worker. Detach rather than joining the current thread; Send() has
                // finished and the worker does not touch this operation afterward.
                m_worker.detach();
            }
            else
            {
                m_worker.join();
            }
        }

        DispatchDestroyEvent();
        res = CURLE_OK;
        if (curl != nullptr)
        {
            curl_easy_cleanup(curl);
        }
        if (m_headersChunk != nullptr)
        {
            curl_slist_free_all(m_headersChunk);
        }
        ReleaseResponse();
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
        long httpStatusCode = 0;
        CURLcode infoResult = CURLE_OK;

        if(!curl)
        {
            res = CURLE_FAILED_INIT;
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }
        if (m_optionFailure)
        {
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }

        // TODO: should we control what local source port we use?
        // curl_easy_setopt(curl, CURLOPT_LOCALPORT, dcf_port);

        // Perform initial connect, handling the timeout if needed
        if (!SetOption(CURLOPT_CONNECT_ONLY, 1L))
        {
            DispatchEvent(OnConnectFailed);
            goto cleanup;
        }
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
        if (!SetOption(CURLOPT_CONNECT_ONLY, 0L))
        {
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }

        // send all data to our callback function
        if (rawResponse)
        {
            if (!SetOption(CURLOPT_HEADER, 1L) ||
                !SetOption(CURLOPT_WRITEFUNCTION, &WriteMemoryCallback) ||
                !SetOption(CURLOPT_WRITEDATA, static_cast<void*>(&response)))
            {
                DispatchEvent(OnSendFailed);
                goto cleanup;
            }
        } else {
            if (!SetOption(CURLOPT_HEADERFUNCTION, &WriteVectorCallback) ||
                !SetOption(CURLOPT_HEADERDATA, static_cast<void*>(&respHeaders)) ||
                !SetOption(CURLOPT_WRITEFUNCTION, &WriteVectorCallback) ||
                !SetOption(CURLOPT_WRITEDATA, static_cast<void*>(&respBody)))
            {
                DispatchEvent(OnSendFailed);
                goto cleanup;
            }
        }

        // TODO: only two methods supported for now - POST and GET
        if (m_method.compare("POST") == 0)
        {
            // POST
            if (!SetOption(CURLOPT_POST, 1L) ||
                !SetOption(CURLOPT_POSTFIELDS, static_cast<const char*>(request)) ||
                !SetOption(CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(reqSize)))
            {
                DispatchEvent(OnSendFailed);
                goto cleanup;
            }
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

        if (!SetOption(CURLOPT_LOW_SPEED_TIME, 30L) ||
            !SetOption(CURLOPT_LOW_SPEED_LIMIT, 4096L))
        {
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }
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
        infoResult = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);
        if (infoResult != CURLE_OK)
        {
            res = infoResult;
            DispatchEvent(OnSendFailed);
            TRACE("Error getting HTTP response code: %s\n", curl_easy_strerror(res));
            goto cleanup;
        }
        res = static_cast<CURLcode>(httpStatusCode);
        // We got some response from server. Dump the contents.
        TRACE("HTTP response code %ld\n", httpStatusCode);
        DispatchEvent(OnResponse);

cleanup:

        // This function returns:
        // - on success: HTTP status code.
        // - on failure: CURL error code.
        // The two sets of enums (CURLE, HTTP codes) - do not intersect, so we collapse them in one set.
        return res;
    }

    void SendAsync(std::function<void(CurlHttpOperation &)> callback = nullptr) {
        // A newly created std::thread may run before it is assigned to m_worker.
        // Hold this gate until the assignment completes so a fast failure cannot
        // destroy the operation from its callback while SendAsync still uses it.
        {
            std::lock_guard<std::mutex> startGuard(m_workerStartMtx);
            if (m_sendAttempted)
            {
                throw std::logic_error("CurlHttpOperation is single-use");
            }
            m_sendAttempted = true;

            try
            {
                m_worker = std::thread([this, callback]() {
                    {
                        std::lock_guard<std::mutex> startGuard(m_workerStartMtx);
                    }
                    try
                    {
                        Send();
                    }
                    catch (...)
                    {
                        // std::async stored worker exceptions in its unobserved
                        // future. A raw thread must contain them.
                        res = CURLE_FAILED_INIT;
                    }
                    Complete(callback);
                });
                return;
            }
            catch (...)
            {
                // Callable allocation/copy or std::thread creation failed.
            }
        }

        res = CURLE_FAILED_INIT;
        Complete(callback);
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

    bool HasOptionFailure() const
    {
        return m_optionFailure;
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

    // Request values
    std::string m_method;
    std::string m_url;
    std::string m_sslCaInfo;
    // The owning CurlHttpRequest destroys this operation before its inherited
    // request-body storage, and cross-thread destruction joins the worker.
    const std::vector<uint8_t>& requestBody;
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

    std::mutex m_workerStartMtx;
    bool m_sendAttempted = false;
    std::thread m_worker;
    std::atomic<bool> m_destroyEventDispatched { false };

    void DispatchDestroyEvent() noexcept
    {
        bool expected = false;
        if (m_destroyEventDispatched.compare_exchange_strong(
                expected, true, std::memory_order_acq_rel))
        {
            try
            {
                DispatchEvent(OnDestroy);
            }
            catch (...)
            {
                // State observers must not terminate the worker or destructor.
            }
        }
    }

    void Complete(const std::function<void(CurlHttpOperation &)>& callback) noexcept
    {
        // Preserve the documented state event while m_callback is still valid.
        // The completion callback can release the last owner, so this must remain
        // the worker's final access to the operation.
        DispatchDestroyEvent();
        try
        {
            if (callback != nullptr)
            {
                callback(*this);
            }
        }
        catch (...)
        {
            // Match the old unobserved-future behavior at the thread boundary.
        }
    }

    template <typename T>
    bool SetOption(CURLoption option, T value)
    {
        if (curl == nullptr)
        {
            res = CURLE_FAILED_INIT;
            m_optionFailure = true;
            return false;
        }

        const CURLcode optionResult = curl_easy_setopt(curl, option, value);
        if (optionResult == CURLE_OK)
        {
            return true;
        }

        LOG_WARN("curl_easy_setopt(%d) failed: %s", static_cast<int>(option), curl_easy_strerror(optionResult));
        res = optionResult;
        m_optionFailure = true;
        return false;
    }

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

    // SECURITY: upper bound on the collector response the client will buffer. The
    // OneCollector protocol responses (status, kill-switch tokens, retry-after, small
    // config) are tiny, so this generous cap never rejects a legitimate response but
    // stops a hostile or MITM'd collector from driving unbounded memory growth by
    // returning an oversized body (a memory-amplification DoS of the embedding process).
    // Exceeding it aborts the transfer, so the upload is treated as failed and retried.
    static constexpr size_t kMaxResponseBytes = 16 * 1024 * 1024; // 16 MB

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
        // Guard the size * nmemb product against size_t overflow before using it.
        if (nmemb != 0 && size > static_cast<size_t>(-1) / nmemb) {
            return 0;
        }
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *)userp;

        // SECURITY: bound the buffered response (see kMaxResponseBytes). Compare
        // overflow-safely (mem->size is always <= kMaxResponseBytes here). Returning a
        // short count aborts the transfer with CURLE_WRITE_ERROR.
        if (realsize > kMaxResponseBytes - mem->size) {
            TRACE("Response exceeds max buffered size (%zu bytes); aborting transfer\n", kMaxResponseBytes);
            return 0;
        }

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
        // Guard the size * nmemb product against size_t overflow before using it.
        if (nmemb != 0 && size > static_cast<size_t>(-1) / nmemb) {
            return 0;
        }
        size_t realsize = size * nmemb;
        if (data != nullptr) {
            // SECURITY: bound the buffered response (see kMaxResponseBytes). Compare
            // overflow-safely (data->size() is always <= kMaxResponseBytes here).
            // Returning a short count aborts the transfer with CURLE_WRITE_ERROR.
            if (realsize > kMaxResponseBytes - data->size()) {
                TRACE("Response exceeds max buffered size (%zu bytes); aborting transfer\n", kMaxResponseBytes);
                return 0;
            }
            const auto* begin = static_cast<const uint8_t*>(ptr);
            const auto* end   = begin + realsize;
            data->insert( data->end(), begin, end);
        }
        return realsize;
    }

};

} MAT_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTCURL_HPP
