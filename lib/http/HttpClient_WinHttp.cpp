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
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace MAT_NS_BEGIN {

class WinHttpRequestWrapper;

struct WinHttpCallbackContext
{
    explicit WinHttpCallbackContext(std::shared_ptr<WinHttpRequestWrapper> request)
      : request(std::move(request))
    {
    }

    std::weak_ptr<WinHttpRequestWrapper> request;
};

class WinHttpRequestWrapper : public std::enable_shared_from_this<WinHttpRequestWrapper>
{
  protected:
    HttpClient_WinHttp&    m_parent;
    std::string            m_id;
    IHttpResponseCallback* m_appCallback {nullptr};
    HINTERNET              m_hConnect {nullptr};
    HINTERNET              m_hRequest {nullptr};
    SimpleHttpRequest*     m_request;
    std::vector<uint8_t>   m_bodyBuffer;
    std::vector<uint8_t>   m_readBuffer;
    std::atomic<bool>      isCallbackCalled {false};
    bool                   isAborted {false};
    bool                   m_isHttps {false};
    WinHttpCallbackContext* m_callbackContext {nullptr};

  public:
    WinHttpRequestWrapper(HttpClient_WinHttp& parent, SimpleHttpRequest* request)
      : m_parent(parent),
        m_id(request->GetId()),
        m_request(request)
    {
        LOG_TRACE("%p WinHttpRequestWrapper()", this);
    }

    WinHttpRequestWrapper(WinHttpRequestWrapper const&) = delete;
    WinHttpRequestWrapper& operator=(WinHttpRequestWrapper const&) = delete;

    ~WinHttpRequestWrapper() noexcept
    {
        LOG_TRACE("%p ~WinHttpRequestWrapper()", this);
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
    /// m_parent.m_requestsMutex across the call (WinInet's pattern, safe there
    /// because its callback runs synchronously on the calling thread) would
    /// deadlock here: this thread would block inside WinHttpCloseHandle holding
    /// the lock, while the completion callback blocks on the same thread's
    /// erase() needing that same lock. So the handle is captured and closed
    /// without holding the lock. This wrapper is only reachable through a
    /// shared_ptr (see HttpClient_WinHttp::m_requests / CancelRequestAsync), so
    /// releasing the lock here cannot race with the object being freed --
    /// the caller already holds its own shared_ptr keeping *this* alive.
    /// </summary>
    void cancel()
    {
        HINTERNET hRequestToClose = nullptr;
        {
            std::lock_guard<std::recursive_mutex> lock(m_parent.m_requestsMutex);
            if (isCallbackCalled)
            {
                return;
            }
            isAborted = true;
            hRequestToClose = m_hRequest;
            m_hRequest = nullptr;
        }
        if (hRequestToClose != nullptr)
        {
            ::WinHttpCloseHandle(hRequestToClose);
            // WinHttpCloseHandle waits for any callback to finish. Some
            // cancellation paths report only HANDLE_CLOSING, so complete the
            // request here if no callback delivered the terminal result.
            if (!isCallbackCalled)
            {
                m_hRequest = nullptr;
                onRequestComplete(ERROR_WINHTTP_OPERATION_CANCELLED);
            }
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
        std::lock_guard<std::recursive_mutex> lock(m_parent.m_requestsMutex);
        return m_hRequest;
    }

    void DispatchEvent(HttpStateEvent type)
    {
        if (m_appCallback != nullptr)
        {
            m_appCallback->OnHttpStateEvent(type, static_cast<void*>(m_hRequest), 0);
        }
    }

    // Asynchronously send HTTP request and invoke response callback.
    // Ownership semantics: send(...) method self-destroys *this* upon
    // reaching the terminal WinHTTP callback. There must be absolutely no
    // methods that attempt to use the object after triggering send on it.
    // Send operation on request may be issued no more than once.
    //
    // Handle setup runs under m_parent.m_requestsMutex (a recursive_mutex,
    // matching HttpClient_WinInet's model), exactly like cancel(): that
    // serializes send() and cancel() completely, so cancel() can never
    // interleave mid-way through handle creation and be silently lost.
    //
    // DEADLOCK NOTE: the lock must NOT still be held when a synchronous
    // failure completes the request. onRequestComplete() invokes the
    // application callback, which is documented (below) to be able to tear the
    // client down synchronously -- that reaches CancelAllRequests(), which
    // waits on m_requestsCv. condition_variable_any::wait() releases only ONE
    // level of a recursive_mutex, so waiting with the mutex held twice leaves
    // it locked: erase() on the WinHTTP callback thread can then never acquire
    // it to notify, and the wait never wakes. So sendLocked() only reports the
    // failure, and send() completes it after the lock is released.
    void send(IHttpResponseCallback* callback)
    {
        bool failed = false;
        DWORD dwError = ERROR_SUCCESS;
        {
            std::lock_guard<std::recursive_mutex> lock(m_parent.m_requestsMutex);
            failed = !sendLocked(callback, dwError);
        }
        if (failed)
        {
            onRequestComplete(dwError);
        }
    }

    // Returns true if the request was handed off to WinHTTP asynchronously.
    // Returns false on synchronous failure, setting dwError to the result the
    // caller must complete the request with (once the lock has been dropped).
    bool sendLocked(IHttpResponseCallback* callback, DWORD& dwErrorOut)
    {
        m_appCallback = callback;
        m_parent.m_requests[m_id] = shared_from_this();

        if (isAborted)
        {
            // Request force-aborted before creating a WinHTTP handle.
            DispatchEvent(OnConnectFailed);
            dwErrorOut = ERROR_WINHTTP_OPERATION_CANCELLED;
            return false;
        }

        DispatchEvent(OnConnecting);

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
            DispatchEvent(OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        if (m_parent.m_hSession == nullptr)
        {
            LOG_WARN("WinHttpOpen() did not produce a usable session handle");
            DispatchEvent(OnConnectFailed);
            dwErrorOut = ERROR_WINHTTP_CANNOT_CONNECT;
            return false;
        }

        // TODO: connect handle for the same target should be cached across
        // requests to enable keep-alive (same pre-existing opportunity noted
        // in HttpClient_WinInet.cpp; out of scope for this transport swap).
        m_hConnect = ::WinHttpConnect(m_parent.m_hSession, hostname, urlc.nPort, 0);
        if (m_hConnect == nullptr)
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpConnect() failed: %d", dwError);
            // Cannot connect to host
            DispatchEvent(OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        std::wstring wMethod = to_utf16_string(m_request->m_method);
        m_isHttps = (urlc.nScheme == INTERNET_SCHEME_HTTPS);
        m_hRequest = ::WinHttpOpenRequest(
            m_hConnect, wMethod.c_str(), path, NULL, WINHTTP_NO_REFERER,
            WINHTTP_DEFAULT_ACCEPT_TYPES,
            WINHTTP_FLAG_REFRESH | (m_isHttps ? WINHTTP_FLAG_SECURE : 0));
        if (m_hRequest == nullptr)
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpOpenRequest() failed: %d", dwError);
            // Request cannot be opened to given URL because of some connectivity issue
            DispatchEvent(OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        // Match the WinInet transport's INTERNET_FLAG_NO_AUTH behavior.
        // Telemetry requests must not answer server or proxy authentication
        // challenges with ambient process credentials.
        DWORD disableFeatures = WINHTTP_DISABLE_AUTHENTICATION;
        if (!::WinHttpSetOption(
                m_hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &disableFeatures, sizeof(disableFeatures)))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpSetOption(DISABLE_AUTHENTICATION) failed: %d", dwError);
            DispatchEvent(OnConnectFailed);
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
                WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_HANDLES, 0) == WINHTTP_INVALID_STATUS_CALLBACK)
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("WinHttpSetStatusCallback() failed: %d", dwError);
            DispatchEvent(OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        std::ostringstream os;
        for (auto const& header : m_request->m_headers) {
            os << header.first << ": " << header.second << "\r\n";
        }
        std::wstring wHeaders = to_utf16_string(os.str());

        if (!wHeaders.empty() &&
            wHeaders.size() > static_cast<size_t>(std::numeric_limits<DWORD>::max()))
        {
            LOG_WARN("Request headers exceed WinHTTP's maximum size");
            DispatchEvent(OnConnectFailed);
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
            DispatchEvent(OnConnectFailed);
            dwErrorOut = dwError;
            return false;
        }

        // Try to send headers and request body to server
        DispatchEvent(OnSending);
        if (m_request->m_body.size() > static_cast<size_t>(std::numeric_limits<DWORD>::max()))
        {
            LOG_WARN("Request body exceeds WinHTTP's maximum size");
            DispatchEvent(OnSendFailed);
            dwErrorOut = ERROR_INVALID_PARAMETER;
            return false;
        }
        void* data = m_request->m_body.empty() ? nullptr : static_cast<void*>(m_request->m_body.data());
        DWORD size = static_cast<DWORD>(m_request->m_body.size());
        m_callbackContext = new WinHttpCallbackContext(shared_from_this());
        DWORD_PTR context = reinterpret_cast<DWORD_PTR>(m_callbackContext);
        BOOL bResult = ::WinHttpSendRequest(
            m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, data, size, size, context);
        if (!bResult)
        {
            DWORD dwError = ::GetLastError();
            // WinHTTP retains the context on the request handle and can deliver
            // HANDLE_CLOSING after this failure. Keep it alive until that callback.
            LOG_WARN("WinHttpSendRequest() failed: %d", dwError);
            // Unable to send request
            DispatchEvent(OnSendFailed);
            dwErrorOut = dwError;
            return false;
        }
        // Async request has been queued; completion arrives via winHttpCallback.
        return true;
    }

    // Drives the WinHTTP async state machine: SendRequest -> ReceiveResponse ->
    // (QueryDataAvailable -> ReadData)* -> onRequestComplete. Unlike WinInet
    // (whose async completions all report through the single
    // INTERNET_STATUS_REQUEST_COMPLETE code, and whose synchronous API calls
    // signal a pending async op via a FALSE return + GetLastError()==
    // ERROR_IO_PENDING), WinHTTP has one distinct callback status per stage,
    // and a FALSE return from any of these calls on an async handle is always a
    // genuine synchronous failure -- never "pending" -- so every failure path
    // here reports immediately instead of waiting for a further callback.
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
            // The callback context outlives the request wrapper and is released
            // only by WinHTTP's final notification.
            delete context;
            return;
        }

        std::shared_ptr<WinHttpRequestWrapper> self = context->request.lock();
        if (self == nullptr)
        {
            return;
        }

        LOG_TRACE("winHttpCallback: hInternet %p, self %p, dwInternetStatus %u", hInternet, self.get(), dwInternetStatus);

        switch (dwInternetStatus)
        {
            case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
            {
                HINTERNET request = self->getRequestHandle();
                if (request == nullptr)
                {
                    self->onRequestComplete(ERROR_WINHTTP_OPERATION_CANCELLED);
                }
                else if (!::WinHttpReceiveResponse(request, NULL))
                {
                    self->onRequestComplete(::GetLastError());
                }
                return;
            }

            case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
            {
                HINTERNET request = self->getRequestHandle();
                if (request == nullptr)
                {
                    self->onRequestComplete(ERROR_WINHTTP_OPERATION_CANCELLED);
                    return;
                }
                // TLS negotiation and response-header receipt are both complete here,
                // so WINHTTP_OPTION_SERVER_CERT_CONTEXT is available for the
                // configured Microsoft-root enforcement.
                if (self->m_isHttps && self->m_parent.IsMsRootCheckRequired() && !self->isMsRootCert(request))
                {
                    self->onRequestComplete(ERROR_WINHTTP_SECURE_INVALID_CERT);
                    return;
                }
                if (!::WinHttpQueryDataAvailable(request, NULL))
                {
                    self->onRequestComplete(::GetLastError());
                }
                return;
            }

            case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
            {
                DWORD bytesAvailable = (lpvStatusInformation != nullptr)
                    ? *static_cast<DWORD*>(lpvStatusInformation) : 0;
                if (bytesAvailable == 0)
                {
                    // No more data: response is complete.
                    self->onRequestComplete(ERROR_SUCCESS);
                    return;
                }
                // SECURITY: refuse an over-large response instead of buffering it
                // (see MAX_HTTP_RESPONSE_SIZE) so a hostile/MITM'd collector cannot
                // exhaust process memory. Checked before every read so the buffer
                // never exceeds the cap; reported as an invalid server response ->
                // NetworkFailure (retried).
                if (self->m_bodyBuffer.size() + bytesAvailable > MAX_HTTP_RESPONSE_SIZE)
                {
                    LOG_WARN("HTTP response exceeds max buffered size (%zu bytes); aborting", MAX_HTTP_RESPONSE_SIZE);
                    self->onRequestComplete(ERROR_WINHTTP_INVALID_SERVER_RESPONSE);
                    return;
                }
                HINTERNET request = self->getRequestHandle();
                if (request == nullptr)
                {
                    self->onRequestComplete(ERROR_WINHTTP_OPERATION_CANCELLED);
                    return;
                }
                self->m_readBuffer.resize(bytesAvailable);
                if (!::WinHttpReadData(request, self->m_readBuffer.data(), bytesAvailable, NULL))
                {
                    self->onRequestComplete(::GetLastError());
                }
                return;
            }

            case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
                // dwStatusInformationLength is the number of bytes actually placed
                // into the buffer passed to WinHttpReadData (may be less than the
                // bytesAvailable that was requested).
                if (dwStatusInformationLength > self->m_readBuffer.size())
                {
                    self->onRequestComplete(ERROR_WINHTTP_INVALID_SERVER_RESPONSE);
                    return;
                }
                self->m_bodyBuffer.insert(self->m_bodyBuffer.end(),
                    self->m_readBuffer.begin(), self->m_readBuffer.begin() + dwStatusInformationLength);
                {
                    HINTERNET request = self->getRequestHandle();
                    if (request == nullptr)
                    {
                        self->onRequestComplete(ERROR_WINHTTP_OPERATION_CANCELLED);
                        return;
                    }
                    if (!::WinHttpQueryDataAvailable(request, NULL))
                    {
                        self->onRequestComplete(::GetLastError());
                    }
                }
                return;

            case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
            {
                WINHTTP_ASYNC_RESULT* result = static_cast<WINHTTP_ASYNC_RESULT*>(lpvStatusInformation);
                DWORD dwError = (result != nullptr) ? result->dwError : ERROR_WINHTTP_INTERNAL_ERROR;
                self->onRequestComplete(dwError);
                return;
            }

            default:
                return;
        }
    }

    void onRequestComplete(DWORD dwError)
    {
        if (isCallbackCalled.exchange(true))
        {
            return;
        }

        std::unique_ptr<SimpleHttpResponse> response(new SimpleHttpResponse(m_id));
        HINTERNET request = getRequestHandle();
        if (dwError == ERROR_SUCCESS && request == nullptr)
        {
            dwError = ERROR_WINHTTP_OPERATION_CANCELLED;
        }

        if (dwError == ERROR_SUCCESS) {
            response->m_body = m_bodyBuffer;
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
            ::WinHttpQueryHeaders(request, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &headerBytes, WINHTTP_NO_HEADER_INDEX);
            DWORD headerErr = ::GetLastError();
            if (headerBytes > 0 && headerErr == ERROR_INSUFFICIENT_BUFFER)
            {
                std::wstring wHeaders(headerBytes / sizeof(wchar_t), L'\0');
                if (::WinHttpQueryHeaders(request, WINHTTP_QUERY_RAW_HEADERS_CRLF,
                        WINHTTP_HEADER_NAME_BY_INDEX, &wHeaders[0], &headerBytes, WINHTTP_NO_HEADER_INDEX))
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
            else
            {
                LOG_WARN("WinHttpQueryHeaders(RAW_HEADERS_CRLF) failed: %d", headerErr);
            }
            // This event handler covers the only positive case when we actually got some server response.
            // We may still invoke OnHttpResponse(...) below for this positive as well as other negative
            // cases where there was a short-read, connection failure or timeout on reading the response.
            DispatchEvent(OnResponse);

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
            auto callback = m_appCallback;
            auto requestId = m_id;
            auto keepAlive = shared_from_this();
            // Remove the request before entering application code. The callback
            // can synchronously tear down the client and destroy this wrapper.
            m_parent.erase(requestId);
            callback->OnHttpResponse(response.release());
            keepAlive.reset();
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

unsigned HttpClient_WinHttp::s_nextRequestId = 0;

HttpClient_WinHttp::HttpClient_WinHttp() :
    m_msRootCheck(false)
{
    // WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY (Windows 8.1+) resolves the proxy
    // without depending on a logged-on interactive user or that user's
    // Internet Explorer settings -- unlike WinInet's
    // INTERNET_OPEN_TYPE_PRECONFIG, which requires one. This is why WinHTTP,
    // not WinInet, is Microsoft's documented recommendation for services and
    // other non-interactive processes. On an older OS that rejects this access
    // type, fall back to the machine-wide WinHTTP proxy configuration. This is
    // the documented pre-Windows-8.1 behavior and avoids bypassing enterprise
    // proxies entirely.
    m_hSession = ::WinHttpOpen(
        NULL, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
    if (m_hSession == nullptr)
    {
        LOG_WARN("WinHttpOpen(AUTOMATIC_PROXY) failed: %d; retrying with default proxy", ::GetLastError());
        m_hSession = ::WinHttpOpen(
            NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
    }
}

HttpClient_WinHttp::~HttpClient_WinHttp()
{
    CancelAllRequests();
    if (m_hSession != nullptr)
    {
        ::WinHttpCloseHandle(m_hSession);
    }
}

/**
 * This method is called exclusively from onRequestComplete.
 * No other code paths that lead to request destruction.
 */
void HttpClient_WinHttp::erase(std::string const& id)
{
    // Drop the map's shared_ptr reference under the lock. If a concurrent
    // cancel() call (see its comment) is holding its own shared_ptr copy, the
    // wrapper's actual destruction is deferred until that copy also goes out
    // of scope -- never while any caller still holds a live reference.
    {
        std::lock_guard<std::recursive_mutex> lock(m_requestsMutex);
        m_requests.erase(id);
    }
    m_requestsCv.notify_all();
}

IHttpRequest* HttpClient_WinHttp::CreateRequest()
{
    std::string id = "WH-" + toString(::InterlockedIncrement(&s_nextRequestId));
    return new SimpleHttpRequest(id);
}

void HttpClient_WinHttp::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    // Note: 'request' is never owned by IHttpClient and gets deleted in EventsUploadContext.clear()
    auto wrapper = std::make_shared<WinHttpRequestWrapper>(*this, static_cast<SimpleHttpRequest*>(request));
    wrapper->send(callback);
}

void HttpClient_WinHttp::CancelRequestAsync(std::string const& id)
{
    // Copy the shared_ptr out of the map while holding the lock only for the
    // lookup, then call cancel() without the lock held (cancel() blocks in
    // WinHttpCloseHandle waiting for a completion callback on another thread
    // that needs this same lock -- see cancel()'s comment). The local copy
    // keeps the wrapper alive for the duration of this call even if erase()
    // concurrently removes the map's own reference.
    std::shared_ptr<WinHttpRequestWrapper> request;
    {
        std::lock_guard<std::recursive_mutex> lock(m_requestsMutex);
        auto it = m_requests.find(id);
        if (it != m_requests.end()) {
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
    if (bestEffortTimeout > std::chrono::milliseconds::zero())
    {
        std::vector<std::string> ids;
        {
            std::lock_guard<std::recursive_mutex> lock(m_requestsMutex);
            for (auto const& item : m_requests) {
                ids.push_back(item.first);
            }
        }
        // Cancel all requests one-by-one without holding the lock.
        for (const auto& id : ids)
            CancelRequestAsync(id);

        std::unique_lock<std::recursive_mutex> lock(m_requestsMutex);
        m_requestsCv.wait_for(lock, bestEffortTimeout, [this]() noexcept -> bool {
            return m_requests.empty();
        });
    }
    else
    {
        // A request can be inserted after the initial cancellation snapshot
        // while the producer side is still shutting down. Repeatedly take a
        // snapshot and cancel until the map is empty; waiting only on the
        // original snapshot can leave a late request uncancelled forever.
        for (;;)
        {
            std::vector<std::string> ids;
            {
                std::lock_guard<std::recursive_mutex> lock(m_requestsMutex);
                if (m_requests.empty())
                {
                    return;
                }
                for (auto const& item : m_requests) {
                    ids.push_back(item.first);
                }
            }

            for (const auto& id : ids)
                CancelRequestAsync(id);

            std::unique_lock<std::recursive_mutex> lock(m_requestsMutex);
            m_requestsCv.wait_for(lock, std::chrono::milliseconds(100), [this]() noexcept -> bool {
                return m_requests.empty();
            });
        }
    }
}

/// <summary>
/// Enforces MS-root server certificate check.
/// </summary>
/// <param name="enforceMsRoot">if set to <c>true</c> [enforce verification that server cert is MS-Rooted].</param>
void HttpClient_WinHttp::ApplySettings(ILogConfiguration& config)
{
    SetMsRootCheck(config[CFG_MAP_HTTP][CFG_BOOL_HTTP_MS_ROOT_CHECK]);
}

void HttpClient_WinHttp::SetMsRootCheck(bool enforceMsRoot)
{
    m_msRootCheck.store(enforceMsRoot, std::memory_order_release);
}

/// <summary>
/// Determines whether MS-Rooted server cert check required.
/// </summary>
/// <returns>
///   <c>true</c> if [MS-Rooted server cert check required]; otherwise, <c>false</c>.
/// </returns>
bool HttpClient_WinHttp::IsMsRootCheckRequired()
{
    return m_msRootCheck.load(std::memory_order_acquire);
}

} MAT_NS_END
#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
// clang-format on
