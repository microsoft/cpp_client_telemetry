//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

// Assume that if we are compiling with MSVC, then we prefer to use Windows HTTP stack,
// e.g. WinInet.dll or Win 10 HTTP client instead
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)

#include "ctmacros.hpp"

#include <memory>

#include "utils/Utils.hpp"
#include "HttpClient_Curl.hpp"
#include "ILogConfiguration.hpp"

namespace MAT_NS_BEGIN {

    static std::string NextReqId() {
        static std::atomic<uint64_t> seq(0);
        return std::string("REQ-") + std::to_string(seq.fetch_add(1));
    }

    class CurlHttpRequest : public SimpleHttpRequest
    {
    public:
        CurlHttpRequest() : SimpleHttpRequest(NextReqId()) { }

        void SetOperation(const std::shared_ptr<CurlHttpOperation>& curlOperation)
        {
            m_curlOperation = curlOperation;
        }

        void Cancel()
        {
            if (m_curlOperation != nullptr) {
                m_curlOperation->Abort();
            }
        }

    private:
        std::shared_ptr<CurlHttpOperation> m_curlOperation;
    };

    HttpClient_Curl::HttpClient_Curl()
    {
        /* In windows, this will init the winsock stuff */
        TRACE("Initializing HttpClient_Curl...\n");
        curl_global_init(CURL_GLOBAL_ALL);
        TRACE("libcurl version = %s\n", curl_version_info(CURLVERSION_NOW)->version);
    }

    HttpClient_Curl::~HttpClient_Curl()
    {
        curl_global_cleanup();
        TRACE("Destroyed HttpClient_Curl.\n");
    };

    IHttpRequest* HttpClient_Curl::CreateRequest()
    {
        return new CurlHttpRequest();
    }

    void HttpClient_Curl::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
    {
        // Note: 'request' is never owned by IHttpClient and gets deleted in EventsUploadContext.clear()
        AddRequest(request);
        auto curlRequest = static_cast<CurlHttpRequest*>(request);

        std::string requestId = curlRequest->GetId();
        std::map<std::string, std::string> requestHeaders;
        for (const auto& header : curlRequest->m_headers) {
            requestHeaders[header.first] = header.second;
        }

        std::string sslCaInfo;
        {
            std::lock_guard<std::mutex> lock(m_requestsMtx);
            sslCaInfo = m_sslCaInfo;
        }

        auto curlOperation = std::make_shared<CurlHttpOperation>(curlRequest->m_method, curlRequest->m_url, callback, requestHeaders, curlRequest->m_body, false, HTTP_CONN_TIMEOUT, m_sslVerify, sslCaInfo);
        curlRequest->SetOperation(curlOperation);
        
        // The lifetime of curlOperation across the async Send is guaranteed by
        // ~CurlHttpOperation. After this function returns, the only remaining
        // shared_ptr is the one held by the owning CurlHttpRequest. When that
        // request is destroyed from another thread, the destructor waits for the
        // async result; if the callback below leads to the request being
        // destroyed on the async thread itself (OnHttpResponse ->
        // EventsUploadContext::clear()), the destructor defers the join instead.
        curlOperation->SendAsync([this, callback, requestId](CurlHttpOperation& operation) {
            this->EraseRequest(requestId);

            auto response = std::unique_ptr<SimpleHttpResponse>(new SimpleHttpResponse(requestId));
            response->m_result = HttpResult_OK;

            response->m_statusCode = operation.GetResponseCode();
            if (response->m_statusCode == CURLE_FAILED_INIT) {
                // There was an error in CURL stack while trying to create request
                response->m_result = HttpResult_LocalFailure;
            } else if ((CURLE_OK < response->m_statusCode) && (response->m_statusCode <= CURL_LAST)) {
                if (operation.WasAborted()) {
                    // Operation was manually aborted
                    response->m_result = HttpResult_Aborted;
                } else {
                    // There was an error in CURL stack while trying to connect
                    response->m_result = HttpResult_NetworkFailure;
                }
            }

            auto responseHeaders = operation.GetResponseHeaders();
            response->m_headers.insert(responseHeaders.begin(), responseHeaders.end());
            response->m_body = operation.GetResponseBody();
            
            // 'response' is no longer owned by IHttpClient and gets deleted in EventsUploadContext.clear()
            callback->OnHttpResponse(response.release());
        });
    }

    void HttpClient_Curl::CancelRequestAsync(std::string const& id)
    {
        CurlHttpRequest* request = nullptr;
        {
            // Hold the lock only while iterating over the list of requests
            std::lock_guard<std::mutex> lock(m_requestsMtx);
            if (m_requests.find(id) != m_requests.cend()) {
                request = static_cast<CurlHttpRequest*>(m_requests[id]);
                LOG_TRACE("HTTP request=%p id=%s being aborted...", request, id.c_str());
                m_requests.erase(id);
            }
        }

        if (request != nullptr) {
            request->Cancel();
        }
    }

    void HttpClient_Curl::ApplySettings(ILogConfiguration& config)
    {
        SetSslVerification(
            config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY],
            (const char *)config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO]);
    }

    void HttpClient_Curl::SetSslVerification(bool sslVerify, const std::string& caInfo)
    {
        m_sslVerify = sslVerify;
        std::lock_guard<std::mutex> lock(m_requestsMtx);
        m_sslCaInfo = caInfo;
    }

    void HttpClient_Curl::EraseRequest(std::string const& id)
    {
        std::lock_guard<std::mutex> lock(m_requestsMtx);
        m_requests.erase(id);
    }

    void HttpClient_Curl::AddRequest(IHttpRequest* request)
    {
        std::lock_guard<std::mutex> lock(m_requestsMtx);
        m_requests[request->GetId()] = request;
    }

} MAT_NS_END

#endif

