//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "HttpClient_CAPI.hpp"

#include <mutex>
#include <sstream>

#include "utils/StringUtils.hpp"

namespace MAT_NS_BEGIN {

    class HttpClient_CAPI_State
    {
    public:
        explicit HttpClient_CAPI_State(uint64_t id)
            : ownerId(id)
        {
        }

        uint64_t const ownerId;
        std::mutex requestsMutex;
    };

    // Represents a single in-flight, cancellable HTTP operation
    class HttpClient_Operation
    {
    public:
        HttpClient_Operation(
            uint64_t ownerId,
            std::string requestId,
            IHttpResponseCallback* callback,
            http_cancel_fn_t cancelFn)
          : m_requestId(std::move(requestId)),
            m_ownerId(ownerId),
            m_callback(callback),
            m_cancelFn(cancelFn)
        {
            if (m_requestId.empty() || (callback == nullptr) || (cancelFn == nullptr))
            {
                MATSDK_THROW(std::invalid_argument("Created HttpClient_Operation with invalid parameters"));
            }
        }

        void Cancel()
        {
            m_cancelFn(m_requestId.c_str());
        }

        void CompleteAborted()
        {
            auto response = std::unique_ptr<SimpleHttpResponse>(
                new SimpleHttpResponse(m_requestId));
            response->m_result = HttpResult_Aborted;
            OnResponse(response.release());
        }

        void BeginSendHandoff()
        {
            std::lock_guard<std::mutex> lock(m_completionMutex);
            m_sendInProgress = true;
        }

        void FinishSendHandoff()
        {
            std::unique_ptr<IHttpResponse> deferredResponse;
            {
                std::lock_guard<std::mutex> lock(m_completionMutex);
                m_sendInProgress = false;
                deferredResponse = std::move(m_deferredResponse);
            }
            if (deferredResponse != nullptr)
            {
                m_callback->OnHttpResponse(deferredResponse.release());
            }
        }

        void OnResponse(IHttpResponse* response)
        {
            std::unique_ptr<IHttpResponse> ownedResponse(response);
            {
                std::lock_guard<std::mutex> lock(m_completionMutex);
                if (m_sendInProgress)
                {
                    m_deferredResponse = std::move(ownedResponse);
                    return;
                }
            }
            m_callback->OnHttpResponse(ownedResponse.release());
        }

        uint64_t OwnerId() const noexcept
        {
            return m_ownerId;
        }

    private:
        std::string                         m_requestId;
        uint64_t const                      m_ownerId;
        IHttpResponseCallback*              m_callback;
        http_cancel_fn_t                    m_cancelFn;
        std::mutex                          m_completionMutex;
        bool                                m_sendInProgress {false};
        std::unique_ptr<IHttpResponse>       m_deferredResponse;
    };


    // Manage tracking of in-flight operations
    static std::mutex s_operationsLock;
    static std::atomic<uint64_t> s_nextOwnerId{0};

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
    std::shared_ptr<HttpClient_Operation> RemovePendingOperation(
        const std::string& requestId,
        uint64_t ownerId = 0)
    {
        LOCKGUARD(s_operationsLock);
        std::shared_ptr<HttpClient_Operation> operation;
        auto itOperation = GetPendingOperations().find(requestId);
        if (itOperation != GetPendingOperations().end() &&
            (ownerId == 0 || itOperation->second->OwnerId() == ownerId))
        {
            operation = itOperation->second;
            GetPendingOperations().erase(itOperation);
        }

        return operation;
    }

    std::vector<std::shared_ptr<HttpClient_Operation>>
    RemovePendingOperations(uint64_t ownerId)
    {
        std::vector<std::shared_ptr<HttpClient_Operation>> operations;
        LOCKGUARD(s_operationsLock);
        for (auto it = GetPendingOperations().begin();
             it != GetPendingOperations().end();)
        {
            if (it->second->OwnerId() == ownerId)
            {
                operations.push_back(it->second);
                it = GetPendingOperations().erase(it);
            }
            else
            {
                ++it;
            }
        }
        return operations;
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
        m_cancelFn(cancelFn),
        m_state(std::make_shared<HttpClient_CAPI_State>(++s_nextOwnerId))
    {
        if ((sendFn == nullptr) || (cancelFn == nullptr))
        {
            MATSDK_THROW(std::invalid_argument("Created HttpClient_CAPI with invalid parameters"));
        }
    }

    HttpClient_CAPI::~HttpClient_CAPI() noexcept
    {
#if HAVE_EXCEPTIONS
        try
        {
            CancelAllRequests();
        }
        catch (const std::exception& ex)
        {
            LOG_ERROR("CAPI HTTP client teardown failed: %s", ex.what());
        }
        catch (...)
        {
            LOG_ERROR("CAPI HTTP client teardown failed with a non-standard exception");
        }
#else
        CancelAllRequests();
#endif
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
        auto state = m_state;
        auto sendFn = m_sendFn;
        auto cancelFn = m_cancelFn;
        // The external hook borrows pointers into the caller's request until it
        // returns. Serialize this short handoff with cancellation so cancellation
        // cannot terminally complete the request while the hook still copies them.
        // Shared state pins the lock and owner identity if a synchronous callback
        // destroys the HttpClient_CAPI facade before this method returns.
        std::unique_lock<std::mutex> requestLock(state->requestsMutex);

        // SendRequestAsync borrows the request; the caller retains ownership.
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

        auto operation = std::make_shared<HttpClient_Operation>(
            state->ownerId, requestId, callback, cancelFn);
        AddPendingOperation(requestId, operation);

        operation->BeginSendHandoff();
#if HAVE_EXCEPTIONS
        std::exception_ptr sendException;
        try
        {
            sendFn(&capiRequest, &OnHttpResponse);
        }
        catch (...)
        {
            sendException = std::current_exception();
        }
        requestLock.unlock();
        operation->FinishSendHandoff();

        if (sendException != nullptr)
        {
            // A throwing send rejected the request. Retire the operation so a
            // misbehaving hook cannot later call into a callback the manager has
            // already completed synthetically.
            auto rejectedOperation = RemovePendingOperation(
                requestId, state->ownerId);
            if (rejectedOperation == nullptr)
            {
                // The hook completed (or cancellation completed) the request
                // synchronously before throwing. The terminal callback is the
                // authoritative outcome; do not expose both completion and an
                // exception to a direct CAPI client.
                LOG_ERROR("CAPI HTTP send hook threw after completing request %s",
                    requestId.c_str());
                return;
            }
            std::rethrow_exception(sendException);
        }
#else
        sendFn(&capiRequest, &OnHttpResponse);
        requestLock.unlock();
        operation->FinishSendHandoff();
#endif
    }

    void HttpClient_CAPI::CancelRequestAsync(const std::string& id)
    {
        auto state = m_state;
        LOG_TRACE("Cancelling CAPI HTTP request '%s'", id.c_str());
        std::shared_ptr<HttpClient_Operation> operation;
        {
            // Wait for the external send hook to release request-backed
            // pointers, then retire the operation before dropping the lock.
            std::lock_guard<std::mutex> requestLock(
                state->requestsMutex);
            operation = RemovePendingOperation(id, state->ownerId);
        }
        
        if (operation != nullptr)
        {
#if HAVE_EXCEPTIONS
            try
            {
                operation->Cancel();
            }
            catch (const std::exception& ex)
            {
                LOG_ERROR("CAPI HTTP cancellation failed for request %s: %s",
                    id.c_str(), ex.what());
            }
            catch (...)
            {
                LOG_ERROR("CAPI HTTP cancellation failed for request %s",
                    id.c_str());
            }
#else
            operation->Cancel();
#endif
            // Cancellation is terminal from the adapter's perspective. The
            // operation was removed first, so synchronous or late external
            // completions are ignored and cannot double-complete the callback.
#if HAVE_EXCEPTIONS
            try
            {
                operation->CompleteAborted();
            }
            catch (const std::exception& ex)
            {
                LOG_ERROR("CAPI HTTP cancellation callback failed for request %s: %s",
                    id.c_str(), ex.what());
            }
            catch (...)
            {
                LOG_ERROR("CAPI HTTP cancellation callback failed for request %s",
                    id.c_str());
            }
#else
            operation->CompleteAborted();
#endif
        }
    }

    void HttpClient_CAPI::CancelAllRequests()
    {
        auto state = m_state;
        LOG_TRACE("Cancelling all CAPI HTTP requests");
        // Retire this client's full snapshot before invoking external
        // cancellation. Other CAPI clients keep their independent operations.
        std::vector<std::shared_ptr<HttpClient_Operation>> operations;
        {
            // Wait until any external send hook has released request-backed
            // pointers. Do not hold this member lock across terminal callbacks:
            // a direct callback is allowed to destroy the client.
            std::lock_guard<std::mutex> requestLock(
                state->requestsMutex);
            operations = RemovePendingOperations(state->ownerId);
        }

        for (const auto& operation : operations)
        {
#if HAVE_EXCEPTIONS
            try
            {
                operation->Cancel();
            }
            catch (const std::exception& ex)
            {
                LOG_ERROR("CAPI HTTP cancellation failed: %s", ex.what());
            }
            catch (...)
            {
                LOG_ERROR("CAPI HTTP cancellation failed with a non-standard exception");
            }
#else
            operation->Cancel();
#endif
#if HAVE_EXCEPTIONS
            try
            {
                operation->CompleteAborted();
            }
            catch (const std::exception& ex)
            {
                LOG_ERROR("CAPI HTTP cancellation callback failed: %s", ex.what());
            }
            catch (...)
            {
                LOG_ERROR("CAPI HTTP cancellation callback failed with a non-standard exception");
            }
#else
            operation->CompleteAborted();
#endif
        }
    }

} MAT_NS_END
