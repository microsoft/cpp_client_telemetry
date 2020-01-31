// Copyright (c) Microsoft. All rights reserved.
#ifndef HTTPPINGER_HPP
#define HTTPPINGER_HPP

#include "IHttpPinger.hpp"

#include "WorkerThread.hpp"
#include "pal/PAL.hpp"

#include "http/HttpClientFactory.hpp"

#include <atomic>
#include <cassert>
#include <memory>
#include <mutex>

namespace ARIASDK_NS_BEGIN
{
    class HttpPinger : public IHttpPinger
    {
       protected:
        std::atomic<HttpPingResult> m_lastResult{HttpPingResult_Unknown};
        std::shared_ptr<IHttpClient> m_httpClient{nullptr};
        std::string m_pingUrl;
        std::mutex m_pingMtx;
        std::atomic<bool> m_isRunning{false};

       public:
        /**
         *
         */
        HttpPinger() :
            IHttpPinger()
        {
        }

        ///
        /// Check if HTTP client is initialized. Lazily create Http client as-needed.
        ///
        bool IsInitialized()
        {
            if (m_httpClient == nullptr)
            {
                m_lastResult = HttpPingResult_Unknown;
                if (m_owner != nullptr)
                {
                    /**
                     * TODO: [maxgolov]
                     * Portion of code that can be shared between
                     * LogManagerImpl.cpp and HttpPinger.hpp
                     */
                    auto configuration = m_owner->GetLogConfiguration();
                    LOG_INFO("Trying to obtain HTTP client module...");
                    m_httpClient = std::static_pointer_cast<IHttpClient>(configuration.GetModule(CFG_MODULE_HTTP_CLIENT));
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
                    if (m_httpClient == nullptr)
                    {
                        LOG_INFO("Initializing default HTTP client...");
                        m_httpClient = HttpClientFactory::Create();
#ifdef HAVE_MAT_WININET_HTTP_CLIENT
                        HttpClient_WinInet* client = static_cast<HttpClient_WinInet*>(m_httpClient.get());
                        if (client != nullptr)
                        {
                            client->SetMsRootCheck(configuration["http"]["msRootCheck"]);
                        }
#endif
                    }
#endif
                }
            }

            return (m_httpClient != nullptr);
        }

        /**
         * Performs sync ping of a default URL passed to HttpPinger constructor.
         *
         * Returns HttpResult_OK if ping is successful.
         *
         */
        virtual void Ping() override
        {
            LOCKGUARD(m_pingMtx);
            if (IsInitialized())
            {
                auto req = m_httpClient->CreateRequest();
                req->SetMethod("GET");
                req->SetUrl(m_pingUrl);
                m_httpClient->SendRequestAsync(req, this);
            }
        }

        ///
        /// Get last cached ping result
        ///
        virtual HttpPingResult GetLastResult() override
        {
            LOCKGUARD(m_pingMtx);
            return m_lastResult;
        }

        virtual void Reset() override
        {
            m_lastResult = HttpPingResult_Unknown;
        }

        ///
        /// Method is invoked when HTTP GET /ping response is ready
        ///
        virtual void OnHttpResponse(IHttpResponse* response) override
        {
            assert(response != nullptr);
            m_lastResult = (
                (response->GetResult() == HttpResult_OK) && (response->GetStatusCode() == 200)) ?
                HttpPingResult_OK :
                HttpPingResult_ConnFailed;
            if (m_lastResult == HttpPingResult_OK)
            {
                auto v = response->GetBody();
                if (!v.empty())
                {
                    std::string body(v.begin(), v.end());
                    if (body.find(">ok<") == std::string::npos)
                    {
                        // "Not Penny's Boat" :( This is NOT what
                        // we expect to get back from our collector.
                        m_lastResult = HttpPingResult_ConnFailed;
                        LOG_WARN("Invalid response received from %s", m_pingUrl.c_str());
                    }
                }
            }
            delete response;
            if (OnPingCompleted != nullptr)
            {
                OnPingCompleted(m_lastResult);
            }
        }

        /**
         *
         */
        virtual void OnHttpStateEvent(HttpStateEvent state, void* data, size_t size) override
        {
            std::ignore = data;
            std::ignore = size;
#if defined(HAVE_LIBCURL) && defined(HAVE_SSL_VERIFY)
            // TODO: handle the case when SSL/TLS certificate is invalid
#endif
            switch (state)
            {
            case OnCreateFailed:
                //no break
            case OnConnectFailed:
                //no break
            case OnSendFailed:
                m_lastResult = HttpPingResult_ConnFailed;
                LOG_WARN("HTTP ping connection failed: %d", state);
                break;
            default:
                break;
            }
        }

        virtual void Teardown() noexcept override
        {
            m_httpClient = nullptr;
            OnPingCompleted = nullptr;
        }

        virtual void Initialize(ILogManager* owner) noexcept override
        {
            IHttpPinger::Initialize(owner);
            if (owner != nullptr)
            {
                auto configuration = owner->GetLogConfiguration();
                // Example service URL after appending /ping
                // https://self.events.data.microsoft.com/ping
                m_pingUrl = static_cast<const char*>(configuration[CFG_STR_COLLECTOR_URL]);
                size_t pos = m_pingUrl.find("://");
                pos = m_pingUrl.find("/", pos + 3);
                m_pingUrl.erase(pos + 1);
                m_pingUrl.append("ping");
            }
        }
    };

}
ARIASDK_NS_END

#endif
