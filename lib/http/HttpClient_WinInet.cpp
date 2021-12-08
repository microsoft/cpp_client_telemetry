// clang-format off
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
#pragma warning(push)
#pragma warning(disable:4189)   /* Turn off Level 4: local variable is initialized but not referenced. dwError unused in Release without printing it. */
#include "HttpClient_WinInet.hpp"
#include "utils/StringUtils.hpp"

#include <Wincrypt.h>
#include <WinInet.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>
#include <oacr.h>

namespace MAT_NS_BEGIN {

class WinInetRequestWrapper
{
  protected:
    HttpClient_WinInet&    m_parent;
    std::string            m_id;
    IHttpResponseCallback* m_appCallback {nullptr};
    HINTERNET              m_hWinInetSession {nullptr};
    HINTERNET              m_hWinInetRequest {nullptr};
    SimpleHttpRequest*     m_request;
    BYTE                   m_buffer[1024] {0};
    DWORD                  m_bufferUsed {0};
    std::vector<uint8_t>   m_bodyBuffer;
    bool                   m_readingData {false};
    bool                   isCallbackCalled {false};
    bool                   isAborted {false};
  public:
    WinInetRequestWrapper(HttpClient_WinInet& parent, SimpleHttpRequest* request)
      : m_parent(parent),
        m_id(request->GetId()),
        m_request(request)
    {
        LOG_TRACE("%p WinInetRequestWrapper()", this);
    }

    WinInetRequestWrapper(WinInetRequestWrapper const&) = delete;
    WinInetRequestWrapper& operator=(WinInetRequestWrapper const&) = delete;

    ~WinInetRequestWrapper() noexcept
    {
        LOG_TRACE("%p ~WinInetRequestWrapper()", this);
        if (m_hWinInetRequest != nullptr)
        {
            ::InternetCloseHandle(m_hWinInetRequest);
            ::InternetCloseHandle(m_hWinInetSession);
        }
    }

    /// <summary>
    /// Asynchronously cancel pending request. This method is not directly calling
    /// the object destructor, but rather hints the implementation to speed-up the
    /// destruction.
    ///
    /// Two possible outcomes:.
    ////
    /// - set isAborted to true: cancel request without sending to WinInet stack,
    ///   in case if request has not been sent to WinInet stack yet.
    ////
    /// - close m_hWinInetRequest handle: WinInet fails all subsequent attempts to
    /// use invalidated handle and aborts all pending WinInet worker threads on it.
    /// In that case we complete with ERROR_INTERNET_OPERATION_CANCELLED.
    ///
    /// It may happen that we get some feedback from WinInet, i.e. we are canceling
    /// at that same moment when the request is complete. In that case we process
    /// completion in the callback (INTERNET_STATUS_REQUEST_COMPLETE).
    /// </summary>
    void cancel()
    {
        LOCKGUARD(m_parent.m_requestsMutex);
        isAborted = true;
        if (m_hWinInetRequest != nullptr)
        {
            ::InternetCloseHandle(m_hWinInetRequest);
            // async request callback destroys the object
        }
    }

    /**
     * Verify that the server end-point certificate is MS-Rooted
     */
    bool isMsRootCert()
    {
        // Pointer to certificate chain obtained via InternetQueryOption :
        // Ref. https://blogs.msdn.microsoft.com/alejacma/2012/01/18/how-to-use-internet_option_server_cert_chain_context-with-internetqueryoption-in-c/
        PCCERT_CHAIN_CONTEXT pCertCtx = nullptr;
        DWORD dwCertChainContextSize = sizeof(PCCERT_CHAIN_CONTEXT);
        // Proceed to process the result if API call succeeds. That option is available in MSIE 8.x+ since Windows 7.1 and Win Server 2008 R2.
        // In case if API call fails, then proceed without cert validation. This behavior is identical to default old behavior to avoid
        // regressions for downlevel OS.
        if (::InternetQueryOption(m_hWinInetRequest, INTERNET_OPTION_SERVER_CERT_CHAIN_CONTEXT, (LPVOID)&pCertCtx, &dwCertChainContextSize))
        {
            CERT_CHAIN_POLICY_STATUS pps = { 0, 0, 0, 0, nullptr };
            pps.cbSize = sizeof(pps);
            // Verify that the cert chain roots up to the Microsoft application root at top level
            CERT_CHAIN_POLICY_PARA policyPara = {0, 0, nullptr };
            policyPara.cbSize = sizeof(policyPara);
            policyPara.dwFlags = MICROSOFT_ROOT_CERT_CHAIN_POLICY_CHECK_APPLICATION_ROOT_FLAG;
            policyPara.pvExtraPolicyPara = nullptr;

            BOOL policyChecked = CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_MICROSOFT_ROOT, pCertCtx, &policyPara, &pps);
            if (pCertCtx != nullptr)
            {
                CertFreeCertificateChain(pCertCtx);
            }
            // Unable to verify the chain
            if (!policyChecked)
            {
                LOG_WARN("CertVerifyCertificateChainPolicy() failed: unable to verify");
                return false;
            }
            // Non-MS rooted cert chain
            if (pps.dwError != ERROR_SUCCESS)
            {
                LOG_WARN("CertVerifyCertificateChainPolicy() failed: invalid root CA - %d", pps.dwError);
                return false;
            }
        }
        else
        {
            // Downlevel OS prior to Win 7 and Win 2008 Server R2 do not support cert chain retrieval
            LOG_TRACE("InternetQueryOption() failed to obtain cert chain");
        }
        return true;
    }

    // Asynchronously send HTTP request and invoke response callback.
    // Ownership semantics: send(...) method self-destroys *this* upon
    // receiving WinInet callback. There must be absolutely no methods
    // that attempt to use the object after triggering send on it.
    // Send operation on request may be issued no more than once.
    //
    // Implementation details:
    //
    // lockguard around m_requestsMutex covers the following stages:
    // - request added to map
    // - URL parsed
    // - DNS lookup performed, socket opened, SSL handshake
    // - MS-Root SSL cert validation (if requested)
    // - populating HTTP request headers
    // - scheduling async(!) upload of HTTP post body
    //
    // Note that if any of the stages above fails, we invoke onRequestComplete(...).
    // That method destroys "this" request object and in order to avoid
    // any corruption we immediately return after invoking onRequestComplete(...).
    //
    void send(IHttpResponseCallback* callback)
    {
        LOCKGUARD(m_parent.m_requestsMutex);
        // Register app callback and request in HttpClient map
        m_appCallback = callback;
        m_parent.m_requests[m_id] = this;

        // If outside code asked us to abort that request before we could proceed with
        // creating a WinInet handle, then clean it right away before proceeding with
        // any async WinInet API calls.
        if (isAborted)
        {
            // Request force-aborted before creating a WinInet handle.
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        DispatchEvent(OnConnecting);
        URL_COMPONENTSA urlc;
        memset(&urlc, 0, sizeof(urlc));
        urlc.dwStructSize = sizeof(urlc);
        char hostname[256] = { 0 };
        urlc.lpszHostName = hostname;
        urlc.dwHostNameLength = sizeof(hostname);
        char path[1024] = { 0 };
        urlc.lpszUrlPath = path;
        urlc.dwUrlPathLength = sizeof(path);
        if (!::InternetCrackUrlA(m_request->m_url.data(), (DWORD)m_request->m_url.size(), 0, &urlc))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("InternetCrackUrl() failed: dwError=%d url=%s", dwError, m_request->m_url.data());
            // Invalid URL passed to WinInet API
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        m_hWinInetSession = ::InternetConnectA(m_parent.m_hInternet, hostname, urlc.nPort,
            NULL, NULL, INTERNET_SERVICE_HTTP, 0, reinterpret_cast<DWORD_PTR>(this));
        if (m_hWinInetSession == NULL) {
            DWORD dwError = ::GetLastError();
            LOG_WARN("InternetConnect() failed: %d", dwError);
            // Cannot connect to host
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }
        // TODO: Session handle for the same target should be cached across requests to enable keep-alive.

        PCSTR szAcceptTypes[] = {"*/*", NULL};
        m_hWinInetRequest = ::HttpOpenRequestA(
            m_hWinInetSession, m_request->m_method.c_str(), path, NULL, NULL, szAcceptTypes,
            INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE |
            INTERNET_FLAG_RELOAD | (urlc.nScheme == INTERNET_SCHEME_HTTPS ? INTERNET_FLAG_SECURE : 0),
            reinterpret_cast<DWORD_PTR>(this));
        if (m_hWinInetRequest == NULL) {
            DWORD dwError = ::GetLastError();
            LOG_WARN("HttpOpenRequest() failed: %d", dwError);
            // Request cannot be opened to given URL because of some connectivity issue
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        /* Perform optional MS Root certificate check for certain end-point URLs */
        if (m_parent.IsMsRootCheckRequired())
        {
            if (!isMsRootCert())
            {
                // Request cannot be completed: end-point certificate is not MS-Rooted
                DispatchEvent(OnConnectFailed);
                onRequestComplete(ERROR_INTERNET_SEC_INVALID_CERT);
                return;
            }
        }

        ::InternetSetStatusCallback(m_hWinInetRequest, &WinInetRequestWrapper::winInetCallback);

        std::ostringstream os;
        for (auto const& header : m_request->m_headers) {
            os << header.first << ": " << header.second << "\r\n";
        }

        if (!::HttpAddRequestHeadersA(m_hWinInetRequest, os.str().data(), static_cast<DWORD>(os.tellp()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE))
        {
            DWORD dwError = ::GetLastError();
            LOG_WARN("HttpAddRequestHeadersA() failed: %d", dwError);
            // Unable to add request headers. There's no point in proceeding with upload because
            // our server is expecting those custom request headers to always be there.
            DispatchEvent(OnConnectFailed);
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }

        // Try to send headers and request body to server
        DispatchEvent(OnSending);
        void *data = static_cast<void *>(m_request->m_body.data());
        DWORD size = static_cast<DWORD>(m_request->m_body.size());
        BOOL bResult = ::HttpSendRequest(m_hWinInetRequest, NULL, 0, data, (DWORD)size);
        DWORD dwError = GetLastError();

        if (bResult == TRUE && dwError != ERROR_IO_PENDING) {
            dwError = ::GetLastError();
            LOG_WARN("HttpSendRequest() failed: %d", dwError);
            // Unable to send requerst
            DispatchEvent(OnSendFailed);
            onRequestComplete(ERROR_INTERNET_OPERATION_CANCELLED);
            return;
        }
        // Async request has been queued in WinInet thread pool
    }

    static void CALLBACK winInetCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
    {
        UNREFERENCED_PARAMETER(dwStatusInformationLength);  // Only used inside an assertion
        UNREFERENCED_PARAMETER(hInternet);                  // Only used in debug printout
        OACR_USE_PTR(hInternet);

        WinInetRequestWrapper* self = reinterpret_cast<WinInetRequestWrapper*>(dwContext);

        LOG_TRACE("winInetCallback: hInternet %p, dwContext %p, dwInternetStatus %u", hInternet, dwContext, dwInternetStatus);
        // Are you looking at logs and need to decode dwInternetStatus values?
        // Go To Definition (F12) on INTERNET_STATUS_REQUEST_COMPLETE below to get to the right place of WinInet.h.

        switch (dwInternetStatus) {
            case INTERNET_STATUS_REQUEST_SENT: {
                assert(hInternet == self->m_hWinInetRequest);
                return;
            }

            case INTERNET_STATUS_HANDLE_CLOSING:
                // HANDLE_CLOSING should always come after REQUEST_COMPLETE. When (and if)
                // it (ever) happens, WinInetRequestWrapper* self pointer may point to object
                // that has been already destroyed. We do not perform any actions on it.
                return;

            case INTERNET_STATUS_REQUEST_COMPLETE: {
                assert(dwStatusInformationLength >= sizeof(INTERNET_ASYNC_RESULT));
                INTERNET_ASYNC_RESULT& result = *static_cast<INTERNET_ASYNC_RESULT*>(lpvStatusInformation);
                assert(hInternet == self->m_hWinInetRequest);
                if ((self != nullptr) && (self->m_hWinInetRequest != nullptr)) {
                    self->onRequestComplete(result.dwError);
                }
                return;
            }

            default:
                return;
        }
    }

    void DispatchEvent(HttpStateEvent type)
    {
        if (m_appCallback != nullptr)
        {
            m_appCallback->OnHttpStateEvent(type, static_cast<void*>(m_hWinInetRequest), 0);
        }
    }

    void onRequestComplete(DWORD dwError)
    {
        if (dwError == ERROR_SUCCESS) {
            // If looking good so far, try to fetch the response body first.
            // It might potentially be another async operation which will
            // trigger INTERNET_STATUS_REQUEST_COMPLETE again.

            m_bodyBuffer.insert(m_bodyBuffer.end(), m_buffer, m_buffer + m_bufferUsed);
            while (!m_readingData || m_bufferUsed != 0) {
                BOOL bResult = ::InternetReadFile(m_hWinInetRequest, m_buffer, sizeof(m_buffer), &m_bufferUsed);
                m_readingData = true;
                if (!bResult) {
                    dwError = GetLastError();
                    if (dwError == ERROR_IO_PENDING) {
                        // Do not touch anything from this thread anymore.
                        // The buffer passed to InternetReadFile() and the
                        // read count will be filled asynchronously, so they
                        // must stay valid and writable until the next
                        // INTERNET_STATUS_REQUEST_COMPLETE callback comes
                        // (that's why those are member variables).
                        LOG_TRACE("InternetReadFile() failed: ERROR_IO_PENDING. Waiting for INTERNET_STATUS_REQUEST_COMPLETE to be called again");
                        return;
                    }
                    LOG_WARN("InternetReadFile() failed: %d", dwError);
                    break;
                }

                m_bodyBuffer.insert(m_bodyBuffer.end(), m_buffer, m_buffer + m_bufferUsed);
            }
        }

        std::unique_ptr<SimpleHttpResponse> response(new SimpleHttpResponse(m_id));

        // SUCCESS with no IO_PENDING means we're done with the response body: try to parse the response headers.
        if (dwError == ERROR_SUCCESS) {
            response->m_body = m_bodyBuffer;
            response->m_result = HttpResult_OK;

            uint32_t value = 0;
            DWORD dwSize = sizeof(value);
            BOOL bResult = ::HttpQueryInfoA(m_hWinInetRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &value, &dwSize, NULL);
            if (!bResult) {
                LOG_WARN("HttpQueryInfo(STATUS_CODE) failed: %d", GetLastError());
            }
            response->m_statusCode = value;

            char* pBuffer = reinterpret_cast<char*>(m_buffer);
            dwSize = sizeof(m_buffer) - 1;
            if (!HttpQueryInfoA(m_hWinInetRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pBuffer, &dwSize, NULL)) {
                dwError = GetLastError();
                if (dwError != ERROR_INSUFFICIENT_BUFFER) {
                    LOG_WARN("HttpQueryInfo(RAW_HEADERS) failed: %d", dwError);
                    dwSize = 0;
                } else {
                    m_bodyBuffer.resize(dwSize + 1);
                    pBuffer = reinterpret_cast<char*>(m_bodyBuffer.data());
                    if (!HttpQueryInfoA(m_hWinInetRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pBuffer, &dwSize, NULL)) {
                        LOG_WARN("HttpQueryInfo(RAW_HEADERS) failed twice: %d", dwError);
                        dwSize = 0;
                    }
                }
            }
            pBuffer[dwSize] = '\0';

            char const* ptr = pBuffer;
            while (*ptr) {
                char const* colon = strchr(ptr, ':');
                if (!colon) {
                    break;
                }
                std::string name(ptr, colon);

                ptr = colon + 1;
                while (*ptr == ' ') {
                    ptr++;
                }

                char const* eol = strstr(ptr, "\r\n");
                if (!eol) {
                    break;
                }
                std::string value1(ptr, eol);

                response->m_headers.add(name, value1);
                ptr = eol + 2;
            }
            // This event handler covers the only positive case when we actually got some server response.
            // We may still invoke OnHttpResponse(...) below for this positive as well as other negative
            // cases where there was a short-read, connection failuire or timeout on reading the response.
            DispatchEvent(OnResponse);

        } else {
            switch (dwError) {
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

        assert(isCallbackCalled == false);
        if (!isCallbackCalled)
        {
            // Only one WinInet worker thread may invoke async callback for a given request at any given moment of time.
            // That ensures that isCallbackCalled does not require a lock around it. We unregister the callback here
            // to ensure that no more callbacks are coming for that m_hWinInetRequest.
            ::InternetSetStatusCallback(m_hWinInetRequest, NULL);
            isCallbackCalled = true;
            m_appCallback->OnHttpResponse(response.release());
            // HttpClient parent is destroying this HttpRequest object by id
            m_parent.erase(m_id);
        }
    }
};

//---

unsigned HttpClient_WinInet::s_nextRequestId = 0;

HttpClient_WinInet::HttpClient_WinInet() :
    m_msRootCheck(false)
{
    m_hInternet = ::InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC);
}

HttpClient_WinInet::~HttpClient_WinInet()
{
    CancelAllRequests();
    ::InternetCloseHandle(m_hInternet);
}

/**
 * This method is called exclusively from onRequestComplete .
 * No other code paths that lead to request destruction.
 */
void HttpClient_WinInet::erase(std::string const& id)
{
    LOCKGUARD(m_requestsMutex);
    auto it = m_requests.find(id);
    if (it != m_requests.end()) {
        auto req = it->second;
        m_requests.erase(it);
        // delete WinInetRequestWrapper
        delete req;
    }
}

IHttpRequest* HttpClient_WinInet::CreateRequest()
{
    std::string id = "WI-" + toString(::InterlockedIncrement(&s_nextRequestId));
    return new SimpleHttpRequest(id);
}

void HttpClient_WinInet::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    // Note: 'request' is never owned by IHttpClient and gets deleted in EventsUploadContext.clear()
    WinInetRequestWrapper *wrapper = new WinInetRequestWrapper(*this, static_cast<SimpleHttpRequest*>(request));
    wrapper->send(callback);
}

void HttpClient_WinInet::CancelRequestAsync(std::string const& id)
{
    LOCKGUARD(m_requestsMutex);
    auto it = m_requests.find(id);
    if (it != m_requests.end()) {
        auto request = it->second;
        if (request) {
            request->cancel();
        }
    }
}


void HttpClient_WinInet::CancelAllRequests()
{
    // vector of all request IDs
    std::vector<std::string> ids;
    {
        LOCKGUARD(m_requestsMutex);
        for (auto const& item : m_requests) {
            ids.push_back(item.first);
        }
    }
    // cancel all requests one-by-one not holding the lock
    for (const auto &id : ids)
        CancelRequestAsync(id);

    // wait for all destructors to run
    while (!m_requests.empty())
    {
        PAL::sleep(100);
        std::this_thread::yield();
    }
};

/// <summary>
/// Enforces MS-root server certificate check.
/// </summary>
/// <param name="enforceMsRoot">if set to <c>true</c> [enforce verification that server cert is MS-Rooted].</param>
void HttpClient_WinInet::SetMsRootCheck(bool enforceMsRoot)
{
    m_msRootCheck = enforceMsRoot;
}

/// <summary>
/// Determines whether MS-Roted server cert check required.
/// </summary>
/// <returns>
///   <c>true</c> if [MS-Rooted server cert check required]; otherwise, <c>false</c>.
/// </returns>
bool HttpClient_WinInet::IsMsRootCheckRequired()
{
    return m_msRootCheck;
}

} MAT_NS_END
#pragma warning(pop)
#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
// clang-format on
