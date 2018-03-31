// Copyright (c) Microsoft. All rights reserved.

#include "HttpClient_HttpStack.hpp"
#include "utils/Common.hpp"
#include <httpstack/httpstack.hpp>
#include <httpstack/init.hpp>
#include <httpstack/uri_ext.hpp>
#include <memory>

namespace ARIASDK_NS_BEGIN {


// Based on ResizableBuffer from HTTP Stack sources
class VectorHttpStackBuffer : public virtual http_stack::IBuffer,
                              private auf::Object
{
  public:
    VectorHttpStackBuffer(std::vector<uint8_t>&& data)
      : m_data(data)
    {
    }

    virtual ~VectorHttpStackBuffer() override
    {
    }

    virtual void Shrink(size_t size) override
    {
        if (size < m_data.size()) {
            m_data.resize(size);
        }
    }

    virtual size_t Size() const override
    {
        return m_data.size();
    }

    virtual void* Get() override
    {
        return m_data.data();
    }

    virtual const void* Get() const override
    {
        return m_data.data();
    }

  protected:
    std::vector<uint8_t> m_data;
};

//---

class HttpStackRequestWrapper : public auf::Object,
                                public http_stack::IRequestCallback
{
  protected:
    HttpClient_HttpStack&   m_parent;
    std::string             m_id;
    http_stack::IRequestPtr m_httpStackRequest;
    IHttpResponseCallback*  m_appCallback;

  public:
    HttpStackRequestWrapper(HttpClient_HttpStack& parent, std::string const& id)
      : m_parent(parent),
        m_id(id)
    {
    }

    HttpStackRequestWrapper(HttpStackRequestWrapper const&) = delete;
    HttpStackRequestWrapper& operator=(HttpStackRequestWrapper const&) = delete;

    void send(SimpleHttpRequest* req, IHttpResponseCallback* callback)
    {
        LOG_TRACE("Sending HttpRequest(%s) to '%s'...", m_id.c_str(), req->m_url.c_str());
        http_stack::HTTPSTACK_ERROR result = send2(req, callback);
        if (result != http_stack::HTTPSTACK_ERROR_OK) {
            LOG_WARN("Unable to send HttpRequest(%s): %s",
                m_id.c_str(), http_stack::ErrorText(result));
            finish(nullptr, result);
        }
    }

    http_stack::HTTPSTACK_ERROR send2(SimpleHttpRequest* req, IHttpResponseCallback* callback)
    {
        retainRef();

        if (!m_parent.m_httpStack || !m_parent.m_httpStackPool) {
            return http_stack::HTTPSTACK_ERROR_OUTOFRESOURCES;
        }

        http_stack::HTTPSTACK_ERROR result = m_parent.m_httpStack->CreatePooledRequest(
            http_stack::IRequestCallbackPtr(this), m_parent.m_httpStackPool, m_httpStackRequest);
        if (result != http_stack::HTTPSTACK_ERROR_OK) {
            return result;
        }

        m_appCallback = callback;

        http_stack::connection_config_t config;
        switch (req->m_priority) {
            // *INDENT-OFF* More readable as single lines with enough horizontal whitespace
            case EventPriority_Low:         config.label = "ClientTelemetry_Priority_Low";         break;
            case EventPriority_Normal:      config.label = "ClientTelemetry_Priority_Normal";      break;
            case EventPriority_High:        config.label = "ClientTelemetry_Priority_High";        break;
            case EventPriority_Immediate:   config.label = "ClientTelemetry_Priority_Immediate";   break;
            default:                        config.label = "ClientTelemetry_Priority_Unspecified"; break;
            // *INDENT-ON*
        }

        result = m_httpStackRequest->Open(req->m_method, http_stack::make_uri(req->m_url), config);
        if (result != http_stack::HTTPSTACK_ERROR_OK) {
            return result;
        }

        for (auto const& header : req->m_headers) {
            m_httpStackRequest->SetHeader(header.first, header.second);
        }

        http_stack::IBufferPtr bodyBuf(new VectorHttpStackBuffer(std::move(req->m_body)), false);

        {
            PAL::ScopedMutexLock guard(m_parent.m_httpStackRequestsMutex);
            m_parent.m_httpStackRequests[m_id] = m_httpStackRequest;
        }

        return m_httpStackRequest->Send(bodyBuf);
    }

    void finish(http_stack::IResponse* response, http_stack::HTTPSTACK_ERROR failureReason)
    {
        std::unique_ptr<SimpleHttpResponse> resp(new SimpleHttpResponse(m_id));

        switch (failureReason) {
            case http_stack::HTTPSTACK_ERROR_OK:
                resp->m_result = HttpResult_OK;
                resp->m_statusCode = response->GetStatusCode();
                break;

            case http_stack::HTTPSTACK_ERROR_ABORTED:
                resp->m_result = HttpResult_Aborted;
                break;

            case http_stack::HTTPSTACK_ERROR_CANNOT_CONNECT:
            case http_stack::HTTPSTACK_ERROR_DATA_NOT_AVAILABLE:
            case http_stack::HTTPSTACK_ERROR_DOWNLOAD_FAILURE:
            case http_stack::HTTPSTACK_ERROR_CONNECTION_TIMEOUT:
            case http_stack::HTTPSTACK_ERROR_INVALID_CERTIFICATE:
            case http_stack::HTTPSTACK_ERROR_REDIRECT:
            case http_stack::HTTPSTACK_ERROR_AUTHORIZATION:
            case http_stack::HTTPSTACK_ERROR_NOT_FOUND:
            case http_stack::HTTPSTACK_ERROR_RETRY_EXCEEDED:
            case http_stack::HTTPSTACK_ERROR_CONNECTION_RESET:
                resp->m_result = HttpResult_NetworkFailure;
                break;

            default:
                resp->m_result = HttpResult_LocalFailure;
                break;
        }

        if (response) {
            http_stack::IResponse::HeaderList headerList;
            response->GetHeaders(headerList);
            for (auto const& header : headerList) {
                resp->m_headers.add(toLower(header.first), header.second);
            }

            http_stack::IBufferPtr bodyBuf;
            if (response->GetBody(bodyBuf) == http_stack::HTTPSTACK_ERROR_OK) {
                resp->m_body.assign(static_cast<uint8_t const*>(bodyBuf->Get()), static_cast<uint8_t const*>(bodyBuf->Get()) + bodyBuf->Size());
            }
        }

        m_appCallback->OnHttpResponse(resp.release());

        {
            PAL::ScopedMutexLock guard(m_parent.m_httpStackRequestsMutex);
            if (response) {
                // Check the assertion only if the request was dispatched.
                // If response == nullptr (called from send() method),
                // the request might have not been added to the map yet.
                assert(m_parent.m_httpStackRequests.find(m_id) != m_parent.m_httpStackRequests.end());
            }
            m_parent.m_httpStackRequests.erase(m_id);
        }

        releaseRef();
    }

    virtual void OnResponseReceived(http_stack::IResponse& response) override
    {
        LOG_TRACE("OnResponseReceived() for HttpRequest(%s)", m_id.c_str());
        finish(&response, http_stack::HTTPSTACK_ERROR_OK);
    }

    virtual void OnFailure(http_stack::IResponse& response, http_stack::HTTPSTACK_ERROR failureReason) override
    {
        LOG_TRACE("OnFailure() for HttpRequest(%s)", m_id.c_str());
        finish(&response, failureReason);
    }

    virtual http_stack::HTTPSTACK_ERROR CheckRetryRequired(http_stack::IResponse& response, http_stack::HTTPSTACK_ERROR failureReason) override
    {
        return failureReason;
    }

    virtual void OnDataReceived(http_stack::IResponse& response) override
    {
    }
};

//---

unsigned HttpClient_HttpStack::s_nextRequestId = 0;

HttpClient_HttpStack::HttpClient_HttpStack(http_stack::IHttpStack* httpStack)
  : m_httpStackRequestsMutex("AriaSDK/HttpStackRequests")
{
    LOG_TRACE("Initializing HttpStack...");

    if (httpStack != nullptr) {
        m_httpStack.reset(httpStack);
    } else {
        http_stack::CreateHttpStack(m_httpStack);
    }

    // Using no retries on HTTP Stack level, retransmission is handled
    // outside of HttpClient_HttpStack. The request pool is used only
    // to maintain persistent keep-alive connections.

    http_stack::requestpool_config_t poolConfig;
    poolConfig.type = http_stack::requestpool_config_t::TYPE_FIFO_ORDER;
    poolConfig.retries = 0;

    m_httpStack->CreateRequestPool(poolConfig, m_httpStackPool);
}

HttpClient_HttpStack::~HttpClient_HttpStack()
{
    LOG_TRACE("Shutting down HttpStack...");

    assert(m_httpStackRequests.empty());
    m_httpStackRequests.clear();
    m_httpStackPool.reset();
    m_httpStack.reset();
}

IHttpRequest* HttpClient_HttpStack::CreateRequest()
{
    std::string id = "HS-" + toString(spl::atomicAddU(&s_nextRequestId, 1));
    return new SimpleHttpRequest(id);
}

void HttpClient_HttpStack::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    SimpleHttpRequest* req = static_cast<SimpleHttpRequest*>(request);
    auf::IntrusivePtr<HttpStackRequestWrapper> wrapper(new HttpStackRequestWrapper(*this, req->m_id), false);
    wrapper->send(req, callback);
    delete req;
}

void HttpClient_HttpStack::CancelRequestAsync(std::string const& id)
{
    PAL::ScopedMutexLock guard(m_httpStackRequestsMutex);
    auto it = m_httpStackRequests.find(id);
    if (it != m_httpStackRequests.end()) {
        LOG_TRACE("Aborting HttpRequest(%s)...", id.c_str());
        it->second->Abort();
        // The request gets removed from the map in the callback.
    }
}


} ARIASDK_NS_END
