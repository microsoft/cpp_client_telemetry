// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <aria/IHttpClient.hpp>
#include <httpstack/fwd.hpp>
#include "pal/PAL.hpp"

#include <mutex>

namespace ARIASDK_NS_BEGIN {


class HttpClient_HttpStack : public IHttpClient {
  public:
    HttpClient_HttpStack(http_stack::IHttpStack* httpStack);
    virtual ~HttpClient_HttpStack();
    virtual IHttpRequest* CreateRequest() override;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
    virtual void CancelRequestAsync(std::string const& id) override;

  protected:
    http_stack::IHttpStackPtr                      m_httpStack;
    http_stack::IRequestPoolPtr                    m_httpStackPool;
    std::mutex                                     m_httpStackRequestsMutex;
    std::map<std::string, http_stack::IRequestPtr> m_httpStackRequests;
    static unsigned                                s_nextRequestId;

    friend class HttpStackRequestWrapper;
};


} ARIASDK_NS_END
