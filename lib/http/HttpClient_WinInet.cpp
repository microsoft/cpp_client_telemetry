// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "HttpClient_WinInet.hpp"
#include "utils/Utils.hpp"

#include <WinInet.h>

#include <algorithm>
#include <memory>
#include <sstream>
#include <vector>
#include <oacr.h>

namespace ARIASDK_NS_BEGIN {

class WinInetRequestWrapper
{
  protected:
    HttpClient_WinInet&    m_parent;
    std::string            m_id;
    IHttpResponseCallback* m_appCallback {};
    HINTERNET              m_hWinInetSession {};
    HINTERNET              m_hWinInetRequest {};
    SimpleHttpRequest*     m_request;
    BYTE                   m_buffer[1024] {};

  public:
    WinInetRequestWrapper(HttpClient_WinInet& parent, SimpleHttpRequest* request)
      : m_parent(parent),
        m_request(request),
        m_id(request->GetId())
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

    /**
     * Async cancel pending request 
     */
    void cancel()
    {
        if (m_hWinInetRequest != nullptr)
        {
            ::InternetCloseHandle(m_hWinInetRequest);
            // don't wait for request callback
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
            CERT_CHAIN_POLICY_STATUS pps = {0};
            pps.cbSize = sizeof(pps);
            // Verify that the cert chain roots up to the Microsoft application root at top level
            CERT_CHAIN_POLICY_PARA policyPara = {0};
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

    void send(IHttpResponseCallback* callback)
    {
        m_appCallback = callback;

        {
            std::lock_guard<std::mutex> lock(m_parent.m_requestsMutex);
            m_parent.m_requests[m_id] = this;
        }

        URL_COMPONENTSA urlc;
        memset(&urlc, 0, sizeof(urlc));
        urlc.dwStructSize = sizeof(urlc);
        char hostname[256];
        urlc.lpszHostName = hostname;
        urlc.dwHostNameLength = sizeof(hostname);
        char path[1024];
        urlc.lpszUrlPath = path;
        urlc.dwUrlPathLength = sizeof(path);
        if (!::InternetCrackUrlA(m_request->m_url.data(), (DWORD)m_request->m_url.size(), 0, &urlc)) {
            DWORD dwError = ::GetLastError();
            LOG_WARN("InternetCrackUrl() failed: dwError=%d url=%s", dwError, m_request->m_url.data());
            onRequestComplete(dwError);
            return;
        }

        m_hWinInetSession = ::InternetConnectA(m_parent.m_hInternet, hostname, urlc.nPort,
            NULL, NULL, INTERNET_SERVICE_HTTP, 0, reinterpret_cast<DWORD_PTR>(this));
        if (m_hWinInetSession == NULL) {
            DWORD dwError = ::GetLastError();
            LOG_WARN("InternetConnect() failed: %d", dwError);
            onRequestComplete(dwError);
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
            onRequestComplete(dwError);
            return;
        }

        /* Perform optional MS Root certificate check for certain end-point URLs */
        if (m_parent.IsMsRootCheckRequired())
        {
            if (!isMsRootCert())
            {
                onRequestComplete(ERROR_INTERNET_SEC_INVALID_CERT);
                return;
            }
        }

        ::InternetSetStatusCallback(m_hWinInetRequest, &WinInetRequestWrapper::winInetCallback);

        std::ostringstream os;
        for (auto const& header : m_request->m_headers) {
            os << header.first << ": " << header.second << "\r\n";
        }
        ::HttpAddRequestHeadersA(m_hWinInetRequest, os.str().data(), static_cast<DWORD>(os.tellp()), HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);

        void *data = static_cast<void *>(m_request->m_body.data());
        DWORD size = static_cast<DWORD>(m_request->m_body.size());
        BOOL bResult = ::HttpSendRequest(m_hWinInetRequest, NULL, 0, data, (DWORD)size);
        DWORD dwError = GetLastError();

        if (bResult == TRUE && dwError != ERROR_IO_PENDING) {
            dwError = ::GetLastError();
            LOG_WARN("HttpSendRequest() failed: %d", dwError);
            onRequestComplete(dwError);
        }
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

    void onRequestComplete(DWORD dwError)
    {
        std::unique_ptr<SimpleHttpResponse> response(new SimpleHttpResponse(m_id));

        std::vector<uint8_t> & m_bodyBuffer = response->m_body;
        DWORD m_bufferUsed = 0;

        if (dwError == ERROR_SUCCESS) {
            // If looking good so far, try to fetch the response body first.
            // It might potentially be another async operation which will
            // trigger INTERNET_STATUS_REQUEST_COMPLETE again.

            m_bodyBuffer.insert(m_bodyBuffer.end(), m_buffer, m_buffer + m_bufferUsed);
            do {
                m_bufferUsed = 0;
                BOOL bResult = ::InternetReadFile(m_hWinInetRequest, m_buffer, sizeof(m_buffer), &m_bufferUsed);
                if (!bResult) {
                    dwError = GetLastError();
                    if (dwError == ERROR_IO_PENDING) {
                        // Do not touch anything from this thread anymore.
                        // The buffer passed to InternetReadFile() and the
                        // read count will be filled asynchronously, so they
                        // must stay valid and writable until the next
                        // INTERNET_STATUS_REQUEST_COMPLETE callback comes
                        // (that's why those are member variables).
                        return;
                    }
                    LOG_WARN("InternetReadFile() failed: %d", dwError);
                    break;
                }

                m_bodyBuffer.insert(m_bodyBuffer.end(), m_buffer, m_buffer + m_bufferUsed);
            } while (m_bufferUsed == sizeof(m_buffer));
        }

        if (dwError == ERROR_SUCCESS) {
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

        // 'response' gets released in EventsUploadContext.clear()
        m_appCallback->OnHttpResponse(response.release());
        m_parent.erase(m_id);
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

void HttpClient_WinInet::erase(std::string const& id)
{
    std::lock_guard<std::mutex> lock(m_requestsMutex);
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
    WinInetRequestWrapper* request = nullptr;
    LOCKGUARD(m_requestsMutex);
    auto it = m_requests.find(id);
    if (it != m_requests.end()) {
        request = it->second;
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
        std::lock_guard<std::mutex> lock(m_requestsMutex);
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

} ARIASDK_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT
