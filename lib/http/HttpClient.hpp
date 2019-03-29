// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IHttpClient.hpp"
#include "pal/PAL.hpp"

#include <map>
#include <mutex>

namespace ARIASDK_NS_BEGIN {

struct IHttpRequestCompare
{
   bool operator() (const IHttpRequest* lhs, const IHttpRequest* rhs) const
   {
       if ((lhs == nullptr)||(rhs == nullptr))
           return false;
       return (lhs->GetId().compare(rhs->GetId()) == 0);
   }
};

class HttpClient : public IHttpClient {

  public:
    HttpClient();
    virtual ~HttpClient();
    virtual IHttpRequest* CreateRequest() override;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
    virtual void CancelRequestAsync(std::string const& id) override;

    /**
     * Remove request from the map
     * @param req
     */
    void erase(IHttpRequest * req)
    {
        std::lock_guard<std::mutex> lock(m_request_mtx);
        m_requests.erase(req->GetId());
    }

    /**
     * Add request-response to the map
     *
     * @param req
     * @param response
     */
    void add(IHttpRequest * req)
    {
        std::lock_guard<std::mutex> lock(m_request_mtx);
        m_requests[req->GetId()] = req;
    }

  protected:

    // Map of requests - responses
    std::mutex m_request_mtx;
    std::map<std::string, IHttpRequest*> m_requests;

    friend class SimpleHttpRequest;
};

} ARIASDK_NS_END
