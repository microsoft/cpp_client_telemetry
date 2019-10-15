// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

// Assume that if we are compiling with MSVC, then we prefer to use Windows HTTP stack,
// e.g. WinInet.dll or Win 10 HTTP client instead
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)

#include "Version.hpp"
#include "HttpClient.hpp"

#include <memory>

#include "utils/Utils.hpp"
#include "HttpClient_Curl.hpp"

namespace ARIASDK_NS_BEGIN {

    HttpClientCurl curlClient;

    static void DebugPrint(SimpleHttpRequest* request)
    {
        LOG_TRACE(">>> %s %s", request->m_method.c_str(), request->m_url.c_str());
        LOG_TRACE(">>> HEADERS:");
        for (auto &h : request->m_headers) {
            LOG_TRACE(">>> %s: %s", h.first.c_str(), h.second.c_str());
        }
    }

    static std::string NextReqId() {
        static std::atomic<uint64_t> seq(0);
        return std::string("REQ-") + std::to_string(seq.fetch_add(1));
    }

    static std::string NextRespId() {
        static std::atomic<uint64_t> seq(0);
        return std::string("RESP-") + std::to_string(seq.fetch_add(1));
    }

    /**
     * curl request wrapper compatible with telemetry SDK
     */
    class CurlHttpRequest : public SimpleHttpRequest {

    public:

        HttpClient             *parent;
        HttpRequestCurl        *request;    // real curl request
        SimpleHttpResponse      response;
        IHttpResponseCallback  *callback;

        CurlHttpRequest(HttpClient* parent) :
            SimpleHttpRequest(NextReqId()),
            response(NextRespId()),
            parent(parent)
        {
            request = nullptr;
            callback = nullptr;
            parent->add( (IHttpRequest *)this );
        }

        void clean()
        {
            if (request != nullptr)
            {
                LOG_TRACE("clean curl=%p", request);
                delete request;
                request = nullptr;
            }
        }

        virtual ~CurlHttpRequest()
        {
            LOG_TRACE("~CurlHttpRequest=%p, curl=%p", this, request);
            clean();
            // Stop tracking this request in request-response map
            parent->erase((IHttpRequest *)this);
        }

        CURLcode OnCurlStateEvent(HttpStateEvent state, HttpRequestCurl& curl)
        {
            callback->OnHttpStateEvent(state, (void*)(curl.GetHandle()), 0);
            return CURLE_OK;
        }

        /**
         * Send async HTTP request and invoke callback on completion
         */
        void SendAsync(IHttpResponseCallback* callback) {
            std::map<std::string, std::string> headers;

            for (auto &kv : m_headers)
                headers[kv.first] = kv.second;

            this->callback = callback;
            request = new HttpRequestCurl(m_method, m_url, headers, m_body,
                // Wire HttpRequestCurl callback to higher-level supplied callback
                [&](HttpStateEvent ev, HttpRequestCurl& obj)
                {
                    return this->OnCurlStateEvent(ev, obj);
                }
            );

            response.m_result = HttpResult_OK;

            if (request!=nullptr) {
                request->SendAsync([this, callback](HttpRequestCurl& req) -> void {
                    response.m_result = HttpResult_OK;

                    response.m_statusCode = request->GetResponseCode();
                    if (response.m_statusCode == CURLE_FAILED_INIT) {
                        // There was an error in CURL stack while trying to create request
                        response.m_result = HttpResult_LocalFailure;
                    } else
                    if ((CURLE_OK<response.m_statusCode)&&(response.m_statusCode<=CURL_LAST)) {
                        // There was an error in CURL stack while trying to connect
                        response.m_result = HttpResult_NetworkFailure;
                    }
                    auto responseHeaders = request->GetResponseHeaders();
                    response.m_headers.insert(responseHeaders.begin(), responseHeaders.end());
                    response.m_body = request->GetResponseBody();
                    // OnHttpResponse is sync on Linux here cause the outside wrapper is async
                    callback->OnHttpResponse(&response);
                });
            };
        }

        /**
         * Abort outstanding HTTP request
         */
        void Abort() {
            if (request!=nullptr) {
                // Signal abort
                response.m_result = HttpResult_Aborted;
                request->Abort();
                // TODO: should we invoke callback? Probably not.
                // callback->OnHttpResponse(&response);
            }
            clean();
        }
    };

    HttpClient::HttpClient()
    {
        LOG_TRACE("Initializing HttpStack...");
    }

    HttpClient::~HttpClient()
    {
        LOG_TRACE("Shutting down HttpStack...");
    }

    IHttpRequest* HttpClient::CreateRequest()
    {
        CurlHttpRequest *result = new CurlHttpRequest(this);
        LOG_TRACE("HTTP request=%p id=%s created", result, result->GetId().c_str());
        return result;
    }

    void HttpClient::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
    {
        CurlHttpRequest* req = dynamic_cast<CurlHttpRequest*>(request);
        DebugPrint(req);
        req->SendAsync(callback);
        LOG_TRACE("HTTP request=%p callback=%p sent", request, callback);
    }

    void HttpClient::CancelRequestAsync(std::string const& id)
    {
        CurlHttpRequest * req = nullptr;
        {   // Hold the lock while iterating over the list of requests
            std::lock_guard<std::mutex> lock(m_request_mtx);
            if (m_requests.find(id) != m_requests.cend())
            {
                req = dynamic_cast<CurlHttpRequest *>(m_requests[id]);
                LOG_TRACE("HTTP request=%p id=%s being aborted...", req, id.c_str());
                m_requests.erase(id);
            }
            if (req != nullptr)
            {
                req->Abort();
            }
        }
    }

} ARIASDK_NS_END

#endif
