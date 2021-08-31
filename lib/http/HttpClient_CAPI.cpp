//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "HttpClient_CAPI.hpp"

#include <mutex>
#include <sstream>

#include "utils/StringUtils.hpp"

namespace MAT_NS_BEGIN {

    // Represents a single in-flight, cancellable HTTP operation
    class HttpClient_Operation
    {
    public:
        HttpClient_Operation(SimpleHttpRequest* request, IHttpResponseCallback* callback, http_cancel_fn_t cancelFn)
          : m_request(request),
            m_callback(callback),
            m_cancelFn(cancelFn)
        {
            if ((m_request == nullptr) || (callback == nullptr) || (cancelFn == nullptr))
            {
                MATSDK_THROW(std::invalid_argument("Created HttpClient_Operation with invalid parameters"));
            }
        }

        void Cancel()
        {
            m_cancelFn(m_request->m_id.c_str());
        }

        void OnResponse(IHttpResponse* response)
        {
            m_callback->OnHttpResponse(response);
        }

    private:
        SimpleHttpRequest*                  m_request;

        IHttpResponseCallback*              m_callback;
        http_cancel_fn_t                    m_cancelFn;
    };


    // Manage tracking of in-flight operations
    static std::mutex s_operationsLock;

    std::map<std::string, std::shared_ptr<HttpClient_Operation>>& GetPendingOperations()
    {
        static std::map<std::string, std::shared_ptr<HttpClient_Operation>> s_operations;
        return s_operations;
    }

    // Track pending http requests for the sake of handling associated responses or cancellations
    void AddPendingOperation(const std::string& requestId, const std::shared_ptr<HttpClient_Operation>& operation)
    {
        LOCKGUARD(s_operationsLock);
        GetPendingOperations()[requestId] = operation;
    }

    // An operation is removed when a response has been received or the operation has been cancelled
    std::shared_ptr<HttpClient_Operation> RemovePendingOperation(const std::string& requestId)
    {
        LOCKGUARD(s_operationsLock);
        std::shared_ptr<HttpClient_Operation> operation;
        auto itOperation = GetPendingOperations().find(requestId);
        if (itOperation != GetPendingOperations().end())
        {
            operation = itOperation->second;
            GetPendingOperations().erase(itOperation);
        }

        return operation;
    }

    // Callback invoked when a response is ready. The ID of the response will match the ID of the corresponding request.
    void EVTSDK_LIBABI_CDECL OnHttpResponse(const char* requestId, http_result_t result, http_response_t* capiResponse)
    {
        auto operation = RemovePendingOperation(requestId);
        if (operation != nullptr)
        {
            std::unique_ptr<SimpleHttpResponse> response(new SimpleHttpResponse(requestId));

            switch (result)
            {
                case HTTP_RESULT_OK:
                    response->m_result = HttpResult_OK;
                    break;
                case HTTP_RESULT_CANCELLED:
                    response->m_result = HttpResult_Aborted;
                    break;
                case HTTP_RESULT_LOCAL_FAILURE:
                    response->m_result = HttpResult_LocalFailure;
                    break;
                case HTTP_RESULT_NETWORK_FAILURE:
                    response->m_result = HttpResult_NetworkFailure;
                    break;
                default:
                    response->m_result = HttpResult_LocalFailure;
                    break;
            }

            // If CAPI response exists, convert it to SimpleHttpResponse
            if (capiResponse != nullptr)
            {
                response->m_statusCode = capiResponse->statusCode;

                if (capiResponse->bodySize > 0)
                {
                    response->m_body = std::vector<uint8_t>(capiResponse->body, capiResponse->body + capiResponse->bodySize);
                }

                for (int32_t i = 0; i < capiResponse->headersCount; ++i)
                {
                    const http_header_t* capiHeader = &capiResponse->headers[i];
                    response->m_headers.emplace(capiHeader->name, capiHeader->value);
                }
            }

            // 'response' is no longer owned by IHttpClient and gets deleted in EventsUploadContext.clear()
            operation->OnResponse(response.release());
        }
    }


    HttpClient_CAPI::HttpClient_CAPI(http_send_fn_t sendFn, http_cancel_fn_t cancelFn)
      : m_sendFn(sendFn),
        m_cancelFn(cancelFn)
    {
        if ((sendFn == nullptr) || (cancelFn == nullptr))
        {
            MATSDK_THROW(std::invalid_argument("Created HttpClient_CAPI with invalid parameters"));
        }
    }

    IHttpRequest* HttpClient_CAPI::CreateRequest()
    {
        // Generate a unique request ID
        static std::atomic<int32_t> s_nextRequestId(0);
        std::ostringstream idStream;
        idStream << "OneDS_HTTP-" << s_nextRequestId++;
        std::string requestId = idStream.str();

        return new SimpleHttpRequest(requestId);
    }

    void HttpClient_CAPI::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
    {
        // Note: 'request' is never owned by IHttpClient and gets deleted in EventsUploadContext.clear()
        auto simpleRequest = static_cast<SimpleHttpRequest*>(request);
        auto requestId = simpleRequest->m_id;

        LOG_TRACE("Sending CAPI HTTP request '%s'", requestId.c_str());

        // Convert IHttpRequest to http_request_t
        // Note that the lifetime of capiRequest's members expires after this method is terminated. It is the
        // responsibility of the external functions to copy any data that must live beyond the initial call to SendHttpRequest.
        http_request_t capiRequest;

        capiRequest.id = requestId.c_str();
        capiRequest.type = equalsIgnoreCase(simpleRequest->m_method, "post") ? HTTP_REQUEST_TYPE_POST : HTTP_REQUEST_TYPE_GET;
        capiRequest.url = simpleRequest->m_url.c_str();
        capiRequest.bodySize = static_cast<int32_t>(simpleRequest->m_body.size());
        capiRequest.body = simpleRequest->m_body.data();

        // Build headers
        std::vector<http_header_t> capiHeaders;
        for (const auto& header : simpleRequest->m_headers)
        {
            http_header_t capiHeader;
            capiHeader.name = header.first.c_str();
            capiHeader.value = header.second.c_str();
            capiHeaders.push_back(capiHeader);
        }
        capiRequest.headersCount = static_cast<int32_t>(capiHeaders.size());
        capiRequest.headers = capiHeaders.data();

        auto operation = std::make_shared<HttpClient_Operation>(simpleRequest, callback, m_cancelFn);
        AddPendingOperation(requestId, operation);

        m_sendFn(&capiRequest, &OnHttpResponse);
    }

    void HttpClient_CAPI::CancelRequestAsync(const std::string& id)
    {
        LOG_TRACE("Cancelling CAPI HTTP request '%s'", id.c_str());
        std::shared_ptr<HttpClient_Operation> operation;
        {
            // Only lock mutex while actually reading/writing pending operations collection to prevent potential recursive deadlock
            LOCKGUARD(s_operationsLock);
            auto itOperation = GetPendingOperations().find(id);
            if (itOperation != GetPendingOperations().end())
            {
                operation = itOperation->second;
            }
        }
        
        if (operation != nullptr)
        {
            operation->Cancel();
        }
    }

    void HttpClient_CAPI::CancelAllRequests()
    {
        LOG_TRACE("Cancelling all CAPI HTTP requests");
        std::vector<std::shared_ptr<HttpClient_Operation>> operations;
        {
            // Only lock mutex while actually reading/writing pending operations collection to prevent potential recursive deadlock
            LOCKGUARD(s_operationsLock);
            for (const auto& operation : GetPendingOperations())
            {
                operations.push_back(operation.second);
            }
        }

        for (const auto& operation : operations)
        {
            operation->Cancel();
        }
    }

} MAT_NS_END
