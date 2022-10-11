//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

// Assume that if we are compiling with MSVC, then we prefer to use Windows HTTP stack,
// e.g. WinInet.dll or Win 10 HTTP client instead
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)

#include "HttpClient_Apple.hpp"
#include "HttpRequestApple.hpp"

namespace MAT_NS_BEGIN {

HttpClient_Apple::HttpClient_Apple()
{
    LOG_TRACE("Initializing HttpClient_Apple...");
}

HttpClient_Apple::~HttpClient_Apple() noexcept
{
    LOG_TRACE("Shutting down HttpClient_Apple...");
}

IHttpRequest* HttpClient_Apple::CreateRequest()
{
    auto request = new HttpRequestApple(this);
    LOG_TRACE("HTTP request=%p id=%s created", request, request->GetId().c_str());
    return request;
}

void HttpClient_Apple::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
{
    auto requestApple = static_cast<HttpRequestApple*>(request);
    requestApple->SendAsync(callback);
    LOG_TRACE("HTTP request=%p callback=%p sent", request, callback);
}

void HttpClient_Apple::CancelRequestAsync(const std::string& id)
{
    HttpRequestApple* request = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_requestsMtx);
        if (m_requests.find(id) != m_requests.cend())
        {
            request = static_cast<HttpRequestApple*>(m_requests[id]);
            if (request != nullptr)
            {
                LOG_TRACE("HTTP request=%p id=%s being aborted...", request, id.c_str());
                request->Cancel();
            }
            m_requests.erase(id);
        }
    }
}

void HttpClient_Apple::CancelAllRequests()
{
    std::vector<std::string> ids;
    {
        std::lock_guard<std::mutex> lock(m_requestsMtx);
        for (auto const& item : m_requests) {
            ids.push_back(item.first);
        }
    }

    for (const auto &id : ids)
        CancelRequestAsync(id);

    while (!m_requests.empty())
    {
        PAL::sleep(100);
        std::this_thread::yield();
    }
}

void HttpClient_Apple::Erase(IHttpRequest* req)
{
    std::lock_guard<std::mutex> lock(m_requestsMtx);
    m_requests.erase(req->GetId());
}

void HttpClient_Apple::Add(IHttpRequest* req)
{
    std::lock_guard<std::mutex> lock(m_requestsMtx);
    m_requests[req->GetId()] = req;
}

} MAT_NS_END

#endif
