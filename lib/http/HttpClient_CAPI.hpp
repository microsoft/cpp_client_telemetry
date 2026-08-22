//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPCLIENT_CAPI_HPP
#define HTTPCLIENT_CAPI_HPP

#include "IHttpClient.hpp"
#include "pal/PAL.hpp"
#include "mat.h"

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

namespace MAT_NS_BEGIN {

    class HttpClient_CAPI_State;

    class HttpClient_CAPI : public IHttpClient {
    public:
        HttpClient_CAPI(http_send_fn_t sendFn, http_cancel_fn_t cancelFn);
        ~HttpClient_CAPI() noexcept override;

        virtual IHttpRequest* CreateRequest() override;
        virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
        virtual void CancelRequestAsync(std::string const& id) override;
        virtual void CancelAllRequests() override;

    private:
        http_send_fn_t    m_sendFn;
        http_cancel_fn_t  m_cancelFn;
        std::shared_ptr<HttpClient_CAPI_State> m_state;
    };

} MAT_NS_END

#endif // HTTPCLIENT_CAPI_HPP
