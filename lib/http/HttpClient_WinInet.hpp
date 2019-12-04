// Copyright (c) Microsoft. All rights reserved.
#ifndef HTTPCLIENT_WININET_HPP
#define HTTPCLIENT_WININET_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

#include "ILogManager.hpp"

namespace ARIASDK_NS_BEGIN {

#ifndef _WININET_
typedef void* HINTERNET;
#endif

class WinInetRequestWrapper;

class HttpClient_WinInet : public IHttpClient {
  public:
    // Common IHttpClient methods
    HttpClient_WinInet();
    virtual ~HttpClient_WinInet();
    virtual IHttpRequest* CreateRequest() override;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
    virtual void CancelRequestAsync(std::string const& id) override;
    virtual void CancelAllRequests() override;

    // Methods unique to WinInet implementation.

    // Pass a pointer to ILogManager in order to obtain config parameters, e.g.
    // to check whether MS-Root cert validation is required or not. This logic
    // is currently unique to Win32 / WinInet. Long-term strategy should be
    // to expose a cross-platform HTTP client configuration object. That way we
    // would not need to couple a concrete client implementation with the ILogManager.
    // Current approach is taken to minimuze the code churn of a critical product
    // release.
    void SetParentLogManager(ILogManager* logManager);

  protected:
    void erase(std::string const& id);

  protected:
    HINTERNET                                                        m_hInternet;
    std::mutex                                                       m_requestsMutex;
    std::map<std::string, WinInetRequestWrapper*>                    m_requests;
    static unsigned                                                  s_nextRequestId;

    // TODO: [maxgolov] - allow the client to have its own configuration object.
    // Currently we anchor to owner ILogManager configuration object.
    // HTTP client lifecycle is managed by its owner LM.
    ILogManager*                                                     m_logManager;
    friend class WinInetRequestWrapper;
};

} ARIASDK_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENT_WININET_HPP
