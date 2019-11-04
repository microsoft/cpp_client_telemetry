// Copyright (c) Microsoft. All rights reserved.
#ifndef HTTPCLIENT_APPLE_HPP
#define HTTPCLIENT_APPLE_HPP

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"
#include "mat.h"

#include <string>

namespace ARIASDK_NS_BEGIN {

    class HttpClient_Apple : public IHttpClient {
    public:
        HttpClient_Apple();
        virtual ~HttpClient_Apple() noexcept;

        virtual IHttpRequest* CreateRequest() override;
        virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
        virtual void CancelRequestAsync(std::string const& id) override;
        virtual void CancelAllRequests() override;

        void Erase(IHttpRequest* req);
        void Add(IHttpRequest* req);

    private:
        std::mutex m_requestsMtx;
        std::map<std::string, IHttpRequest*> m_requests;
    };

} ARIASDK_NS_END

#endif // HTTPCLIENT_APPLE_HPP

