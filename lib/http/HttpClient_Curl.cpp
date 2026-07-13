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
        auto state = m_state;
        auto activeOps = state->activeOps;

        // Detached worker threads run curl_easy_cleanup in ~CurlHttpOperation after
        // the request callback has already been removed from HttpClientManager's
        // tracking, so waiting only on that tracking is not enough. Wait (bounded)
        // for all in-flight operations to finish their easy-handle cleanup before
        // curl_global_cleanup, which must not run concurrently with it.
        bool drained;
        {
            std::unique_lock<std::mutex> lock(activeOps->mtx);
            drained = activeOps->cv.wait_for(lock, std::chrono::seconds(5),
                    [activeOps] { return activeOps->inFlight == 0; });
            if (!drained)
            {
                TRACE("~HttpClient_Curl: %d operation(s) still in flight after 5s; skipping curl_global_cleanup\n", activeOps->inFlight);
            }
        }
        if (!drained)
        {
            activeOps->abandonCallbacks.store(true, std::memory_order_release);
            std::lock_guard<std::mutex> lock(state->requestsMtx);
            // Detached workers capture this shared state, not HttpClient_Curl. If the
            // bounded drain times out, the client object is about to be destroyed; do
            // not retain raw request pointers or dispatch late response/logging
            // callbacks that may refer to shutdown-owned state. The worker will erase
            // no-op and drop the response instead of dereferencing the destroyed client.
            state->requests.clear();
        }
        // curl_global_cleanup must not run concurrently with any other libcurl use,
        // including the curl_easy_cleanup that in-flight CurlHttpOperation destructors
        // run on their detached workers. If the drain timed out, skip it: leaking
        // libcurl's global state once at shutdown is safer than the crash/UB of tearing
        // it down while an easy handle is still live on another thread.
        if (drained)
        {
            curl_global_cleanup();
        }
        TRACE("Destroyed HttpClient_Curl.\n");
    };

    IHttpRequest* HttpClient_Curl::CreateRequest()
    {
        return new CurlHttpRequest();
    }

    void HttpClient_Curl::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
    {
        auto state = m_state;

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
            std::lock_guard<std::mutex> lock(state->requestsMtx);
            sslCaInfo = state->sslCaInfo;
        }

        // Copy the request body into the operation instead of moving it out. The
        // detached send needs an owned buffer (the request can be released -- e.g. by
        // cancellation -- while the worker is still sending), but the request's
        // m_body is also read again after the send: HttpResponseDecoder emits the
        // request payload on EVT_HTTP_OK / EVT_HTTP_ERROR when requestDone runs the
        // decode chain, before the request is released. Moving it out would leave
        // those debug events with an empty payload -- a curl-only regression versus
        // the WinInet and NSURLSession clients, which leave the request intact.
        auto curlOperation = std::make_shared<CurlHttpOperation>(curlRequest->m_method, curlRequest->m_url, callback, requestHeaders, curlRequest->m_body, false, HTTP_CONN_TIMEOUT, m_sslVerify, sslCaInfo);
        // Count this operation before the async send starts so ~HttpClient_Curl waits
        // for its curl_easy_cleanup to complete before curl_global_cleanup.
        curlOperation->trackWith(state->activeOps);
        curlRequest->SetOperation(curlOperation);

        // The async Send() runs on a detached worker that holds its own shared_ptr
        // to curlOperation (see CurlHttpOperation::SendAsync), so the operation --
        // and its curl handle, response buffer and owned copy of the request body --
        // stay alive until Send() and the callback below have finished, regardless
        // of when the owning CurlHttpRequest is released. If the callback leads to
        // that request being destroyed on the worker thread (OnHttpResponse ->
        // EventsUploadContext::clear()), the operation is simply destroyed there
        // once the worker returns; there is no future to join.
        curlOperation->SendAsync([state, callback, requestId](CurlHttpOperation& operation) {
            const bool abandonCallback = state->activeOps->abandonCallbacks.load(std::memory_order_acquire);
            {
                std::lock_guard<std::mutex> lock(state->requestsMtx);
                state->requests.erase(requestId);
            }
            if (abandonCallback)
            {
                TRACE("HttpClient_Curl shutdown abandoned response callback for %s\n", requestId.c_str());
                return;
            }

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
        auto state = m_state;
        CurlHttpRequest* request = nullptr;
        {
            // Hold the lock only while iterating over the list of requests
            std::lock_guard<std::mutex> lock(state->requestsMtx);
            auto requestIt = state->requests.find(id);
            if (requestIt != state->requests.cend()) {
                request = static_cast<CurlHttpRequest*>(requestIt->second);
                LOG_TRACE("HTTP request=%p id=%s being aborted...", request, id.c_str());
                state->requests.erase(requestIt);
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
        auto state = m_state;
        std::lock_guard<std::mutex> lock(state->requestsMtx);
        state->sslCaInfo = caInfo;
    }

    void HttpClient_Curl::AddRequest(IHttpRequest* request)
    {
        auto state = m_state;
        std::lock_guard<std::mutex> lock(state->requestsMtx);
        state->requests[request->GetId()] = request;
    }

} MAT_NS_END

#endif
