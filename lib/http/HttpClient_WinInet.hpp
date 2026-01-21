//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPCLIENT_WININET_HPP
#define HTTPCLIENT_WININET_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"
#include <cstddef>

#include "ILogManager.hpp"

namespace MAT_NS_BEGIN {

#ifndef _WININET_
typedef void* HINTERNET;
#endif
#define CERTIFICATE_THUMBPRINT_SHA256_SIZE 32

class WinInetRequestWrapper;

class HttpClient_WinInet : public IHttpClient {
  public:
    // Common IHttpClient methods
    HttpClient_WinInet();
    virtual ~HttpClient_WinInet();
    virtual IHttpRequest* CreateRequest() final;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) final;
    virtual void CancelRequestAsync(std::string const& id) final;
    virtual void CancelAllRequests() final;

    // Methods unique to WinInet implementation.
    void SetMsRootCheck(bool enforceMsRoot);
    bool IsMsRootCheckRequired();

    void SetCustomRootCheck(bool enforceCustomRoot);
    bool IsCustomRootCheckRequired();
    bool AddCustomRootCertSHA256Thumbprint(std::array<std::byte, CERTIFICATE_THUMBPRINT_SHA256_SIZE>& aCertThumbprint);
    bool IsTrustedRootCert(std::array<std::byte, CERTIFICATE_THUMBPRINT_SHA256_SIZE>& aCertThumbprint);
    bool AddCustomTrustedSubjectOrg(std::string& aTrustedOrg);
    bool IsTrustedSubjectOrg(char* aTrustedOrg);

   protected:
    void erase(std::string const& id);

  protected:
    HINTERNET                                                        m_hInternet;
    std::recursive_mutex                                             m_requestsMutex;
    std::map<std::string, WinInetRequestWrapper*>                    m_requests;
    static unsigned                                                  s_nextRequestId;
    bool                                                             m_msRootCheck;
    bool                                                             m_fcustomRootCheck;
    std::vector<std::array<std::byte, CERTIFICATE_THUMBPRINT_SHA256_SIZE>> m_customRootCerts;
    std::vector<std::string>                                         m_customTrustedSubjectOrgs;
    friend class WinInetRequestWrapper;
};

} MAT_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENT_WININET_HPP

