// clang-format off
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
#include "HttpClient_WinInet.hpp"
#include "detail/MsRootCertPolicy.hpp"
#include "utils/StringUtils.hpp"

#include <Wincrypt.h>
#include <WinInet.h>

#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
#include <oacr.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "wininet.lib")

namespace MAT_NS_BEGIN {

namespace {

std::atomic<bool> g_terminalFinalizationFaultForTests {false};
std::atomic<bool> g_terminalFinalizationFaultInjectedForTests {false};

} // namespace

class WinInetRequestWrapper;

struct WinInetCallbackContext
{
    explicit WinInetCallbackContext(std::shared_ptr<WinInetRequestWrapper> request)
        : request(std::move(request))
    {
    }

    std::shared_ptr<WinInetRequestWrapper> request;
};

struct WinInetClientState
{
    explicit WinInetClientState(HINTERNET internetHandle);
    ~WinInetClientState();

    bool registerRequest(
        std::string const& id,
        std::shared_ptr<WinInetRequestWrapper> request);
    void eraseRequest(std::string const& id);
    void stopAcceptingRequests();
    void beginCallback();
    void endCallback();

    HINTERNET internet;
    std::mutex requestsMutex;
    std::map<std::string, std::shared_ptr<WinInetRequestWrapper>> requests;
    std::condition_variable requestsCv;
    std::atomic<bool> msRootCheck {false};
    bool acceptingRequests {true};
    size_t cancelAllDepth {0};
    size_t registryGeneration {0};
    size_t callbackGeneration {0};
    size_t callbacksInFlight {0};
    std::map<std::thread::id, size_t> callbacksByThread;
};

class WinInetCallbackScope
{
  public:
    explicit WinInetCallbackScope(
        std::shared_ptr<WinInetClientState> state)
        : m_state(std::move(state))
    {
        m_state->beginCallback();
    }

    ~WinInetCallbackScope()
    {
        m_state->endCallback();
    }

    WinInetCallbackScope(WinInetCallbackScope const&) = delete;
    WinInetCallbackScope& operator=(WinInetCallbackScope const&) = delete;

  private:
    std::shared_ptr<WinInetClientState> m_state;
};

class WinInetRequestWrapper : public std::enable_shared_from_this<WinInetRequestWrapper>
{
  protected:
    enum class PendingApi
    {
        None,
        StagedHeaders,
        StagedBody,
        StagedEnd
    };

    std::shared_ptr<WinInetClientState> m_clientState;
    std::string            m_id;
    IHttpResponseCallback* m_appCallback {nullptr};
    // WinInet may deliver completion callbacks synchronously from an async API.
    // This per-request recursive mutex permits only that narrow re-entry. It is
    // never nested with the parent request-map mutex; cancellation snapshots
    // the registry before touching request handles or invoking application code.
    std::recursive_mutex    m_handleMutex;
    HINTERNET              m_hWinInetSession {nullptr};
    HINTERNET              m_hWinInetRequest {nullptr};
    WinInetCallbackContext* m_callbackContext {nullptr};
    SimpleHttpRequest*     m_request;
    BYTE                   m_buffer[1024] {0};
    DWORD                  m_bufferUsed {0};
    std::vector<uint8_t>   m_bodyBuffer;
    bool                   m_readingData {false};
    std::atomic<bool>       m_terminalCallbackStarted {false};
    std::atomic<bool>       m_isAborted {false};
    std::atomic<DWORD>      m_deferredError {ERROR_SUCCESS};
    bool                   m_msRootCheckRequired {false};
    // HTTPS is latched before the request handle exists so the staged send can
    // distinguish requests that require certificate-policy enforcement.
    bool                   m_isHttps {false};
    // The MS-root check runs at most once per request handle, on the first
    // staged-send completion after the TLS handshake completes.
    std::atomic<bool>       m_msRootChecked {false};
    bool                   m_contextInstalled {false};
    bool                   m_sendIssued {false};
    bool                   m_setupActive {false};
    unsigned               m_stateCallbackDepth {0};
    std::map<std::thread::id, size_t> m_stateCallbacksByThread;
    bool                   m_setupCompletionPending {false};
    DWORD                  m_setupCompletionError {ERROR_SUCCESS};
    unsigned               m_asyncApiDepth {0};
    bool                   m_apiCompletionPending {false};
    DWORD                  m_apiCompletionError {ERROR_SUCCESS};
    PendingApi             m_pendingApi {PendingApi::None};
    size_t                 m_stagedBodyOffset {0};
    DWORD                  m_stagedBytesWritten {0};

    class SetupGuard
    {
      public:
        explicit SetupGuard(WinInetRequestWrapper& owner) noexcept
            : m_owner(owner)
        {
            std::lock_guard<std::recursive_mutex> lock(m_owner.m_handleMutex);
            m_owner.m_setupActive = true;
        }

        ~SetupGuard() noexcept
        {
#if HAVE_EXCEPTIONS
            try
            {
#endif
            m_owner.finishSetup();
#if HAVE_EXCEPTIONS
            }
            catch (...)
            {
                LOG_ERROR("Unhandled exception while completing WinInet request setup");
            }
#endif
        }

        SetupGuard(SetupGuard const&) = delete;
        SetupGuard& operator=(SetupGuard const&) = delete;

      private:
        WinInetRequestWrapper& m_owner;
    };

    class StateCallbackGuard
    {
      public:
        StateCallbackGuard(
            WinInetRequestWrapper& owner,
            std::thread::id callbackThread) noexcept
            : m_owner(owner),
              m_callbackThread(callbackThread)
        {
        }

        ~StateCallbackGuard() noexcept
        {
#if HAVE_EXCEPTIONS
            try
            {
#endif
            m_owner.finishStateCallback(m_callbackThread);
#if HAVE_EXCEPTIONS
            }
            catch (...)
            {
                LOG_ERROR("Unhandled exception while completing a WinInet state callback");
            }
#endif
        }

        StateCallbackGuard(StateCallbackGuard const&) = delete;
        StateCallbackGuard& operator=(StateCallbackGuard const&) = delete;

      private:
        WinInetRequestWrapper& m_owner;
        std::thread::id m_callbackThread;
    };

    class TerminalFinalizationGuard
    {
       public:
        TerminalFinalizationGuard(
            WinInetRequestWrapper& owner,
            std::shared_ptr<WinInetClientState> state,
            std::string const& requestId) noexcept
            : m_owner(owner),
              m_state(std::move(state)),
              m_requestId(requestId)
        {
        }

        ~TerminalFinalizationGuard() noexcept
        {
            Finalize();
        }

        void Finalize() noexcept
        {
            if (!m_active)
            {
                return;
            }
            m_active = false;
#if HAVE_EXCEPTIONS
            try
            {
#endif
                m_owner.closeRequestHandle();
#if HAVE_EXCEPTIONS
            }
            catch (...)
            {
                LOG_ERROR("Unhandled exception while closing a completed WinInet request");
            }
            try
            {
#endif
                m_owner.closeSessionHandle();
#if HAVE_EXCEPTIONS
            }
            catch (...)
            {
                LOG_ERROR("Unhandled exception while closing a completed WinInet session");
            }
            try
            {
#endif
                m_state->eraseRequest(m_requestId);
#if HAVE_EXCEPTIONS
            }
            catch (...)
            {
                LOG_ERROR("Unhandled exception while removing a completed WinInet request");
            }
#endif
        }

        TerminalFinalizationGuard(TerminalFinalizationGuard const&) = delete;
        TerminalFinalizationGuard& operator=(TerminalFinalizationGuard const&) = delete;

       private:
        WinInetRequestWrapper& m_owner;
        std::shared_ptr<WinInetClientState> m_state;
        std::string const& m_requestId;
        bool m_active{true};
    };

    void finishSetup()
    {
        bool complete = false;
        DWORD completionError = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            m_setupActive = false;
            complete = m_setupCompletionPending;
            completionError = m_setupCompletionError;
            m_setupCompletionPending = false;
            m_setupCompletionError = ERROR_SUCCESS;
        }
        if (complete)
        {
            onRequestComplete(completionError);
        }
    }

    void finishStateCallback(std::thread::id callbackThread)
    {
        bool complete = false;
        DWORD completionError = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            assert(m_stateCallbackDepth != 0);
            --m_stateCallbackDepth;
            auto it = m_stateCallbacksByThread.find(callbackThread);
            assert(it != m_stateCallbacksByThread.end());
            if (it != m_stateCallbacksByThread.end() && --it->second == 0)
            {
                m_stateCallbacksByThread.erase(it);
            }
            if (m_stateCallbackDepth == 0 && !m_setupActive &&
                m_setupCompletionPending)
            {
                complete = true;
                completionError = m_setupCompletionError;
                m_setupCompletionPending = false;
                m_setupCompletionError = ERROR_SUCCESS;
            }
        }
        if (complete)
        {
            onRequestComplete(completionError);
        }
    }

    HINTERNET detachRequestHandle()
    {
        std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
        HINTERNET request = m_hWinInetRequest;
        m_hWinInetRequest = nullptr;
        return request;
    }

    HINTERNET detachSessionHandle()
    {
        std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
        HINTERNET session = m_hWinInetSession;
        m_hWinInetSession = nullptr;
        return session;
    }

    void closeRequestHandle()
    {
        HINTERNET request = detachRequestHandle();
        if (request != nullptr)
        {
            // InternetCloseHandle may synchronously deliver HANDLE_CLOSING.
            // Never hold either mutex while closing.
            ::InternetCloseHandle(request);
        }
    }

    void closeSessionHandle()
    {
        HINTERNET session = detachSessionHandle();
        if (session != nullptr)
        {
            ::InternetCloseHandle(session);
        }
    }

    bool shouldStopSetup() const noexcept
    {
        return m_isAborted.load(std::memory_order_acquire) ||
            m_terminalCallbackStarted.load(std::memory_order_acquire);
    }

    void handleWinInetCompletion(DWORD dwError)
    {
        PendingApi completedApi = PendingApi::None;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_asyncApiDepth != 0)
            {
                m_apiCompletionPending = true;
                m_apiCompletionError = dwError;
                return;
            }
            completedApi = m_pendingApi;
            m_pendingApi = PendingApi::None;
        }

        if (completedApi == PendingApi::None)
        {
            onRequestComplete(dwError);
            return;
        }
        continueStagedSend(completedApi, dwError);
    }

    void completeIssuedApi(BOOL result, DWORD error)
    {
        bool completionPending = false;
        DWORD completionError = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            completionPending = m_apiCompletionPending;
            completionError = m_apiCompletionError;
            m_apiCompletionPending = false;
            m_apiCompletionError = ERROR_SUCCESS;
        }

        if (completionPending)
        {
            handleWinInetCompletion(completionError);
        }
        else if (result)
        {
            handleWinInetCompletion(ERROR_SUCCESS);
        }
        else if (error != ERROR_IO_PENDING)
        {
            handleWinInetCompletion(error);
        }
    }

    void issueStagedEnd()
    {
        BOOL result = FALSE;
        DWORD error = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_hWinInetRequest == nullptr || shouldStopSetup())
            {
                error = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else
            {
                m_pendingApi = PendingApi::StagedEnd;
                ++m_asyncApiDepth;
                result = ::HttpEndRequestA(m_hWinInetRequest, nullptr, 0, 0);
                error = result ? ERROR_SUCCESS : ::GetLastError();
                --m_asyncApiDepth;
            }
        }
        if (error == ERROR_INTERNET_OPERATION_CANCELLED)
        {
            onRequestComplete(error);
            return;
        }
        completeIssuedApi(result, error);
    }

    void issueStagedBody()
    {
        size_t const bodySize = m_request->m_body.size();
        if (m_stagedBodyOffset == bodySize)
        {
            issueStagedEnd();
            return;
        }

        BOOL result = FALSE;
        DWORD error = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_hWinInetRequest == nullptr || shouldStopSetup())
            {
                error = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else
            {
                size_t const remaining = bodySize - m_stagedBodyOffset;
                m_stagedBytesWritten = 0;
                m_pendingApi = PendingApi::StagedBody;
                ++m_asyncApiDepth;
                result = ::InternetWriteFile(
                    m_hWinInetRequest,
                    m_request->m_body.data() + m_stagedBodyOffset,
                    static_cast<DWORD>(remaining),
                    &m_stagedBytesWritten);
                error = result ? ERROR_SUCCESS : ::GetLastError();
                --m_asyncApiDepth;
            }
        }
        if (error == ERROR_INTERNET_OPERATION_CANCELLED)
        {
            onRequestComplete(error);
            return;
        }
        completeIssuedApi(result, error);
    }

    void continueStagedSend(PendingApi completedApi, DWORD dwError)
    {
        if (dwError != ERROR_SUCCESS)
        {
            DispatchEvent(OnSendFailed);
            onRequestComplete(dwError);
            return;
        }

        switch (completedApi)
        {
            case PendingApi::StagedHeaders:
                runMsRootCheckOnce();
                dwError = m_deferredError.load(std::memory_order_acquire);
                if (dwError != ERROR_SUCCESS)
                {
                    onRequestComplete(dwError);
                    return;
                }
                issueStagedBody();
                return;

            case PendingApi::StagedBody:
                if (m_stagedBytesWritten == 0 ||
                    m_stagedBytesWritten >
                        m_request->m_body.size() - m_stagedBodyOffset)
                {
                    LOG_ERROR("InternetWriteFile() returned an invalid byte count");
                    DispatchEvent(OnSendFailed);
                    onRequestComplete(ERROR_INTERNET_INTERNAL_ERROR);
                    return;
                }
                m_stagedBodyOffset += m_stagedBytesWritten;
                issueStagedBody();
                return;

            case PendingApi::StagedEnd:
                onRequestComplete(ERROR_SUCCESS);
                return;

            case PendingApi::None:
                onRequestComplete(ERROR_INTERNET_INTERNAL_ERROR);
                return;
        }
    }

    void issueStagedHeaders()
    {
        INTERNET_BUFFERSA buffers {};
        buffers.dwStructSize = sizeof(buffers);
        buffers.dwBufferTotal = static_cast<DWORD>(m_request->m_body.size());

        BOOL result = FALSE;
        DWORD error = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_hWinInetRequest == nullptr || shouldStopSetup())
            {
                error = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else
            {
                m_sendIssued = true;
                m_pendingApi = PendingApi::StagedHeaders;
                ++m_asyncApiDepth;
                result = ::HttpSendRequestExA(
                    m_hWinInetRequest, &buffers, nullptr, 0,
                    reinterpret_cast<DWORD_PTR>(m_callbackContext));
                error = result ? ERROR_SUCCESS : ::GetLastError();
                --m_asyncApiDepth;
            }
        }
        if (error == ERROR_INTERNET_OPERATION_CANCELLED)
        {
            onRequestComplete(error);
            return;
        }
        completeIssuedApi(result, error);
    }

  public:
    WinInetRequestWrapper(
        std::shared_ptr<WinInetClientState> clientState,
        SimpleHttpRequest* request)
      : m_clientState(std::move(clientState)),
        m_id(request->GetId()),
        m_request(request)
    {
        LOG_TRACE("%p WinInetRequestWrapper()", this);
    }

    WinInetRequestWrapper(WinInetRequestWrapper const&) = delete;
    WinInetRequestWrapper& operator=(WinInetRequestWrapper const&) = delete;

    bool hasStateCallbackOnThread(std::thread::id threadId)
    {
        std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
        return m_stateCallbacksByThread.find(threadId) !=
            m_stateCallbacksByThread.end();
    }

    bool hasActiveStateCallback()
    {
        std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
        return m_stateCallbackDepth != 0;
    }

    ~WinInetRequestWrapper() noexcept
    {
        LOG_TRACE("%p ~WinInetRequestWrapper()", this);
        closeRequestHandle();
        closeSessionHandle();
    }

    /// <summary>
    /// Asynchronously cancel pending request. This method is not directly calling
    /// the object destructor, but rather hints the implementation to speed-up the
    /// destruction.
    ///
    /// Cancellation marks setup as aborted and closes an existing request handle.
    /// Before the asynchronous send starts, completion can be delivered directly.
    /// After it starts, completion is deferred until REQUEST_COMPLETE or
    /// HANDLE_CLOSING proves that WinInet has released the caller's body buffer.
    ///
    /// It may happen that we get some feedback from WinInet, i.e. we are canceling
    /// at that same moment when the request is complete. In that case we process
    /// completion in the callback (INTERNET_STATUS_REQUEST_COMPLETE).
    /// </summary>
    void cancel()
    {
        HINTERNET request = nullptr;
        bool completeHere = false;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_terminalCallbackStarted.load(std::memory_order_acquire))
            {
                return;
            }
            m_isAborted.store(true, std::memory_order_release);
            DWORD noError = ERROR_SUCCESS;
            m_deferredError.compare_exchange_strong(
                noError, ERROR_INTERNET_OPERATION_CANCELLED, std::memory_order_acq_rel);
            request = m_hWinInetRequest;
            m_hWinInetRequest = nullptr;
            // Before an async send is issued, WinInet owns none of the request
            // body's storage and no REQUEST_COMPLETE callback is guaranteed.
            completeHere =
                m_stateCallbackDepth == 0 &&
                !m_setupActive &&
                (!m_contextInstalled || !m_sendIssued);
        }
        if (request != nullptr)
        {
            // WinInet may invoke callbacks here. The callback context retains
            // this wrapper until HANDLE_CLOSING.
            ::InternetCloseHandle(request);
        }
        if (completeHere)
        {
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
        }
    }

    /**
     * Gather the server certificate chain facts for the current request handle
     * and reduce them to a pure policy decision. This is the only place that
     * touches WinInet/Wincrypt; the Allow/Reject/Unable logic lives in the
     * platform-independent detail::EvaluateMsRootPolicy helper so it can be
     * reasoned about and unit-tested without a live connection.
     *
     * Called after HttpSendRequestEx completes and before InternetWriteFile, so
     * cancellation and terminal completion cannot close the request handle
     * during the query, policy evaluation, or chain release.
     */
    detail::MsRootPolicyDecision evaluateServerCertificatePolicyLocked()
    {
        detail::MsRootCertQuery query;
        query.httpsScheme = m_isHttps;

        if (m_hWinInetRequest == nullptr)
        {
            // Cancellation or terminal completion won before evaluation began.
            return detail::EvaluateMsRootPolicy(query);
        }

        // Pointer to certificate chain obtained via InternetQueryOption :
        // Ref. https://blogs.msdn.microsoft.com/alejacma/2012/01/18/how-to-use-internet_option_server_cert_chain_context-with-internetqueryoption-in-c/
        PCCERT_CHAIN_CONTEXT pCertCtx = nullptr;
        DWORD dwCertChainContextSize = sizeof(PCCERT_CHAIN_CONTEXT);
        // That option is available in MSIE 8.x+ since Windows 7.1 and Win Server
        // 2008 R2. If the chain cannot be obtained, the optional policy fails closed.
        if (::InternetQueryOption(m_hWinInetRequest, INTERNET_OPTION_SERVER_CERT_CHAIN_CONTEXT, (LPVOID)&pCertCtx, &dwCertChainContextSize))
        {
            query.chainQuerySucceeded = true;
            query.chainContextPresent = (pCertCtx != nullptr);
            if (pCertCtx != nullptr)
            {
                CERT_CHAIN_POLICY_STATUS pps = { sizeof(pps), 0, 0, 0, nullptr };
                // Verify that the cert chain roots up to the Microsoft application root at top level
                CERT_CHAIN_POLICY_PARA policyPara = { sizeof(policyPara), 0, nullptr };
                policyPara.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;
                policyPara.pvExtraPolicyPara = nullptr;

                BOOL policyChecked = CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_MICROSOFT_ROOT, pCertCtx, &policyPara, &pps);
                query.policyCheckPerformed = (policyChecked == TRUE);
                query.policyStatusError = static_cast<std::uint32_t>(pps.dwError);
                CertFreeCertificateChain(pCertCtx);
            }
            else
            {
                LOG_TRACE("InternetQueryOption() returned no server cert chain");
            }
        }
        else
        {
            // Downlevel OS prior to Win 7 and Win 2008 Server R2 do not support cert chain retrieval
            LOG_TRACE("InternetQueryOption() failed to obtain cert chain");
        }

        return detail::EvaluateMsRootPolicy(query);
    }

    /**
     * Run the MS-root certificate policy exactly once per request handle after
     * the staged headers establish TLS but before any request body is written.
     * Rejection and inability to evaluate both fail closed.
     */
    void runMsRootCheckOnce()
    {
        if (!m_msRootCheckRequired || !m_isHttps)
        {
            return;
        }
        if (m_msRootChecked.exchange(true, std::memory_order_acq_rel))
        {
            return;  // atomic latch: at most once per handle
        }

        HINTERNET requestToClose = nullptr;
        detail::MsRootPolicyDecision decision;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_hWinInetRequest == nullptr)
            {
                return;
            }
            decision = evaluateServerCertificatePolicyLocked();
            if (!detail::ShouldProceed(decision))
            {
                // We still own a live handle under this lock, so this evaluated
                // rejection takes precedence over a cancellation that has not yet
                // acquired the lock. A prior cancellation removes the handle and
                // therefore evaluates as Unable above.
                m_deferredError.store(
                    ERROR_INTERNET_SEC_INVALID_CERT, std::memory_order_release);
                m_isAborted.store(true, std::memory_order_release);
                requestToClose = m_hWinInetRequest;
                m_hWinInetRequest = nullptr;
            }
        }

        switch (decision)
        {
            case detail::MsRootPolicyDecision::Allow:
                return;

            case detail::MsRootPolicyDecision::Unable:
                LOG_ERROR("MS-root certificate policy could not be evaluated; aborting request");
                break;

            case detail::MsRootPolicyDecision::Reject:
                LOG_ERROR("Server certificate chain is not MS-rooted; aborting request");
                break;
        }
        if (requestToClose != nullptr)
        {
            // InternetCloseHandle may synchronously deliver HANDLE_CLOSING.
            // The callback holds its own shared_ptr before this call.
            ::InternetCloseHandle(requestToClose);
        }
    }

    // Asynchronously send HTTP request and invoke response callback.
    // The request map owns the wrapper during setup, and the callback context
    // retains it after a WinInet request handle is created. Send may be issued
    // only once.
    void send(IHttpResponseCallback* callback)
    {
        SetupGuard setupGuard(*this);
        m_appCallback = callback;
        m_msRootCheckRequired =
            m_clientState->msRootCheck.load(std::memory_order_acquire);
        if (!m_clientState->registerRequest(m_id, shared_from_this()))
        {
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

#if HAVE_EXCEPTIONS
        try
        {
#endif
        if (shouldStopSetup())
        {
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        DispatchEvent(OnConnecting);
        if (shouldStopSetup())
        {
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        if (m_request->m_url.size() > static_cast<size_t>(std::numeric_limits<DWORD>::max()))
        {
            LOG_WARN("Request URL exceeds WinInet's maximum size");
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INVALID_PARAMETER);
            return;
        }

        URL_COMPONENTSA urlc;
        memset(&urlc, 0, sizeof(urlc));
        urlc.dwStructSize = sizeof(urlc);
        char hostname[256] = { 0 };
        urlc.lpszHostName = hostname;
        urlc.dwHostNameLength = sizeof(hostname);
        char path[1024] = { 0 };
        urlc.lpszUrlPath = path;
        urlc.dwUrlPathLength = sizeof(path);
        if (!::InternetCrackUrlA(
                m_request->m_url.c_str(), static_cast<DWORD>(m_request->m_url.size()), 0, &urlc))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("InternetCrackUrl() failed: dwError=%d url=%s", dwError, m_request->m_url.c_str());
            DispatchEvent(OnConnectFailed);
            onRequestComplete(dwError);
            return;
        }

        // Latch the scheme before the request handle exists: the SENDING_REQUEST
        // callback uses this to apply the MS-root policy to HTTPS only.
        m_isHttps = (urlc.nScheme == INTERNET_SCHEME_HTTPS);
        if (m_msRootCheckRequired && m_isHttps)
        {
            LOG_ERROR("MS-root certificate policy requires WinHTTP; rejecting WinInet request before sending headers");
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INTERNET_SEC_INVALID_CERT);
            return;
        }

        DWORD dwError = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (shouldStopSetup())
            {
                dwError = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else
            {
                m_hWinInetSession = ::InternetConnectA(
                    m_clientState->internet, hostname, urlc.nPort,
                    NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
                if (m_hWinInetSession == nullptr)
                {
                    dwError = ::GetLastError();
                }
            }
        }
        if (dwError != ERROR_SUCCESS)
        {
            LOG_WARN("InternetConnect() failed: %d", dwError);
            DispatchEvent(OnConnectFailed);
            onRequestComplete(dwError);
            return;
        }
        // TODO: Session handle for the same target should be cached across requests to enable keep-alive.

        PCSTR szAcceptTypes[] = {"*/*", NULL};
        {
            std::unique_ptr<WinInetCallbackContext> context(
                new WinInetCallbackContext(shared_from_this()));
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (shouldStopSetup())
            {
                dwError = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else
            {
                m_hWinInetRequest = ::HttpOpenRequestA(
                    m_hWinInetSession, m_request->m_method.c_str(), path, NULL, NULL, szAcceptTypes,
                    INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_CACHE_WRITE |
                    INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE |
                    INTERNET_FLAG_RELOAD |
                        (m_msRootCheckRequired ? INTERNET_FLAG_NO_AUTO_REDIRECT : 0) |
                        (urlc.nScheme == INTERNET_SCHEME_HTTPS ? INTERNET_FLAG_SECURE : 0),
                    reinterpret_cast<DWORD_PTR>(context.get()));
                if (m_hWinInetRequest == nullptr)
                {
                    dwError = ::GetLastError();
                }
                else if (::InternetSetStatusCallback(
                             m_hWinInetRequest, &WinInetRequestWrapper::winInetCallback) ==
                         INTERNET_INVALID_STATUS_CALLBACK)
                {
                    dwError = ::GetLastError();
                }
                else
                {
                    m_callbackContext = context.get();
                    context.release();
                    m_contextInstalled = true;
                }
            }
        }
        if (dwError != ERROR_SUCCESS)
        {
            LOG_WARN("HttpOpenRequest() failed: %d", dwError);
            DispatchEvent(OnConnectFailed);
            onRequestComplete(dwError);
            return;
        }
        if (shouldStopSetup())
        {
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        // HTTPS requests with the optional policy use a staged send below:
        // establish TLS and send headers, validate the negotiated chain, then
        // write the body only after the policy allows the connection.

        std::ostringstream os;
        for (auto const& header : m_request->m_headers) {
            os << header.first << ": " << header.second << "\r\n";
        }
        std::string headers = os.str();

        if (headers.size() > static_cast<size_t>(std::numeric_limits<DWORD>::max()))
        {
            LOG_WARN("Request headers exceed WinInet's maximum size");
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INVALID_PARAMETER);
            return;
        }

        if (!headers.empty())
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_hWinInetRequest == nullptr || shouldStopSetup())
            {
                dwError = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else if (!::HttpAddRequestHeadersA(
                         m_hWinInetRequest, headers.c_str(), static_cast<DWORD>(headers.size()),
                         HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
            {
                dwError = ::GetLastError();
            }
        }
        if (dwError != ERROR_SUCCESS)
        {
            LOG_WARN("HttpAddRequestHeadersA() failed: %d", dwError);
            DispatchEvent(OnConnectFailed);
            onRequestComplete(dwError);
            return;
        }
        if (shouldStopSetup())
        {
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        DispatchEvent(OnSending);
        if (shouldStopSetup())
        {
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }
        if (m_request->m_body.size() > static_cast<size_t>(std::numeric_limits<DWORD>::max()))
        {
            LOG_WARN("Request body exceeds WinInet's maximum size");
            DispatchEvent(OnSendFailed);
            onRequestComplete(ERROR_INVALID_PARAMETER);
            return;
        }

        if (m_msRootCheckRequired && m_isHttps)
        {
            issueStagedHeaders();
            return;
        }

        BOOL sendResult = FALSE;
        bool completionPending = false;
        DWORD completionError = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_hWinInetRequest == nullptr || shouldStopSetup())
            {
                dwError = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else
            {
                void* data = m_request->m_body.empty()
                    ? nullptr
                    : static_cast<void*>(m_request->m_body.data());
                m_sendIssued = true;
                ++m_asyncApiDepth;
                sendResult = ::HttpSendRequestA(
                    m_hWinInetRequest, nullptr, 0, data,
                    static_cast<DWORD>(m_request->m_body.size()));
                dwError = sendResult ? ERROR_SUCCESS : ::GetLastError();
                --m_asyncApiDepth;
                completionPending = m_apiCompletionPending;
                completionError = m_apiCompletionError;
                m_apiCompletionPending = false;
                m_apiCompletionError = ERROR_SUCCESS;
            }
        }

        if (completionPending)
        {
            onRequestComplete(completionError);
            return;
        }
        if (sendResult)
        {
            // WinInet is permitted to finish an asynchronous-session request
            // synchronously. A TRUE return is success, not an error.
            onRequestComplete(ERROR_SUCCESS);
            return;
        }
        if (dwError != ERROR_IO_PENDING)
        {
            LOG_WARN("HttpSendRequest() failed: %d", dwError);
            DispatchEvent(OnSendFailed);
            onRequestComplete(dwError);
            return;
        }
#if HAVE_EXCEPTIONS
        }
        catch (...)
        {
            // Registration transferred callback responsibility to this wrapper.
            // Convert setup callback failures into terminal completion.
            cancel();
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
        }
#endif
    }

    static void CALLBACK winInetCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
    {
        OACR_USE_PTR(hInternet);

        WinInetCallbackContext* context = reinterpret_cast<WinInetCallbackContext*>(dwContext);
        if (context == nullptr)
        {
            return;
        }

        LOG_TRACE("winInetCallback: hInternet %p, dwContext %p, dwInternetStatus %u", hInternet, dwContext, dwInternetStatus);
        // Are you looking at logs and need to decode dwInternetStatus values?
        // Go To Definition (F12) on INTERNET_STATUS_REQUEST_COMPLETE below to get to the right place of WinInet.h.

        switch (dwInternetStatus) {
            case INTERNET_STATUS_SENDING_REQUEST:
                return;

            case INTERNET_STATUS_REQUEST_SENT:
                return;

            case INTERNET_STATUS_HANDLE_CLOSING: {
                // The request handle owns the callback context after callback
                // registration. HANDLE_CLOSING is its final notification.
                std::unique_ptr<WinInetCallbackContext> contextOwner(context);
                auto self = contextOwner->request;
                {
                    std::lock_guard<std::recursive_mutex> lock(self->m_handleMutex);
                    self->m_callbackContext = nullptr;
                }
                DWORD deferredError = self->m_deferredError.load(std::memory_order_acquire);
                if (deferredError != ERROR_SUCCESS &&
                    !self->m_terminalCallbackStarted.load(std::memory_order_acquire))
                {
                    self->onRequestComplete(deferredError);
                }
                return;
            }

            case INTERNET_STATUS_REQUEST_COMPLETE: {
                auto self = context->request;
                if (lpvStatusInformation == nullptr ||
                    dwStatusInformationLength < sizeof(INTERNET_ASYNC_RESULT))
                {
                    LOG_WARN("WinInet REQUEST_COMPLETE callback returned invalid status data");
                    self->onRequestComplete(ERROR_INTERNET_INTERNAL_ERROR);
                    return;
                }
                INTERNET_ASYNC_RESULT const& result =
                    *static_cast<INTERNET_ASYNC_RESULT const*>(lpvStatusInformation);
                self->handleWinInetCompletion(result.dwError);
                return;
            }

            default:
                return;
        }
    }

    void DispatchEvent(HttpStateEvent type)
    {
        IHttpResponseCallback* callback = nullptr;
        HINTERNET request = nullptr;
        std::thread::id const callbackThread = std::this_thread::get_id();
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_appCallback == nullptr ||
                m_terminalCallbackStarted.load(std::memory_order_acquire))
            {
                return;
            }
            callback = m_appCallback;
            request = m_hWinInetRequest;
            ++m_stateCallbackDepth;
            ++m_stateCallbacksByThread[callbackThread];
        }
        StateCallbackGuard callbackGuard(*this, callbackThread);
        callback->OnHttpStateEvent(type, static_cast<void*>(request), 0);
    }

    void onRequestComplete(DWORD dwError)
    {
        auto keepAlive = shared_from_this();
        auto state = m_clientState;
        auto callback = m_appCallback;
        auto requestId = m_id;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            if (m_stateCallbackDepth != 0 || m_setupActive)
            {
                m_setupCompletionPending = true;
                m_setupCompletionError = dwError;
                return;
            }
            if (m_asyncApiDepth != 0)
            {
                // WinInet can invoke REQUEST_COMPLETE before an asynchronous
                // API returns. Let the issuing frame consume that completion
                // after it has restored its local state.
                m_apiCompletionPending = true;
                m_apiCompletionError = dwError;
                return;
            }
            if (m_terminalCallbackStarted.load(std::memory_order_acquire))
            {
                return;
            }
            DWORD deferredError = m_deferredError.load(std::memory_order_acquire);
            if (deferredError != ERROR_SUCCESS)
            {
                dwError = deferredError;
            }
        }

        if (dwError == ERROR_SUCCESS)
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            DWORD deferredError = m_deferredError.load(std::memory_order_acquire);
            if (deferredError != ERROR_SUCCESS)
            {
                dwError = deferredError;
            }
            else if (m_hWinInetRequest == nullptr)
            {
                dwError = ERROR_INTERNET_OPERATION_CANCELLED;
            }
            else
            {
                auto appendReadBuffer = [this]() -> bool
                {
                    if (m_bodyBuffer.size() > MAX_HTTP_RESPONSE_SIZE ||
                        m_bufferUsed > MAX_HTTP_RESPONSE_SIZE - m_bodyBuffer.size())
                    {
                        return false;
                    }
                    m_bodyBuffer.insert(m_bodyBuffer.end(), m_buffer, m_buffer + m_bufferUsed);
                    return true;
                };

                bool shouldRead = !m_readingData || m_bufferUsed != 0;
                if (m_readingData && !appendReadBuffer())
                {
                    dwError = ERROR_HTTP_INVALID_SERVER_RESPONSE;
                }

                while (dwError == ERROR_SUCCESS && shouldRead)
                {
                    ++m_asyncApiDepth;
                    BOOL readResult = ::InternetReadFile(
                        m_hWinInetRequest, m_buffer, sizeof(m_buffer), &m_bufferUsed);
                    DWORD readError = readResult ? ERROR_SUCCESS : ::GetLastError();
                    --m_asyncApiDepth;
                    m_readingData = true;

                    bool completionPending = m_apiCompletionPending;
                    DWORD completionError = m_apiCompletionError;
                    m_apiCompletionPending = false;
                    m_apiCompletionError = ERROR_SUCCESS;

                    if (completionPending)
                    {
                        if (completionError != ERROR_SUCCESS)
                        {
                            dwError = completionError;
                            break;
                        }
                    }
                    else if (!readResult)
                    {
                        if (readError == ERROR_IO_PENDING)
                        {
                            LOG_TRACE("InternetReadFile() is pending; waiting for REQUEST_COMPLETE");
                            return;
                        }
                        dwError = readError;
                        break;
                    }

                    if (!appendReadBuffer())
                    {
                        dwError = ERROR_HTTP_INVALID_SERVER_RESPONSE;
                        break;
                    }
                    shouldRead = m_bufferUsed != 0;
                }
            }
        }

        if (dwError == ERROR_HTTP_INVALID_SERVER_RESPONSE)
        {
            LOG_WARN("HTTP response exceeds max buffered size (%zu bytes); aborting", MAX_HTTP_RESPONSE_SIZE);
        }
        else if (dwError != ERROR_SUCCESS &&
                 dwError != ERROR_INTERNET_OPERATION_CANCELLED)
        {
            LOG_WARN("WinInet request failed: %d", dwError);
        }

        HINTERNET request = nullptr;
        {
            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
            DWORD deferredError = m_deferredError.load(std::memory_order_acquire);
            if (deferredError != ERROR_SUCCESS)
            {
                dwError = deferredError;
            }
            if (m_terminalCallbackStarted.exchange(true, std::memory_order_acq_rel))
            {
                return;
            }
            request = m_hWinInetRequest;
            if (dwError == ERROR_SUCCESS && request == nullptr)
            {
                dwError = ERROR_INTERNET_OPERATION_CANCELLED;
            }
        }

        TerminalFinalizationGuard terminalFinalization(*this, state, requestId);
#if HAVE_EXCEPTIONS
        try
        {
            if (g_terminalFinalizationFaultForTests.exchange(false))
            {
                g_terminalFinalizationFaultInjectedForTests.store(true);
                throw std::runtime_error("injected WinInet terminal finalization failure");
            }
#endif
            std::unique_ptr<SimpleHttpResponse> response(new SimpleHttpResponse(m_id));
            if (dwError == ERROR_SUCCESS)
            {
                response->m_body = std::move(m_bodyBuffer);

                uint32_t statusCode = 0;
                DWORD statusBytes = sizeof(statusCode);
                {
                    std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
                    if (!::HttpQueryInfoA(
                            request, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                            &statusCode, &statusBytes, nullptr))
                    {
                        dwError = ::GetLastError();
                        LOG_WARN("HttpQueryInfo(STATUS_CODE) failed: %d", dwError);
                    }
                }
                response->m_statusCode = statusCode;

                if (dwError == ERROR_SUCCESS)
                {
                    response->m_result = HttpResult_OK;

                    DWORD headerBytes = 0;
                    BOOL headersQueried = FALSE;
                    DWORD headerError = ERROR_SUCCESS;
                    {
                        std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
                        headersQueried = ::HttpQueryInfoA(
                            request, HTTP_QUERY_RAW_HEADERS_CRLF, nullptr,
                            &headerBytes, nullptr);
                        headerError = headersQueried ? ERROR_SUCCESS : ::GetLastError();
                    }
                    if (!headersQueried &&
                        headerError == ERROR_INSUFFICIENT_BUFFER &&
                        headerBytes > 0 &&
                        headerBytes < std::numeric_limits<DWORD>::max())
                    {
                        std::vector<char> headers(static_cast<size_t>(headerBytes) + 1, '\0');
                        DWORD bufferBytes = headerBytes;
                        {
                            std::lock_guard<std::recursive_mutex> lock(m_handleMutex);
                            headersQueried = ::HttpQueryInfoA(
                                request, HTTP_QUERY_RAW_HEADERS_CRLF, headers.data(),
                                &bufferBytes, nullptr);
                            headerError = headersQueried ? ERROR_SUCCESS : ::GetLastError();
                        }
                        if (headersQueried)
                        {
                            headers.back() = '\0';
                            parseHeaders(std::string(headers.data()), *response);
                        }
                        else
                        {
                            LOG_WARN("HttpQueryInfo(RAW_HEADERS) failed twice: %d", headerError);
                        }
                    }
                    else if (!headersQueried && headerError != ERROR_SUCCESS)
                    {
                        LOG_WARN("HttpQueryInfo(RAW_HEADERS) failed: %d", headerError);
                    }
                }
            }

            if (dwError != ERROR_SUCCESS)
            {
                switch (dwError)
                {
                case ERROR_INTERNET_OPERATION_CANCELLED:
                    response->m_result = HttpResult_Aborted;
                    break;

                case ERROR_INTERNET_TIMEOUT:
                case ERROR_INTERNET_EXTENDED_ERROR:
                case ERROR_INTERNET_NAME_NOT_RESOLVED:
                case ERROR_INTERNET_ITEM_NOT_FOUND:
                case ERROR_INTERNET_CANNOT_CONNECT:
                case ERROR_INTERNET_CONNECTION_ABORTED:
                case ERROR_INTERNET_CONNECTION_RESET:
                case ERROR_INTERNET_SEC_CERT_DATE_INVALID:
                case ERROR_INTERNET_SEC_CERT_CN_INVALID:
                case ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR:
                case ERROR_INTERNET_HTTPS_TO_HTTP_ON_REDIR:
                case ERROR_INTERNET_CHG_POST_IS_NON_SECURE:
                case ERROR_INTERNET_POST_IS_NON_SECURE:
                case ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED:
                case ERROR_INTERNET_INVALID_CA:
                case ERROR_INTERNET_HTTPS_HTTP_SUBMIT_REDIR:
                case ERROR_INTERNET_SEC_CERT_ERRORS:
                case ERROR_HTTP_DOWNLEVEL_SERVER:
                case ERROR_HTTP_INVALID_SERVER_RESPONSE:
                case ERROR_HTTP_REDIRECT_FAILED:
                case ERROR_HTTP_NOT_REDIRECTED:
                case ERROR_INTERNET_SEC_INVALID_CERT:
                case ERROR_INTERNET_SEC_CERT_REVOKED:
                case ERROR_INTERNET_DECODING_FAILED:
                    response->m_result = HttpResult_NetworkFailure;
                    break;

                default:
                    response->m_result = HttpResult_LocalFailure;
                    break;
                }
            }

            WinInetCallbackScope callbackScope(state);
            // Closing first guarantees WinInet no longer owns the caller's request
            // body before OnHttpResponse allows that request to be destroyed.
            terminalFinalization.Finalize();

            if (callback != nullptr)
            {
                if (dwError == ERROR_SUCCESS)
                {
                    // The implementation-specific handle is no longer valid once
                    // terminal delivery begins, so do not expose a stale handle.
#if HAVE_EXCEPTIONS
                    try
                    {
#endif
                        callback->OnHttpStateEvent(OnResponse, nullptr, 0);
#if HAVE_EXCEPTIONS
                    }
                    catch (...)
                    {
                        LOG_ERROR("Unhandled exception in WinInet OnResponse state callback");
                    }
#endif
                }
                callback->OnHttpResponse(response.release());
            }
#if HAVE_EXCEPTIONS
        }
        catch (...)
        {
            LOG_ERROR("Unhandled exception while finalizing a WinInet request");
        }
#endif
    }

    static void parseHeaders(std::string const& raw, SimpleHttpResponse& response)
    {
        size_t lineStart = 0;
        while (lineStart < raw.size())
        {
            size_t lineEnd = raw.find("\r\n", lineStart);
            if (lineEnd == std::string::npos)
            {
                lineEnd = raw.size();
            }

            std::string const line = raw.substr(lineStart, lineEnd - lineStart);
            size_t const colon = line.find(':');
            if (colon != std::string::npos)
            {
                size_t valueStart = colon + 1;
                while (valueStart < line.size() && line[valueStart] == ' ')
                {
                    ++valueStart;
                }
                response.m_headers.add(
                    line.substr(0, colon), line.substr(valueStart));
            }

            if (lineEnd == raw.size())
            {
                break;
            }
            lineStart = lineEnd + 2;
        }
    }
};

//---

WinInetClientState::WinInetClientState(HINTERNET internetHandle) :
    internet(internetHandle)
{
}

WinInetClientState::~WinInetClientState()
{
    if (internet != nullptr)
    {
        ::InternetCloseHandle(internet);
    }
}

bool WinInetClientState::registerRequest(
    std::string const& id,
    std::shared_ptr<WinInetRequestWrapper> request)
{
    bool shouldSend;
    {
        std::lock_guard<std::mutex> lock(requestsMutex);
        if (!acceptingRequests)
        {
            return false;
        }
        requests[id] = std::move(request);
        ++registryGeneration;
        shouldSend = cancelAllDepth == 0;
    }
    requestsCv.notify_all();
    return shouldSend;
}

void WinInetClientState::eraseRequest(std::string const& id)
{
    {
        std::lock_guard<std::mutex> lock(requestsMutex);
        requests.erase(id);
        ++registryGeneration;
    }
    requestsCv.notify_all();
}

void WinInetClientState::stopAcceptingRequests()
{
    std::lock_guard<std::mutex> lock(requestsMutex);
    acceptingRequests = false;
}

void WinInetClientState::beginCallback()
{
    {
        std::lock_guard<std::mutex> lock(requestsMutex);
        ++callbacksInFlight;
        ++callbacksByThread[std::this_thread::get_id()];
        ++callbackGeneration;
    }
    requestsCv.notify_all();
}

void WinInetClientState::endCallback()
{
    {
        std::lock_guard<std::mutex> lock(requestsMutex);
        if (callbacksInFlight == 0)
        {
            LOG_ERROR("WinInet callback accounting underflow");
            requestsCv.notify_all();
            return;
        }

        --callbacksInFlight;
        auto it = callbacksByThread.find(std::this_thread::get_id());
        if (it == callbacksByThread.end() || it->second == 0)
        {
            LOG_ERROR("WinInet callback thread was not registered");
        }
        else if (--it->second == 0)
        {
            callbacksByThread.erase(it);
        }
        ++callbackGeneration;
    }
    requestsCv.notify_all();
}

unsigned HttpClient_WinInet::s_nextRequestId = 0;

HttpClient_WinInet::HttpClient_WinInet()
{
    auto internet = ::InternetOpen(
        NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
    m_state = std::make_shared<WinInetClientState>(internet);
}

HttpClient_WinInet::~HttpClient_WinInet()
{
    m_state->stopAcceptingRequests();
    CancelAllRequests();
}

IHttpRequest* HttpClient_WinInet::CreateRequest()
{
    std::string id = "WI-" + toString(::InterlockedIncrement(&s_nextRequestId));
    return new SimpleHttpRequest(id);
}

void HttpClient_WinInet::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    // SendRequestAsync borrows the request; the caller retains ownership.
    auto wrapper = std::make_shared<WinInetRequestWrapper>(
        m_state, static_cast<SimpleHttpRequest*>(request));
    wrapper->send(callback);
}

void HttpClient_WinInet::CancelRequestAsync(std::string const& id)
{
    std::shared_ptr<WinInetRequestWrapper> request;
    {
        std::lock_guard<std::mutex> lock(m_state->requestsMutex);
        auto it = m_state->requests.find(id);
        if (it != m_state->requests.end()) {
            request = it->second;
        }
    }
    if (request) {
        request->cancel();
    }
}


void HttpClient_WinInet::CancelAllRequests()
{
    CancelAllRequests(std::chrono::milliseconds::zero());
}

void HttpClient_WinInet::CancelAllRequests(std::chrono::milliseconds bestEffortTimeout)
{
    auto state = m_state;
    class CancelAllScope
    {
      public:
        explicit CancelAllScope(std::shared_ptr<WinInetClientState> state)
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
        std::shared_ptr<WinInetClientState> m_state;
        bool m_active {true};
    } cancelAllScope(state);

    bool const hasTimeout =
        bestEffortTimeout > std::chrono::milliseconds::zero();
    auto const deadline =
        std::chrono::steady_clock::now() + bestEffortTimeout;
    std::thread::id const callerThread = std::this_thread::get_id();
    auto requestsDrainedForCaller = [&state, callerThread]() {
        if (state->requests.empty())
        {
            return true;
        }

        bool callerIsInStateCallback = false;
        for (auto const& item : state->requests)
        {
            if (item.second->hasStateCallbackOnThread(callerThread))
            {
                callerIsInStateCallback = true;
                break;
            }
        }
        for (auto const& item : state->requests)
        {
            if (!callerIsInStateCallback ||
                !item.second->hasActiveStateCallback())
            {
                return false;
            }
        }
        return true;
    };
    auto callbacksDrainedForCaller = [&state, callerThread]() {
        // A terminal callback cannot wait for peer callbacks: two callbacks
        // doing so concurrently would wait on each other. Each callback scope
        // retains the shared client state independently.
        return state->callbacksByThread.find(callerThread) !=
                state->callbacksByThread.end() ||
            state->callbacksInFlight == 0;
    };

    for (;;)
    {
        std::vector<std::shared_ptr<WinInetRequestWrapper>> requests;
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
void HttpClient_WinInet::ApplySettings(ILogConfiguration& config)
{
    SetMsRootCheck(config[CFG_MAP_HTTP][CFG_BOOL_HTTP_MS_ROOT_CHECK]);
}

void HttpClient_WinInet::SetMsRootCheck(bool enforceMsRoot)
{
    m_state->msRootCheck.store(enforceMsRoot, std::memory_order_release);
}

/// <summary>
/// Determines whether an MS-Rooted server certificate check is required.
/// </summary>
/// <returns>
///   <c>true</c> if [MS-Rooted server cert check required]; otherwise, <c>false</c>.
/// </returns>
bool HttpClient_WinInet::IsMsRootCheckRequired()
{
    return m_state->msRootCheck.load(std::memory_order_acquire);
}

void HttpClient_WinInet::SetTerminalFinalizationFaultForTests(bool enabled)
{
    g_terminalFinalizationFaultInjectedForTests.store(false);
    g_terminalFinalizationFaultForTests.store(enabled);
}

bool HttpClient_WinInet::WasTerminalFinalizationFaultInjectedForTests()
{
    return g_terminalFinalizationFaultInjectedForTests.load();
}

} MAT_NS_END
#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
// clang-format on
