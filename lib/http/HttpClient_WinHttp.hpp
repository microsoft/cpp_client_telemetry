//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPCLIENT_WINHTTP_HPP
#define HTTPCLIENT_WINHTTP_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "IHttpClient.hpp"
#include "IBoundedHttpClientCancel.hpp"
#include "pal/PAL.hpp"

#include "ILogManager.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>

namespace MAT_NS_BEGIN {

#ifndef _WINHTTPX_
typedef void* HINTERNET;
#endif

class WinHttpRequestWrapper;
struct WinHttpClientState;

// WinHTTP-based HTTP client. Unlike WinInet, WinHTTP does not depend on a
// logged-on interactive user or that user's Internet Explorer settings, so
// it is Microsoft's recommended transport for services and other
// non-interactive processes (see
// https://learn.microsoft.com/windows/win32/winhttp/porting-wininet-applications-to-winhttp).
// This is the default Win32 desktop transport; HttpClient_WinInet remains
// available as an explicit opt-in for callers that need IE-integrated proxy
// or cookie behavior.
class HttpClient_WinHttp : public IHttpClient, public IBoundedHttpClientCancel {
  public:
    // Common IHttpClient methods
    HttpClient_WinHttp();
    virtual ~HttpClient_WinHttp();
    virtual IHttpRequest* CreateRequest() final;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) final;
    virtual void CancelRequestAsync(std::string const& id) final;
    virtual void CancelAllRequests() final;
    virtual void CancelAllRequests(std::chrono::milliseconds bestEffortTimeout) final;

    virtual void ApplySettings(ILogConfiguration& config) override;

    // Methods unique to WinHttp implementation.
    void SetMsRootCheck(bool enforceMsRoot);
    bool IsMsRootCheckRequired();

  protected:
    std::shared_ptr<WinHttpClientState> m_state;
    static unsigned                  s_nextRequestId;
    friend class WinHttpRequestWrapper;
};

} MAT_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENT_WINHTTP_HPP
