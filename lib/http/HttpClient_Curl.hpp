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

#include <algorithm>
#include <numeric>
#include <future>
#include <atomic>

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

private:
    void EraseRequest(std::string const& id);
    void AddRequest(IHttpRequest* request);

    std::mutex m_requestsMtx;
    std::map<std::string, IHttpRequest*> m_requests;
};

class CurlHttpOperation {
public:

    void DispatchEvent(HttpStateEvent type)
    {
        if(m_callback != nullptr)
            m_callback->OnHttpStateEvent(type, static_cast<void*>(curl), 0);
    }

    std::atomic<bool> isAborted { false };      // Set to 'true' when async callback is aborted

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
            // Default empty headers and empty request body
            const std::map<std::string, std::string>& requestHeaders = std::map<std::string, std::string>(),
            const std::vector<uint8_t>& requestBody                  = std::vector<uint8_t>(),
            // Default connectivity and response size options
            bool rawResponse                                         = false,
            size_t httpConnTimeout                                   = HTTP_CONN_TIMEOUT) :

            // Optional connection params
            rawResponse(rawResponse),
            httpConnTimeout(httpConnTimeout),

            m_callback(callback),
            m_method(method),
            m_url(url),

            // Local vars
            requestHeaders(requestHeaders),
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

#if 0
        // Be verbose
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#else
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
#endif

        // Specify target URL
        curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());

        // TODO: expose SSL cert verification opts via ILogConfiguration
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);      // 1L
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);      // 2L
        // HTTP/2 please, fallback to HTTP/1.1 if not supported
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

        // Specify our custom headers
        for(auto &kv : this->requestHeaders)
        {
            std::string header = kv.first.c_str();
            header += ": ";
            header += kv.second.c_str();
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
        // Given the request has not been aborted we should wait for completion here
        // This guarantees the lifetime of this request.
        if (result.valid())
        {
            result.wait();
        }
        DispatchEvent(OnDestroy);
        res = CURLE_OK;
        curl_easy_cleanup(curl);
        curl_slist_free_all(m_headersChunk);
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
        const void *request  = (requestBody.empty())?NULL:&requestBody[0];
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
        res = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &sockextr);
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
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char *)request);
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

    std::future<long> & SendAsync(std::function<void(CurlHttpOperation &)> callback = nullptr) {
        result = std::async(std::launch::async, [this, callback] {
            long result = Send();
            if (callback!=nullptr)
                callback(*this);
            return result;
        });
        return result;
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
     * Return a copy of resposne headers
     *
     * @return
     */
    std::map<std::string, std::string> GetResponseHeaders()
    {
        std::map<std::string, std::string> result;
        if (respHeaders.size() == 0)
            return result;

        std::stringstream ss;
        std::string headers((const char *)&respHeaders[0], respHeaders.size());
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
        if ((response.memory!=nullptr)&&(response.size!=0))
            result.insert(result.end(), (const char *)response.memory, ((const char *)response.memory) + response.size);
        return result;
    }

    /**
     * Release memory allocated for response
     */
    void ReleaseResponse()
    {
        if (response.memory!=nullptr) {
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
    const std::map<std::string, std::string>& requestHeaders;
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

    std::future<long>       result;

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
        struct timeval tv;
        fd_set infd, outfd, errfd;
        int res;

        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        FD_ZERO(&infd);
        FD_ZERO(&outfd);
        FD_ZERO(&errfd);

        FD_SET(sockfd, &errfd); /* always check for error */

        if(for_recv) {
            FD_SET(sockfd, &infd);
        } else {
            FD_SET(sockfd, &outfd);
        }

        /* select() returns the number of signalled sockets or -1 */
        res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
        return res;
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

        mem->memory = (char *)(realloc(mem->memory, mem->size + realsize + 1));
        if(mem->memory == NULL) {
          /* out of memory! */
          TRACE("not enough memory (realloc returned NULL)\n");
          return 0;
        }
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
        if (data!=nullptr) {
            const unsigned char * begin = (unsigned char *)(ptr);
            const unsigned char * end   = begin + size * nmemb;
            data->insert( data->end(), begin, end);
        }
        return size * nmemb;
    }

};

} MAT_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTCURL_HPP

