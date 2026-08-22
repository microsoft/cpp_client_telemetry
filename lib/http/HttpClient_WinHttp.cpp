//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
#include "HttpClient_WinHttp.hpp"
#include "utils/StringConversion.hpp"
#include "utils/StringUtils.hpp"

#include <winhttp.h>
#include <wincrypt.h>

#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "winhttp.lib")

namespace MAT_NS_BEGIN {

namespace {

constexpr DWORD DEFAULT_MAX_CONNECTIONS_PER_SERVER = 4;

void setConnectionLimits(HINTERNET session, DWORD maxConnections) noexcept
{
    if (session == nullptr)
    {
        return;
    }

    if (!::WinHttpSetOption(session, WINHTTP_OPTION_MAX_CONNS_PER_SERVER,
            &maxConnections, sizeof(maxConnections)))
    {
        LOG_WARN("WinHttpSetOption(MAX_CONNS_PER_SERVER) failed: %d", ::GetLastError());
    }
    if (!::WinHttpSetOption(session, WINHTTP_OPTION_MAX_CONNS_PER_1_0_SERVER,
            &maxConnections, sizeof(maxConnections)))
    {
        LOG_WARN("WinHttpSetOption(MAX_CONNS_PER_1_0_SERVER) failed: %d", ::GetLastError());
    }
}

} // namespace

class WinHttpRequestWrapper;

struct WinHttpClientState
{
    explicit WinHttpClientState(HINTERNET sessionHandle);
    ~WinHttpClientState();

    bool registerRequest(
        std::string const& id,
        std::shared_ptr<WinHttpRequestWrapper> request);
    void eraseRequest(std::string const& id);
    void stopAcceptingRequests();
    void beginCallback();
    void beginCallbackLocked();
    void endCallback();

    HINTERNET session;
    std::mutex requestsMutex;
    std::map<std::string, std::shared_ptr<WinHttpRequestWrapper>> requests;
    std::condition_variable requestsCv;
    std::atomic<bool> msRootCheck {false};
    bool acceptingRequests {true};
    size_t cancelAllDepth {0};
    size_t registryGeneration {0};
    size_t callbackGeneration {0};
    size_t callbacksInFlight {0};
    std::map<std::thread::id, size_t> callbacksByThread;
};

struct WinHttpCallbackAlreadyStarted
{
};

class WinHttpCallbackScope
{
  public:
    explicit WinHttpCallbackScope(std::shared_ptr<WinHttpClientState> state)
      : m_state(std::move(state))
    {
        m_state->beginCallback();
    }

    WinHttpCallbackScope(
        std::shared_ptr<WinHttpClientState> state,
        WinHttpCallbackAlreadyStarted)
      : m_state(std::move(state))
    {
    }

    ~WinHttpCallbackScope()
    {
        m_state->endCallback();
    }

    WinHttpCallbackScope(WinHttpCallbackScope const&) = delete;
    WinHttpCallbackScope& operator=(WinHttpCallbackScope const&) = delete;

  private:
    std::shared_ptr<WinHttpClientState> m_state;
};

// Ownership of the WinHTTP status-callback context.
//
// WinHTTP keeps the context value associated with a request handle until that
// handle is torn down, and documents WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING as
// the final callback for the handle ("There will be no more callbacks for this
// handle"). The context therefore holds a *strong* reference to the wrapper:
// every buffer WinHTTP was handed lives inside (or is kept alive by) that
// wrapper, so it stays valid for exactly as long as WinHTTP can still touch it.
// The reference is released only from the HANDLE_CLOSING callback, which also
// deletes the context.
struct WinHttpCallbackContext
{
    explicit WinHttpCallbackContext(std::shared_ptr<WinHttpRequestWrapper> request)
      : request(std::move(request))
    {
    }

    std::shared_ptr<WinHttpRequestWrapper> request;
};

class WinHttpRequestWrapper : public std::enable_shared_from_this<WinHttpRequestWrapper>
{
  protected:
    // The step the WinHTTP state machine should take next. Operations are never
    // issued directly from a completion callback; see schedule()/runPump().
    enum class NextOperation
    {
        None,
        WriteBody,
        ReceiveResponse,
        QueryDataAvailable,
        ReadData,
        Complete
    };

    std::shared_ptr<WinHttpClientState> m_clientState;
    std::string            m_id;
    IHttpResponseCallback* m_appCallback {nullptr};
    HINTERNET              m_hConnect {nullptr};
    HINTERNET              m_hRequest {nullptr};
    SimpleHttpRequest*     m_request;
    std::vector<uint8_t>   m_bodyBuffer;
    // Fixed response read buffer. WinHttpReadData keeps the pointer until the
    // read completes, so the buffer must never move for the life of the
    // request; sizing it once up front also keeps the number of read
    // completions needed to drain a response low (see MAX_HTTP_RESPONSE_SIZE,
    // which still bounds the total that is buffered).
    uint8_t                m_readBuffer[8192] {0};
    size_t                 m_bodyWritten {0};
    std::atomic<bool>      isCallbackCalled {false};
    bool                   isAborted {false};
    bool                   m_isHttps {false};
    bool                   m_msRootCheckRequired {false};
    std::atomic<bool>      m_msRootCheckCompleted {false};
    bool                   m_contextInstalled {false};
    bool                   m_sendIssued {false};
    bool                   m_handleCallInProgress {false};
    bool                   m_closeRequestAfterCall {false};
    unsigned               m_stateCallbackDepth {0};
    std::map<std::thread::id, size_t> m_stateCallbacksByThread;
    bool                   m_stateCompletionPending {false};
    DWORD                  m_stateCompletionError {ERROR_SUCCESS};
    // Reason recorded by an abort that must let WinHTTP report the terminal
    // callback itself instead of completing inline.
    std::atomic<DWORD>     m_deferredError {ERROR_SUCCESS};

    // requestsMutex may nest this mutex only while the initial send claims or
    // releases the pump. Code holding m_pumpMutex must release it before any
    // operation that acquires requestsMutex.
    std::mutex             m_pumpMutex;
    bool                   m_pumpActive {false};
    NextOperation          m_nextOperation {NextOperation::None};
    DWORD                  m_completionError {ERROR_SUCCESS};

  public:
    WinHttpRequestWrapper(
        std::shared_ptr<WinHttpClientState> clientState,
        SimpleHttpRequest* request)
      : m_clientState(std::move(clientState)),
        m_id(request->GetId()),
        m_request(request)
    {
        LOG_TRACE("%p WinHttpRequestWrapper()", this);
    }

    WinHttpRequestWrapper(WinHttpRequestWrapper const&) = delete;
    WinHttpRequestWrapper& operator=(WinHttpRequestWrapper const&) = delete;

    // The caller must hold m_clientState->requestsMutex.
    bool hasStateCallbackOnThreadLocked(std::thread::id threadId) const
    {
        return m_stateCallbacksByThread.find(threadId) !=
            m_stateCallbacksByThread.end();
    }

    // The caller must hold m_clientState->requestsMutex.
    bool hasActiveStateCallbackLocked() const
    {
        return m_stateCallbackDepth != 0;
    }

    ~WinHttpRequestWrapper() noexcept
    {
        LOG_TRACE("%p ~WinHttpRequestWrapper()", this);
        // Both completion and cancellation close the request handle explicitly:
        // while WinHTTP owns the callback context it also owns a strong
        // reference to this object, so the destructor can never be what closes
        // that handle. Anything still open here belongs to a request that
        // failed before WinHTTP took ownership of the context.
        if (m_hRequest != nullptr)
        {
            ::WinHttpCloseHandle(m_hRequest);
        }
        if (m_hConnect != nullptr)
        {
            ::WinHttpCloseHandle(m_hConnect);
        }
    }

    /// <summary>
    /// Asynchronously cancel pending request.
    ///
    /// Unlike WinInet's InternetCloseHandle, WinHttpCloseHandle on a request
    /// with a pending async operation blocks the calling thread until that
    /// operation's completion callback has finished running -- and that
    /// callback runs on a *different* WinHTTP-internal thread. Holding
    /// m_clientState->requestsMutex across the call (WinInet's pattern, safe there
    /// because its callback runs synchronously on the calling thread) would
    /// deadlock here: this thread would block inside WinHttpCloseHandle holding
    /// the lock, while the completion callback blocks on the same thread's
    /// erase() needing that same lock. So the handle is captured and closed
    /// without holding the lock. This wrapper is only reachable through a
    /// shared_ptr (see WinHttpClientState::requests / CancelRequestAsync), so
    /// releasing the lock here cannot race with the object being freed --
    /// the caller already holds its own shared_ptr keeping *this* alive.
    /// </summary>
    void cancel()
    {
        abortRequest(ERROR_WINHTTP_OPERATION_CANCELLED);
    }

    /// <summary>
    /// Tears the request down and records why, without delivering the terminal
    /// response from this call.
    ///
    /// WinHttpSendRequest documents that buffers handed to WinHTTP must stay
    /// valid until an aborted operation reports
    /// WINHTTP_CALLBACK_STATUS_REQUEST_ERROR with ERROR_WINHTTP_OPERATION_CANCELLED,
    /// and invoking OnHttpResponse() is precisely what lets the caller destroy
    /// the request object those buffers live in. Synthesizing the response as
    /// soon as WinHttpCloseHandle returns would assume a teardown ordering
    /// WinHTTP does not guarantee, so instead the handle is closed and the
    /// response is delivered from the resulting REQUEST_ERROR callback -- or
    /// from HANDLE_CLOSING, which WinHTTP always delivers last.
    /// </summary>
    void abortRequest(DWORD dwError, bool calledFromWinHttpCallback = false)
    {
        HINTERNET hRequestToClose = nullptr;
        bool completeHere = false;
        {
            std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
            if (isCallbackCalled)
            {
                return;
            }
            isAborted = true;
            DWORD noError = ERROR_SUCCESS;
            m_deferredError.compare_exchange_strong(noError, dwError);
            if (m_handleCallInProgress && !calledFromWinHttpCallback)
            {
                // WinHTTP forbids another thread from closing an asynchronous
                // handle while this thread is inside WinHttpSendRequest or
                // WinHttpWriteData. Record the cancellation and let that API
                // frame close the handle as soon as its call returns.
                m_closeRequestAfterCall = true;
                return;
            }
            hRequestToClose = m_hRequest;
            m_hRequest = nullptr;
            // Without an installed callback context WinHTTP has no way to
            // report HANDLE_CLOSING back to this object, so nothing else would
            // ever complete the request. And until WinHttpSendRequest has been
            // issued WinHTTP holds none of this request's buffers, so there is
            // nothing to wait for. Both states may be completed inline.
            completeHere = !m_contextInstalled || !m_sendIssued;
        }
        if (hRequestToClose != nullptr)
        {
            ::WinHttpCloseHandle(hRequestToClose);
        }
        if (completeHere)
        {
            onRequestComplete(dwError);
        }
    }

    /// <summary>
    /// Verify that the server end-point certificate is MS-Rooted.
    /// Unlike WinInet's INTERNET_OPTION_SERVER_CERT_CHAIN_CONTEXT (which hands
    /// back a ready-made chain), WinHttpQueryOption only returns the leaf server
    /// certificate context, so the chain must be built explicitly before running
    /// the same CERT_CHAIN_POLICY_MICROSOFT_ROOT policy check WinInet performs.
    /// </summary>
    bool isMsRootCert(HINTERNET hRequest)
    {
        PCCERT_CONTEXT pCertContext = nullptr;
        DWORD dwSize = sizeof(pCertContext);
        if (!::WinHttpQueryOption(hRequest, WINHTTP_OPTION_SERVER_CERT_CONTEXT, &pCertContext, &dwSize))
        {
            LOG_WARN("WinHttpQueryOption(SERVER_CERT_CONTEXT) failed: %d", ::GetLastError());
            return false;
        }

        bool result = true;
        PCCERT_CHAIN_CONTEXT pChainCtx = nullptr;
        CERT_CHAIN_PARA chainPara = { sizeof(chainPara) };
        if (::CertGetCertificateChain(NULL, pCertContext, NULL, pCertContext->hCertStore, &chainPara, 0, NULL, &pChainCtx))
        {
            CERT_CHAIN_POLICY_STATUS pps = { 0, 0, 0, 0, nullptr };
            pps.cbSize = sizeof(pps);
            // Verify that the cert chain roots up to the Microsoft application root at top level
            CERT_CHAIN_POLICY_PARA policyPara = { 0, 0, nullptr };
            policyPara.cbSize = sizeof(policyPara);
            policyPara.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;
            policyPara.pvExtraPolicyPara = nullptr;

            BOOL policyChecked = ::CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_MICROSOFT_ROOT, pChainCtx, &policyPara, &pps);
            if (!policyChecked)
            {
                LOG_WARN("CertVerifyCertificateChainPolicy() failed: unable to verify");
                result = false;
            }
            else if (pps.dwError != ERROR_SUCCESS)
            {
                LOG_WARN("CertVerifyCertificateChainPolicy() failed: invalid root CA - %d", pps.dwError);
                result = false;
            }
            ::CertFreeCertificateChain(pChainCtx);
        }
        else
        {
            LOG_WARN("CertGetCertificateChain() failed: %d", ::GetLastError());
            result = false;
        }
        ::CertFreeCertificateContext(pCertContext);
        return result;
    }

    HINTERNET getRequestHandle()
    {
        std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
        return m_hRequest;
    }

    // Keep each WinHTTP operation and the handle check under the same lock as
    // cancellation. WinHttpCloseHandle remains outside the lock because it
    // waits for callbacks that may need this mutex.
    DWORD receiveResponse()
    {
        std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
        if (m_hRequest == nullptr)
        {
            return ERROR_WINHTTP_OPERATION_CANCELLED;
        }
        if (!::WinHttpReceiveResponse(m_hRequest, NULL))
        {
            return ::GetLastError();
        }
        return ERROR_SUCCESS;
    }

    DWORD queryDataAvailable()
    {
        std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
        if (m_hRequest == nullptr)
        {
            return ERROR_WINHTTP_OPERATION_CANCELLED;
        }
        if (!::WinHttpQueryDataAvailable(m_hRequest, NULL))
        {
            return ::GetLastError();
        }
        return ERROR_SUCCESS;
    }

    DWORD readData()
    {
        std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
        if (m_hRequest == nullptr)
        {
            return ERROR_WINHTTP_OPERATION_CANCELLED;
        }
        if (!::WinHttpReadData(m_hRequest, m_readBuffer,
                static_cast<DWORD>(sizeof(m_readBuffer)), NULL))
        {
            return ::GetLastError();
        }
        return ERROR_SUCCESS;
    }

    // Hands the remaining request body to WinHTTP. The body is deliberately not
    // passed as WinHttpSendRequest's lpOptional: that buffer belongs to the
    // caller's request object and WinHTTP may hold it until the request handle
    // is closed, whereas WinHttpWriteData releases it at WRITE_COMPLETE.
    DWORD writeBody()
    {
        HINTERNET request = nullptr;
        const void* body = nullptr;
        DWORD bodySize = 0;
        {
            std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
            if (m_hRequest == nullptr)
            {
                return ERROR_WINHTTP_OPERATION_CANCELLED;
            }
            size_t remaining = m_request->m_body.size() - m_bodyWritten;
            request = m_hRequest;
            body = m_request->m_body.data() + m_bodyWritten;
            bodySize = static_cast<DWORD>(remaining);
            m_handleCallInProgress = true;
        }

        BOOL result = ::WinHttpWriteData(request, body, bodySize, NULL);
        DWORD error = result ? ERROR_SUCCESS : ::GetLastError();

        HINTERNET cancelledRequest = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
            m_handleCallInProgress = false;
            if (m_closeRequestAfterCall)
            {
                m_closeRequestAfterCall = false;
                cancelledRequest = m_hRequest;
                m_hRequest = nullptr;
            }
        }
        if (cancelledRequest != nullptr)
        {
            ::WinHttpCloseHandle(cancelledRequest);
        }
        return error;
    }

    DWORD validateCurrentRequestMsRootCert()
    {
        std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
        if (m_hRequest == nullptr)
        {
            return ERROR_WINHTTP_OPERATION_CANCELLED;
        }
        return isMsRootCert(m_hRequest) ? ERROR_SUCCESS : ERROR_WINHTTP_SECURE_INVALID_CERT;
    }

    // Detaches and closes the request handle. WinHttpCloseHandle can block
    // until an in-flight callback returns, and that callback may need
    // m_clientState->requestsMutex, so the handle is detached under the lock and
    // closed without it.
    void closeRequestHandle()
    {
        HINTERNET hRequestToClose = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
            hRequestToClose = m_hRequest;
            m_hRequest = nullptr;
        }
        if (hRequestToClose != nullptr)
        {
            ::WinHttpCloseHandle(hRequestToClose);
        }
    }

    // Queues the next step of the WinHTTP state machine.
    //
    // WinHTTP is explicitly allowed to complete an operation synchronously and
    // re-enter this object's status callback on the calling thread ("reentered
    // on the same thread for the current request"). Issuing the next WinHTTP
    // call straight from a completion would then nest a pair of stack frames
    // per response chunk -- unbounded for a large response -- and would also
    // re-enter m_clientState->requestsMutex, which is not recursive. So only the
    // outermost frame ever issues operations: a nested completion records what
    // should happen next and returns, and runPump() picks it up once the
    // WinHTTP call it was nested inside has returned.
    void schedule(NextOperation next, DWORD completionError = ERROR_SUCCESS)
    {
        {
            std::lock_guard<std::mutex> lock(m_pumpMutex);
            if (m_nextOperation == NextOperation::Complete && next != NextOperation::Complete)
            {
                // A terminal result is already queued; nothing may displace it.
                return;
            }
            m_nextOperation = next;
            m_completionError = completionError;
            if (m_pumpActive)
            {
                return;
            }
            m_pumpActive = true;
        }
        runPump();
    }

    // Issues queued operations until WinHTTP takes one asynchronously. The
    // caller must already own the pump (m_pumpActive set) and must not hold
    // m_clientState->requestsMutex.
    void runPump()
    {
        for (;;)
        {
            NextOperation current = NextOperation::None;
            DWORD completionError = ERROR_SUCCESS;
            {
                std::lock_guard<std::mutex> lock(m_pumpMutex);
                current = m_nextOperation;
                completionError = m_completionError;
                m_nextOperation = NextOperation::None;
                if (current == NextOperation::None || isCallbackCalled)
                {
                    m_pumpActive = false;
                    return;
                }
                if (current == NextOperation::Complete)
                {
                    m_pumpActive = false;
                }
            }

            if (current == NextOperation::Complete)
            {
                onRequestComplete(completionError);
                return;
            }

            DWORD dwError = issueOperation(current);
            if (dwError == ERROR_SUCCESS)
            {
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(m_pumpMutex);
                m_nextOperation = NextOperation::None;
                m_pumpActive = false;
            }
            if (current == NextOperation::WriteBody)
            {
                // A synchronous WinHttpWriteData failure leaves no documented
                // way to prove WinHTTP has let go of the caller's body buffer,
                // so let the handle's final callback deliver the response.
                abortRequest(dwError);
            }
            else
            {
                onRequestComplete(dwError);
            }
            return;
        }
    }

    DWORD issueOperation(NextOperation operation)
    {
        switch (operation)
        {
            case NextOperation::WriteBody:
                return writeBody();

            case NextOperation::ReceiveResponse:
                return receiveResponse();

            case NextOperation::QueryDataAvailable:
                return queryDataAvailable();

            case NextOperation::ReadData:
                return readData();

            default:
                return ERROR_SUCCESS;
        }
    }

    void DispatchEvent(std::unique_lock<std::mutex>& lock, HttpStateEvent type)
    {
        if (m_appCallback != nullptr && !isCallbackCalled)
        {
            void* handle = static_cast<void*>(m_hRequest);
            IHttpResponseCallback* callback = m_appCallback;
            auto state = m_clientState;
            ++m_stateCallbackDepth;
            ++m_stateCallbacksByThread[std::this_thread::get_id()];
            state->beginCallbackLocked();
            lock.unlock();
            {
                WinHttpCallbackScope callbackScope(
                    state, WinHttpCallbackAlreadyStarted {});
                callback->OnHttpStateEvent(type, handle, 0);
            }

            bool complete = false;
            DWORD completionError = ERROR_SUCCESS;
            {
                lock.lock();
                assert(m_stateCallbackDepth != 0);
                --m_stateCallbackDepth;
                auto stateCallback = m_stateCallbacksByThread.find(
                    std::this_thread::get_id());
                assert(stateCallback != m_stateCallbacksByThread.end());
                if (stateCallback != m_stateCallbacksByThread.end() &&
                    --stateCallback->second == 0)
                {
                    m_stateCallbacksByThread.erase(stateCallback);
                }
                if (m_stateCallbackDepth == 0 && m_stateCompletionPending)
                {
                    complete = true;
                    completionError = m_stateCompletionError;
                    m_stateCompletionPending = false;
                    m_stateCompletionError = ERROR_SUCCESS;
                }
            }
            if (complete)
            {
                // Terminal delivery may free the application callback. Leave the
                // setup lock released, matching the existing DispatchEvent
                // contract when a state callback synchronously completes.
                lock.unlock();
                onRequestComplete(completionError);
            }
        }
    }

    // Asynchronously send HTTP request and invoke response callback.
    // Ownership semantics: send(...) method self-destroys *this* upon
    // reaching the terminal WinHTTP callback. There must be absolutely no
    // methods that attempt to use the object after triggering send on it.
    // Send operation on request may be issued no more than once.
    //
    // Handle setup runs under m_clientState->requestsMutex. State callbacks are the
    // deliberate exception: DispatchEvent releases the lock while invoking
    // application code, then setup checks cancellation before continuing.
    //
    // DEADLOCK NOTE: the lock must NOT still be held when a synchronous
    // failure completes the request. onRequestComplete() invokes the
    // application callback, which is documented (below) to be able to tear the
    // client down synchronously -- that reaches CancelAllRequests(), which
    // waits on the shared state's condition variable. DispatchEvent releases this lock
    // around application state callbacks. If a callback completes the request,
    // it leaves the lock released and sendLocked() returns without touching the
    // client again; otherwise it reacquires the lock before setup continues.
    void send(IHttpResponseCallback* callback)
    {
        m_appCallback = callback;
        std::shared_ptr<WinHttpRequestWrapper> keepAlive = shared_from_this();
        if (!m_clientState->registerRequest(m_id, keepAlive))
        {
            onRequestComplete(ERROR_WINHTTP_OPERATION_CANCELLED);
            return;
        }

        bool failed = false;
        DWORD dwError = ERROR_SUCCESS;
        {
            std::unique_lock<std::mutex> lock(m_clientState->requestsMutex);
            failed = !sendLocked(lock, dwError);
        }
        if (failed)
        {
            onRequestComplete(dwError);
            return;
        }
        // sendLocked() claimed the pump before calling WinHttpSendRequest, so a
        // completion WinHTTP delivered synchronously on this thread could only
        // park the next step instead of issuing it while the setup lock was
        // still held. Run whatever it parked now that the lock is gone.
        runPump();
    }

    // Returns true if the request was handed off to WinHTTP asynchronously.
    // Returns false on synchronous failure, setting dwError to the result the
    // caller must complete the request with (once the lock has been dropped).
    bool sendLocked(std::unique_lock<std::mutex>& lock, DWORD& dwErrorOut)
    {
        if (isCallbackCalled || isAborted)
        {
            // Request force-aborted before creating a WinHTTP handle.
            if (!isCallbackCalled)
            {
                DispatchEvent(lock, OnConnectFailed);
            }
            dwErrorOut = ERROR_WINHTTP_OPERATION_CANCELLED;
            return false;
        }

        DispatchEvent(lock, OnConnecting);
        if (isCallbackCalled || isAborted)
        {
            dwErrorOut = ERROR_WINHTTP_OPERATION_CANCELLED;
            return false;
        }

        std::wstring wUrl = to_utf16_string(m_request->m_url);
        URL_COMPONENTS urlc;
        memset(&urlc, 0, sizeof(urlc));
        urlc.dwStructSize = sizeof(urlc);
        wchar_t hostname[256] = { 0 };
        urlc.lpszHostName = hostname;
        urlc.dwHostNameLength = ARRAYSIZE(hostname);
        wchar_t path[1024] = { 0 };
        urlc.lpszUrlPath = path;
        urlc.dwUrlPathLength = ARRAYSIZE(path);
        if (!::WinHttpCrackUrl(wUrl.c_str(), static_cast<DWORD>(wUrl.size()), 0, &urlc))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpCrackUrl() failed: dwError=%d url=%s", dwError, m_request->m_url.c_str());
            // Invalid URL passed to WinHTTP API
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        if (m_clientState->session == nullptr)
        {
            LOG_WARN("WinHttpOpen() did not produce a usable session handle");
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = ERROR_WINHTTP_CANNOT_CONNECT;
            return false;
        }

        // TODO: connect handle for the same target should be cached across
        // requests to enable keep-alive (same pre-existing opportunity noted
        // in HttpClient_WinInet.cpp; out of scope for this transport swap).
        m_hConnect = ::WinHttpConnect(m_clientState->session, hostname, urlc.nPort, 0);
        if (m_hConnect == nullptr)
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpConnect() failed: %d", dwError);
            // Cannot connect to host
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        std::wstring wMethod = to_utf16_string(m_request->m_method);
        m_isHttps = (urlc.nScheme == INTERNET_SCHEME_HTTPS);
        // Latch the policy for this request: the callbacks that enforce it run
        // long after send() returns, and the setting can be changed at any time.
        m_msRootCheckRequired =
            m_clientState->msRootCheck.load(std::memory_order_acquire);
        m_hRequest = ::WinHttpOpenRequest(
            m_hConnect, wMethod.c_str(), path, NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_REFRESH | (m_isHttps ? WINHTTP_FLAG_SECURE : 0));
        if (m_hRequest == nullptr)
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpOpenRequest() failed: %d", dwError);
            // Request cannot be opened to given URL because of some connectivity issue
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        // Match the WinInet transport's INTERNET_FLAG_NO_AUTH behavior.
        // Telemetry requests must not answer server or proxy authentication
        // challenges with ambient process credentials.
        DWORD disableFeatures = WINHTTP_DISABLE_AUTHENTICATION;
        if (m_msRootCheckRequired)
        {
            // Automatic redirects would move the request to a new TLS peer
            // after the original certificate check, potentially forwarding
            // telemetry credentials to a non-Microsoft-root endpoint.
            disableFeatures |= WINHTTP_DISABLE_REDIRECTS;
        }
        if (!::WinHttpSetOption(
                m_hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &disableFeatures, sizeof(disableFeatures)))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpSetOption(DISABLE_AUTHENTICATION) failed: %d", dwError);
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        // Unlike WinInet, WinHTTP has no automatic cookie jar to suppress (it
        // never manages cookies on the caller's behalf) and never shows UI, so
        // neither INTERNET_FLAG_NO_COOKIES nor INTERNET_FLAG_NO_UI has a WinHTTP
        // equivalent to set here.

        // WinHttpSetStatusCallback returns the PREVIOUS callback function
        // pointer (typically NULL here, since this is the first registration
        // on a freshly opened request handle) -- not a BOOL -- and signals
        // failure only via the distinct WINHTTP_INVALID_STATUS_CALLBACK
        // sentinel. Treating a null "previous callback" as failure would
        // reject every request immediately after this call.
        if (::WinHttpSetStatusCallback(m_hRequest, &WinHttpRequestWrapper::winHttpCallback,
                WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS |
                    WINHTTP_CALLBACK_FLAG_HANDLES |
                    WINHTTP_CALLBACK_FLAG_SEND_REQUEST,
                0) == WINHTTP_INVALID_STATUS_CALLBACK)
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpSetStatusCallback() failed: %d", dwError);
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        // Install the callback context explicitly, before anything else can
        // fail. Relying on WinHttpSendRequest's dwContext instead would strand
        // the context (and the strong reference it holds) whenever the send
        // fails before WinHTTP records it -- WinHTTP would then report
        // HANDLE_CLOSING with a zero context and nothing would free it. Once
        // the option is set, the handle owns the context and HANDLE_CLOSING is
        // guaranteed to hand it back. Until then unique_ptr owns it, so no path
        // out of this function can leak it.
        std::unique_ptr<WinHttpCallbackContext> context(new WinHttpCallbackContext(shared_from_this()));
        DWORD_PTR contextValue = reinterpret_cast<DWORD_PTR>(context.get());
        if (!::WinHttpSetOption(
                m_hRequest, WINHTTP_OPTION_CONTEXT_VALUE, &contextValue, sizeof(contextValue)))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpSetOption(CONTEXT_VALUE) failed: %d", dwError);
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }
        context.release();
        m_contextInstalled = true;

        std::ostringstream os;
        for (auto const& header : m_request->m_headers) {
            os << header.first << ": " << header.second << "\r\n";
        }
        std::wstring wHeaders = to_utf16_string(os.str());

        if (!wHeaders.empty() &&
            wHeaders.size() > static_cast<size_t>(std::numeric_limits<DWORD>::max()))
        {
            LOG_WARN("Request headers exceed WinHTTP's maximum size");
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = ERROR_INVALID_PARAMETER;
            return false;
        }
        if (!wHeaders.empty() &&
            !::WinHttpAddRequestHeaders(m_hRequest, wHeaders.c_str(), static_cast<DWORD>(wHeaders.size()),
                WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpAddRequestHeaders() failed: %d", dwError);
            // Unable to add request headers. There's no point in proceeding with upload because
            // our server is expecting those custom request headers to always be there.
            DispatchEvent(lock, OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        // Try to send headers and request body to server
        DispatchEvent(lock, OnSending);
        if (isCallbackCalled || isAborted)
        {
            dwErrorOut = ERROR_WINHTTP_OPERATION_CANCELLED;
            return false;
        }
        if (m_request->m_body.size() > static_cast<size_t>(std::numeric_limits<DWORD>::max()))
        {
            LOG_WARN("Request body exceeds WinHTTP's maximum size");
            DispatchEvent(lock, OnSendFailed);
            dwErrorOut = ERROR_INVALID_PARAMETER;
            return false;
        }
        if (m_hRequest == nullptr)
        {
            dwErrorOut = ERROR_WINHTTP_OPERATION_CANCELLED;
            return false;
        }
        // Send the headers only. dwTotalLength still declares Content-Length, so
        // the server sees the same request; the body follows via
        // WinHttpWriteData. The SENDING_REQUEST callback validates the negotiated
        // certificate before WinHTTP commits these headers to the wire.
        DWORD totalLength = static_cast<DWORD>(m_request->m_body.size());
        // Claim the pump so that a completion WinHTTP may deliver synchronously
        // on this thread parks its next step instead of issuing a WinHTTP call
        // (and re-entering the shared-state mutex) while setup still holds the lock.
        // send() releases the pump once the lock is gone.
        {
            std::lock_guard<std::mutex> pumpLock(m_pumpMutex);
            m_pumpActive = true;
            m_nextOperation = NextOperation::None;
        }
        m_sendIssued = true;
        m_handleCallInProgress = true;
        HINTERNET hRequest = m_hRequest;
        // SENDING_REQUEST may run synchronously from WinHttpSendRequest and must
        // acquire requestsMutex to enforce the certificate policy. Keep the
        // wrapper alive, but release the registry lock across the WinHTTP call.
        lock.unlock();
        BOOL bResult = ::WinHttpSendRequest(
            hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, totalLength, contextValue);
        DWORD dwSendError = bResult ? ERROR_SUCCESS : ::GetLastError();
        lock.lock();
        m_handleCallInProgress = false;
        HINTERNET cancelledRequest = nullptr;
        if (m_closeRequestAfterCall)
        {
            m_closeRequestAfterCall = false;
            cancelledRequest = m_hRequest;
            m_hRequest = nullptr;
        }
        if (cancelledRequest != nullptr)
        {
            // Closing the handle may synchronously invoke a terminal callback,
            // which acquires requestsMutex through onRequestComplete().
            lock.unlock();
            ::WinHttpCloseHandle(cancelledRequest);
            lock.lock();
        }
        if (!bResult)
        {
            DWORD dwError = m_deferredError.load(std::memory_order_acquire);
            if (dwError == ERROR_SUCCESS)
            {
                dwError = dwSendError;
            }
            {
                std::lock_guard<std::mutex> pumpLock(m_pumpMutex);
                m_pumpActive = false;
                m_nextOperation = NextOperation::None;
            }
            // The send never started, so WinHTTP holds none of this request's
            // buffers and cancellation may still complete inline. It does keep
            // the context on the request handle and delivers HANDLE_CLOSING once
            // onRequestComplete() closes that handle, which is what frees it.
            m_sendIssued = false;
            LOG_WARN("WinHttpSendRequest() failed: %d", dwError);
            // Unable to send request
            DispatchEvent(lock, OnSendFailed);
            dwErrorOut = dwError;
            return false;
        }
        // Async request has been queued; completion arrives via winHttpCallback.
        return true;
    }

    // Drives the WinHTTP async state machine: SendRequest -> (certificate
    // policy) -> WriteData -> ReceiveResponse -> (QueryDataAvailable ->
    // ReadData)* -> onRequestComplete. Unlike WinInet (whose async completions
    // all report through the single INTERNET_STATUS_REQUEST_COMPLETE code, and
    // whose synchronous API calls signal a pending async op via a FALSE return
    // + GetLastError()==ERROR_IO_PENDING), WinHTTP has one distinct callback
    // status per stage, and a FALSE return from any of these calls on an async
    // handle is always a genuine synchronous failure -- never "pending".
    //
    // No stage issues the next WinHTTP call directly: everything goes through
    // schedule(), so a completion WinHTTP delivers synchronously on the calling
    // thread cannot nest another operation inside the one it is reporting.
    static void CALLBACK winHttpCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
    {
        UNREFERENCED_PARAMETER(hInternet);

        WinHttpCallbackContext* context = reinterpret_cast<WinHttpCallbackContext*>(dwContext);
        if (context == nullptr)
        {
            return;
        }

        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)
        {
            // Documented as the final callback for this handle, so WinHTTP no
            // longer references anything this request handed it. Release the
            // context -- and with it the strong reference that has been keeping
            // the wrapper (and its read buffer) alive -- but only after using it
            // as the backstop that guarantees every request produces exactly one
            // terminal response, including the cancellation paths that
            // deliberately do not complete inline.
            std::shared_ptr<WinHttpRequestWrapper> self = context->request;
            delete context;
            if (self != nullptr && !self->isCallbackCalled)
            {
                self->onRequestComplete(self->m_deferredError.exchange(ERROR_SUCCESS));
            }
            return;
        }

        std::shared_ptr<WinHttpRequestWrapper> self = context->request;
        if (self == nullptr || self->isCallbackCalled)
        {
            // The terminal response has already been delivered; the request is
            // no longer tracked by the client, which may since have been torn
            // down. Nothing here may touch it again.
            return;
        }

        LOG_TRACE("winHttpCallback: hInternet %p, self %p, dwInternetStatus %u", hInternet, self.get(), dwInternetStatus);

        switch (dwInternetStatus)
        {
            case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:
                // TLS is negotiated, but the request headers have not left the
                // process. Enforce the configured Microsoft-root policy here so
                // API keys and auth tickets are never disclosed to a server that
                // only passes the platform's broader certificate policy.
                if (self->m_isHttps && self->m_msRootCheckRequired &&
                    !self->m_msRootCheckCompleted.exchange(true))
                {
                    DWORD dwError = self->validateCurrentRequestMsRootCert();
                    if (dwError != ERROR_SUCCESS)
                    {
                        // WinHTTP permits closing a handle from its own status
                        // callback even while WinHttpSendRequest is active. Do
                        // that here so rejected credentials never leave the
                        // process; external cancellation uses the deferred path.
                        self->abortRequest(dwError, true);
                    }
                }
                return;

            case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
                self->schedule(self->m_request->m_body.empty()
                    ? NextOperation::ReceiveResponse
                    : NextOperation::WriteBody);
                return;

            case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
            {
                // WinHTTP has released the caller's body buffer for the bytes it
                // reports here. Short writes are not expected, but honour them
                // rather than truncating the payload.
                DWORD written = (lpvStatusInformation != nullptr)
                    ? *static_cast<DWORD*>(lpvStatusInformation) : 0;
                self->m_bodyWritten += written;
                if (self->m_bodyWritten < self->m_request->m_body.size())
                {
                    if (written == 0)
                    {
                        self->schedule(NextOperation::Complete, ERROR_WINHTTP_CONNECTION_ERROR);
                        return;
                    }
                    self->schedule(NextOperation::WriteBody);
                    return;
                }
                self->schedule(NextOperation::ReceiveResponse);
                return;
            }

            case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
                // The certificate policy was already enforced before the
                // request headers were transmitted.
                self->schedule(NextOperation::QueryDataAvailable);
                return;

            case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
            {
                DWORD bytesAvailable = (lpvStatusInformation != nullptr)
                    ? *static_cast<DWORD*>(lpvStatusInformation) : 0;
                if (bytesAvailable == 0)
                {
                    // No more data: response is complete.
                    self->schedule(NextOperation::Complete, ERROR_SUCCESS);
                    return;
                }
                // SECURITY: refuse an over-large response instead of buffering it
                // (see MAX_HTTP_RESPONSE_SIZE) so a hostile/MITM'd collector cannot
                // exhaust process memory. Checked before every read so the buffer
                // never exceeds the cap; reported as an invalid server response ->
                // NetworkFailure (retried).
                if (self->m_bodyBuffer.size() > MAX_HTTP_RESPONSE_SIZE ||
                    bytesAvailable > MAX_HTTP_RESPONSE_SIZE - self->m_bodyBuffer.size())
                {
                    LOG_WARN("HTTP response exceeds max buffered size (%zu bytes); aborting", MAX_HTTP_RESPONSE_SIZE);
                    self->schedule(NextOperation::Complete, ERROR_WINHTTP_INVALID_SERVER_RESPONSE);
                    return;
                }
                // readData() takes whatever fits in the fixed buffer; anything
                // beyond that is reported again by the next QueryDataAvailable.
                self->schedule(NextOperation::ReadData);
                return;
            }

            case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
                // dwStatusInformationLength is the number of bytes actually placed
                // into the buffer passed to WinHttpReadData (may be less than the
                // buffer size that was offered).
                if (dwStatusInformationLength > sizeof(self->m_readBuffer) ||
                    self->m_bodyBuffer.size() > MAX_HTTP_RESPONSE_SIZE ||
                    dwStatusInformationLength > MAX_HTTP_RESPONSE_SIZE - self->m_bodyBuffer.size())
                {
                    self->schedule(NextOperation::Complete, ERROR_WINHTTP_INVALID_SERVER_RESPONSE);
                    return;
                }
                self->m_bodyBuffer.insert(self->m_bodyBuffer.end(),
                    self->m_readBuffer, self->m_readBuffer + dwStatusInformationLength);
                self->schedule(NextOperation::QueryDataAvailable);
                return;

            case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
            {
                DWORD dwError = ERROR_WINHTTP_INTERNAL_ERROR;
                if (lpvStatusInformation != nullptr &&
                    dwStatusInformationLength >= sizeof(WINHTTP_ASYNC_RESULT))
                {
                    dwError = static_cast<WINHTTP_ASYNC_RESULT*>(lpvStatusInformation)->dwError;
                }
                // The operation that owned the buffers WinHTTP was given has
                // finished failing, so the response may be handed back now. A
                // locally recorded abort reason wins over WinHTTP's generic
                // "operation cancelled".
                DWORD deferred = self->m_deferredError.exchange(ERROR_SUCCESS);
                self->schedule(NextOperation::Complete, (deferred != ERROR_SUCCESS) ? deferred : dwError);
                return;
            }

            default:
                return;
        }
    }

    void onRequestComplete(DWORD dwError)
    {
        {
            std::lock_guard<std::mutex> lock(m_clientState->requestsMutex);
            if (m_stateCallbackDepth != 0)
            {
                m_stateCompletionPending = true;
                m_stateCompletionError = dwError;
                return;
            }
            if (isCallbackCalled.exchange(true))
            {
                return;
            }
        }

        std::unique_ptr<SimpleHttpResponse> response(new SimpleHttpResponse(m_id));
        // Closing the request handle below releases WinHTTP's callback context,
        // and that context holds the strong reference that has been keeping
        // this object alive. Hold one here so the rest of this method -- and
        // the application callback it invokes -- cannot run on a freed object.
        auto keepAlive = shared_from_this();
        HINTERNET request = getRequestHandle();
        if (dwError == ERROR_SUCCESS && request == nullptr)
        {
            dwError = ERROR_WINHTTP_OPERATION_CANCELLED;
        }
        bool const receivedResponse = dwError == ERROR_SUCCESS;

        if (dwError == ERROR_SUCCESS) {
            response->m_body = std::move(m_bodyBuffer);
            response->m_result = HttpResult_OK;

            DWORD statusCode = 0;
            DWORD dwSize = sizeof(statusCode);
            if (!::WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                    WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &dwSize, WINHTTP_NO_HEADER_INDEX))
            {
                LOG_WARN("WinHttpQueryHeaders(STATUS_CODE) failed: %d", ::GetLastError());
                response->m_result = HttpResult_NetworkFailure;
            }
            response->m_statusCode = statusCode;

            // Raw headers, as "Name: Value\r\n..." pairs -- the same shape WinInet
            // hands back via HTTP_QUERY_RAW_HEADERS_CRLF.
            DWORD headerBytes = 0;
            BOOL headersQueried = ::WinHttpQueryHeaders(
                request, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &headerBytes,
                WINHTTP_NO_HEADER_INDEX);
            DWORD headerErr = headersQueried ? ERROR_SUCCESS : ::GetLastError();
            if (!headersQueried && headerErr == ERROR_INSUFFICIENT_BUFFER && headerBytes > 0)
            {
                if (headerBytes % sizeof(wchar_t) != 0)
                {
                    LOG_WARN("WinHttpQueryHeaders(RAW_HEADERS_CRLF) returned an invalid byte count: %lu", headerBytes);
                }
                else
                {
                    std::wstring wHeaders(headerBytes / sizeof(wchar_t), L'\0');
                    DWORD bufferBytes = headerBytes;
                    if (::WinHttpQueryHeaders(
                            request, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                            WINHTTP_HEADER_NAME_BY_INDEX, &wHeaders[0], &bufferBytes,
                            WINHTTP_NO_HEADER_INDEX))
                    {
                        // WinHttpQueryHeaders includes the buffer's trailing NUL(s) in
                        // the byte count; trim at the first one before converting.
                        size_t nul = wHeaders.find(L'\0');
                        if (nul != std::wstring::npos)
                        {
                            wHeaders.resize(nul);
                        }
                        parseHeaders(to_utf8_string(wHeaders), *response);
                    }
                    else
                    {
                        LOG_WARN("WinHttpQueryHeaders(RAW_HEADERS_CRLF) failed twice: %d", ::GetLastError());
                    }
                }
            }
            else if (!headersQueried)
            {
                LOG_WARN("WinHttpQueryHeaders(RAW_HEADERS_CRLF) failed: %d", headerErr);
            }
        } else {
            switch (dwError) {
                case ERROR_WINHTTP_OPERATION_CANCELLED:
                    response->m_result = HttpResult_Aborted;
                    break;

                case ERROR_WINHTTP_TIMEOUT:
                case ERROR_WINHTTP_NAME_NOT_RESOLVED:
                case ERROR_WINHTTP_CANNOT_CONNECT:
                case ERROR_WINHTTP_CONNECTION_ERROR:
                case ERROR_WINHTTP_RESEND_REQUEST:
                case ERROR_WINHTTP_SECURE_CERT_DATE_INVALID:
                case ERROR_WINHTTP_SECURE_CERT_CN_INVALID:
                case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED:
                case ERROR_WINHTTP_SECURE_INVALID_CA:
                case ERROR_WINHTTP_SECURE_CERT_REV_FAILED:
                case ERROR_WINHTTP_SECURE_CHANNEL_ERROR:
                case ERROR_WINHTTP_SECURE_INVALID_CERT:
                case ERROR_WINHTTP_SECURE_CERT_REVOKED:
                case ERROR_WINHTTP_SECURE_CERT_WRONG_USAGE:
                case ERROR_WINHTTP_SECURE_FAILURE:
                case ERROR_WINHTTP_REDIRECT_FAILED:
                case ERROR_WINHTTP_INVALID_SERVER_RESPONSE:
                case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW:
                    response->m_result = HttpResult_NetworkFailure;
                    break;

                default:
                    response->m_result = HttpResult_LocalFailure;
                    break;
            }
        }

        {
            auto state = m_clientState;
            WinHttpCallbackScope callbackScope(state);
            auto callback = m_appCallback;
            auto requestId = m_id;
            // Let go of the request handle before entering application code:
            // OnHttpResponse() is what allows the caller to destroy the request
            // object whose body buffer WinHTTP was given, so WinHTTP must be
            // done with this request first. Closing it is also what triggers
            // HANDLE_CLOSING, which releases the callback context.
            closeRequestHandle();
            // Remove the request before entering application code. The callback
            // can synchronously tear down the client and destroy this wrapper.
            state->eraseRequest(requestId);
            if (callback != nullptr)
            {
                // The implementation-specific handle is no longer valid once
                // terminal delivery begins, so do not expose a stale handle.
                if (receivedResponse)
                {
                    callback->OnHttpStateEvent(OnResponse, nullptr, 0);
                }
                callback->OnHttpResponse(response.release());
            }
        }
    }

  private:
    // Parses "Name: Value\r\n"-formatted raw headers (as returned by
    // WINHTTP_QUERY_RAW_HEADERS_CRLF / HTTP_QUERY_RAW_HEADERS_CRLF) into an
    // HttpHeaders map. Shared shape with HttpClient_WinInet's inline parser.
    static void parseHeaders(std::string const& raw, SimpleHttpResponse& response)
    {
        size_t lineStart = 0;
        while (lineStart < raw.size()) {
            size_t lineEnd = raw.find("\r\n", lineStart);
            if (lineEnd == std::string::npos) {
                lineEnd = raw.size();
            }

            const std::string line = raw.substr(lineStart, lineEnd - lineStart);
            const size_t colon = line.find(':');
            if (colon != std::string::npos) {
                size_t valueStart = colon + 1;
                while (valueStart < line.size() && line[valueStart] == ' ') {
                    ++valueStart;
                }
                response.m_headers.add(line.substr(0, colon), line.substr(valueStart));
            }

            if (lineEnd == raw.size()) {
                break;
            }
            lineStart = lineEnd + 2;
        }
    }
};

//---

WinHttpClientState::WinHttpClientState(HINTERNET sessionHandle) :
    session(sessionHandle)
{
}

WinHttpClientState::~WinHttpClientState()
{
    if (session != nullptr)
    {
        ::WinHttpCloseHandle(session);
    }
}

bool WinHttpClientState::registerRequest(
    std::string const& id,
    std::shared_ptr<WinHttpRequestWrapper> request)
{
    std::lock_guard<std::mutex> lock(requestsMutex);
    if (!acceptingRequests)
    {
        return false;
    }
    requests[id] = std::move(request);
    ++registryGeneration;
    bool const shouldSend = cancelAllDepth == 0;
    requestsCv.notify_all();
    return shouldSend;
}

void WinHttpClientState::eraseRequest(std::string const& id)
{
    std::lock_guard<std::mutex> lock(requestsMutex);
    requests.erase(id);
    ++registryGeneration;
    requestsCv.notify_all();
}

void WinHttpClientState::stopAcceptingRequests()
{
    std::lock_guard<std::mutex> lock(requestsMutex);
    acceptingRequests = false;
}

void WinHttpClientState::beginCallback()
{
    std::lock_guard<std::mutex> lock(requestsMutex);
    beginCallbackLocked();
    requestsCv.notify_all();
}

void WinHttpClientState::beginCallbackLocked()
{
    ++callbacksInFlight;
    ++callbacksByThread[std::this_thread::get_id()];
    ++callbackGeneration;
}

void WinHttpClientState::endCallback()
{
    std::lock_guard<std::mutex> lock(requestsMutex);
    auto it = callbacksByThread.find(std::this_thread::get_id());
    if (callbacksInFlight == 0)
    {
        LOG_ERROR("WinHTTP callback accounting underflow");
        requestsCv.notify_all();
        return;
    }

    --callbacksInFlight;
    if (it == callbacksByThread.end() || it->second == 0)
    {
        LOG_ERROR("WinHTTP callback thread was not registered");
    }
    else if (--it->second == 0)
    {
        callbacksByThread.erase(it);
    }
    ++callbackGeneration;
    requestsCv.notify_all();
}

unsigned HttpClient_WinHttp::s_nextRequestId = 0;

HttpClient_WinHttp::HttpClient_WinHttp()
{
    // WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY (Windows 8.1+) resolves the proxy
    // without depending on a logged-on interactive user or that user's
    // Internet Explorer settings -- unlike WinInet's
    // INTERNET_OPEN_TYPE_PRECONFIG, which requires one. This is why WinHTTP,
    // not WinInet, is Microsoft's documented recommendation for services and
    // other non-interactive processes. On an older OS that rejects this access
    // type, fall back to the machine-wide WinHTTP proxy configuration. This is
    // the documented pre-Windows-8.1 behavior and avoids bypassing enterprise
    // proxies entirely. Only fall back for the compatibility error; other
    // failures should not be hidden by a second, unrelated WinHttpOpen call.
    HINTERNET session = ::WinHttpOpen(
        NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
    if (session == nullptr)
    {
        DWORD dwError = ::GetLastError();
        if (dwError == ERROR_INVALID_PARAMETER)
        {
            LOG_WARN("WinHttpOpen(AUTOMATIC_PROXY) is unsupported; retrying with default proxy");
            session = ::WinHttpOpen(
                NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
        }
        else
        {
            LOG_WARN("WinHttpOpen(AUTOMATIC_PROXY) failed: %lu", dwError);
        }
    }
    // WinHTTP otherwise permits an unlimited number of connections per origin.
    // Keep transport concurrency aligned with the SDK's default pending-upload
    // limit until ApplySettings supplies the configured value.
    setConnectionLimits(session, DEFAULT_MAX_CONNECTIONS_PER_SERVER);
    m_state = std::make_shared<WinHttpClientState>(session);
}

HttpClient_WinHttp::~HttpClient_WinHttp()
{
    m_state->stopAcceptingRequests();
    CancelAllRequests();
    m_state.reset();
}

IHttpRequest* HttpClient_WinHttp::CreateRequest()
{
    std::string id = "WH-" + toString(::InterlockedIncrement(&s_nextRequestId));
    return new SimpleHttpRequest(id);
}

void HttpClient_WinHttp::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    // SendRequestAsync borrows the request; the caller retains ownership.
    auto state = m_state;
    auto wrapper = std::make_shared<WinHttpRequestWrapper>(
        std::move(state), static_cast<SimpleHttpRequest*>(request));
    wrapper->send(callback);
}

void HttpClient_WinHttp::CancelRequestAsync(std::string const& id)
{
    auto state = m_state;
    // Copy the shared_ptr out of the map while holding the lock only for the
    // lookup, then call cancel() without the lock held (cancel() blocks in
    // WinHttpCloseHandle waiting for a completion callback on another thread
    // that needs this same lock -- see cancel()'s comment). The local copy
    // keeps the wrapper alive for the duration of this call even if erase()
    // concurrently removes the map's own reference.
    std::shared_ptr<WinHttpRequestWrapper> request;
    {
        std::lock_guard<std::mutex> lock(state->requestsMutex);
        auto it = state->requests.find(id);
        if (it != state->requests.end()) {
            request = it->second;
        }
    }
    if (request) {
        request->cancel();
    }
}

void HttpClient_WinHttp::CancelAllRequests()
{
    CancelAllRequests(std::chrono::milliseconds::zero());
}

void HttpClient_WinHttp::CancelAllRequests(std::chrono::milliseconds bestEffortTimeout)
{
    auto state = m_state;
    class CancelAllScope
    {
      public:
        explicit CancelAllScope(std::shared_ptr<WinHttpClientState> state)
          : m_state(std::move(state))
        {
            std::lock_guard<std::mutex> lock(m_state->requestsMutex);
            ++m_state->cancelAllDepth;
        }

        ~CancelAllScope()
        {
            if (m_active)
            {
                std::lock_guard<std::mutex> lock(m_state->requestsMutex);
                --m_state->cancelAllDepth;
            }
        }

        void finishLocked()
        {
            --m_state->cancelAllDepth;
            m_active = false;
        }

      private:
        std::shared_ptr<WinHttpClientState> m_state;
        bool m_active {true};
    } cancelAllScope(state);

    bool const hasTimeout =
        bestEffortTimeout > std::chrono::milliseconds::zero();
    auto const deadline =
        std::chrono::steady_clock::now() + bestEffortTimeout;
    std::thread::id const callerThread = std::this_thread::get_id();
    auto callbacksDrainedForCaller = [&state, callerThread]() {
        // Application callbacks cannot wait for peer callbacks: simultaneous
        // callbacks doing so would wait on one another. Each callback scope
        // retains the shared client state independently.
        return state->callbacksByThread.find(callerThread) !=
                state->callbacksByThread.end() ||
            state->callbacksInFlight == 0;
    };
    auto requestsDrainedForCaller = [&state, callerThread]() {
        if (state->requests.empty())
        {
            return true;
        }

        bool callerIsInStateCallback = false;
        for (auto const& item : state->requests)
        {
            if (item.second->hasStateCallbackOnThreadLocked(callerThread))
            {
                callerIsInStateCallback = true;
                break;
            }
        }
        for (auto const& item : state->requests)
        {
            if (!callerIsInStateCallback ||
                !item.second->hasActiveStateCallbackLocked())
            {
                return false;
            }
        }
        return true;
    };

    for (;;)
    {
        std::vector<std::shared_ptr<WinHttpRequestWrapper>> requests;
        size_t registryGeneration;
        size_t callbackGeneration;
        {
            std::lock_guard<std::mutex> lock(state->requestsMutex);
            if (state->requests.empty() && callbacksDrainedForCaller())
            {
                // Holding the registry lock makes completion of this cancellation
                // epoch the linearization point: later registrations are new work.
                cancelAllScope.finishLocked();
                return;
            }

            registryGeneration = state->registryGeneration;
            callbackGeneration = state->callbackGeneration;
            for (auto const& item : state->requests)
            {
                requests.push_back(item.second);
            }
        }

        for (auto const& request : requests)
        {
            if (hasTimeout && std::chrono::steady_clock::now() >= deadline)
            {
                break;
            }
            request->cancel();
        }

        std::unique_lock<std::mutex> lock(state->requestsMutex);
        if (requestsDrainedForCaller() && callbacksDrainedForCaller())
        {
            cancelAllScope.finishLocked();
            return;
        }
        auto stateChangedOrDrained = [&]() {
            return state->registryGeneration != registryGeneration ||
                state->callbackGeneration != callbackGeneration ||
                (requestsDrainedForCaller() && callbacksDrainedForCaller());
        };
        if (hasTimeout)
        {
            if (!state->requestsCv.wait_until(
                    lock, deadline, stateChangedOrDrained))
            {
                return;
            }
        }
        else
        {
            state->requestsCv.wait(lock, stateChangedOrDrained);
        }
    }
}

/// <summary>
/// Enforces MS-root server certificate check.
/// </summary>
/// <param name="enforceMsRoot">if set to <c>true</c> [enforce verification that server cert is MS-Rooted].</param>
void HttpClient_WinHttp::ApplySettings(ILogConfiguration& config)
{
    int64_t configuredMaxConnections = config[CFG_INT_MAX_PENDING_REQ];
    DWORD maxConnections = DEFAULT_MAX_CONNECTIONS_PER_SERVER;
    if (configuredMaxConnections > 0)
    {
        auto const largestFiniteLimit =
            static_cast<int64_t>(std::numeric_limits<DWORD>::max() - 1);
        maxConnections = static_cast<DWORD>(
            configuredMaxConnections > largestFiniteLimit
                ? largestFiniteLimit
                : configuredMaxConnections);
    }
    setConnectionLimits(m_state->session, maxConnections);
    SetMsRootCheck(config[CFG_MAP_HTTP][CFG_BOOL_HTTP_MS_ROOT_CHECK]);
}

void HttpClient_WinHttp::SetMsRootCheck(bool enforceMsRoot)
{
    m_state->msRootCheck.store(enforceMsRoot, std::memory_order_release);
}

/// <summary>
/// Determines whether MS-Rooted server cert check required.
/// </summary>
/// <returns>
///   <c>true</c> if [MS-Rooted server cert check required]; otherwise, <c>false</c>.
/// </returns>
bool HttpClient_WinHttp::IsMsRootCheckRequired()
{
    return m_state->msRootCheck.load(std::memory_order_acquire);
}

} MAT_NS_END
#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
// clang-format on
