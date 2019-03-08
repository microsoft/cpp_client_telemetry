// Copyright (c) Microsoft. All rights reserved.

#pragma once
#pragma comment(lib, "windowsapp")
#pragma comment(lib, "runtimeobject")

#include <Windows.h>

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

#include <ppltasks.h>

using namespace ::Windows::Foundation;
using namespace ::Windows::Foundation::Collections;
using namespace ::Windows::Web;
using namespace ::Windows::Web::Http;
using namespace ::Windows::Web::Http::Filters;
using namespace ::Windows::Web::Http::Headers;

namespace ARIASDK_NS_BEGIN {

class WinRtRequestWrapper;

class HttpClient_WinRt : public IHttpClient {
  public:
    HttpClient_WinRt();
    virtual ~HttpClient_WinRt();
    virtual IHttpRequest* CreateRequest() override;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
    virtual void CancelRequestAsync(std::string const& id) override;
    virtual void CancelAllRequests() override;
	HttpClient^ getHttpClient() { return m_httpClient; }

  protected:
    void erase(std::string const& id);
	
  protected:
	HttpClient^                                                      m_httpClient;
    std::mutex                                                       m_requestsMutex;
    std::map<std::string, WinRtRequestWrapper*>                      m_requests;
    static unsigned                                                  s_nextRequestId;

    friend class WinRtRequestWrapper;
};


} ARIASDK_NS_END
