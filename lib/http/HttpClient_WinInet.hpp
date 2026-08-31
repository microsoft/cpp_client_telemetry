//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPCLIENT_WININET_HPP
#define HTTPCLIENT_WININET_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include "IHttpClient.hpp"
#include "IBoundedHttpClientCancel.hpp"
#include "pal/PAL.hpp"

#include "ILogManager.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>

namespace MAT_NS_BEGIN {

#ifndef _WININET_
typedef void* HINTERNET;
#endif

class WinInetRequestWrapper;
struct WinInetClientState;

class HttpClient_WinInet : public IHttpClient, public IBoundedHttpClientCancel {
  public:
    // Common IHttpClient methods
    HttpClient_WinInet();
    virtual ~HttpClient_WinInet();
    virtual IHttpRequest* CreateRequest() final;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) final;
    virtual void CancelRequestAsync(std::string const& id) final;
    virtual void CancelAllRequests() final;
    virtual void CancelAllRequests(std::chrono::milliseconds bestEffortTimeout) final;

    virtual void ApplySettings(ILogConfiguration& config) override;

    // Methods unique to WinInet implementation.
    void SetMsRootCheck(bool enforceMsRoot);
    bool IsMsRootCheckRequired();

  protected:
    std::shared_ptr<WinInetClientState>                              m_state;
    static unsigned                                                  s_nextRequestId;
};

} MAT_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENT_WININET_HPP
