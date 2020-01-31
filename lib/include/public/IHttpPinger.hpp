// Copyright (c) Microsoft. All rights reserved.
#ifndef IHTTPPINGER_HPP
#define IHTTPPINGER_HPP

#include "Version.hpp"

#include "Enums.hpp"
#include "IHttpClient.hpp"
#include "IModule.hpp"

#include <string>

///@cond INTERNAL_DOCS
namespace ARIASDK_NS_BEGIN
{
    enum HttpPingResult
    {
        HttpPingResult_Unknown = -1,    // Connection not attempted yet
        HttpPingResult_OK = 0,          // Connection successful: server responded with 200 OK
        HttpPingResult_ConnFailed = 2,  // Connection attempted: timeout or not 200 OK
        HttpPingResult_CertFailed = 3   // Connection attempted: Secure Socket failure
    };

    /// <summary>
    /// IHttpPinger override module
    /// </summary>
    class IHttpPinger : public IHttpResponseCallback, public IModule
    {
       protected:
        ILogManager* m_owner{nullptr};

       public:
        std::function<void(HttpPingResult)> OnPingCompleted{nullptr};

        /// <summary>
        /// Construct HTTP ping connectivity checker.
        /// </summary>
        IHttpPinger() = default;

        /// <summary>
        /// Destroy HTTP ping connectivity checker.
        /// </summary>
        virtual ~IHttpPinger(){};

        /// <summary>
        /// Performs async ping of a collector URL.
        /// </summary>
        virtual void Ping() = 0;

        /// <summary>
        /// Thread-safe way to obtain the last ping result.
        /// This method would block if there is a Ping result pending.
        /// </summary>
        virtual HttpPingResult GetLastResult() = 0;

        /// <summary>
        /// Reset last result to force another ping
        /// </summary>
        virtual void Reset() = 0;

        /// <summary>
        /// Optional callback for systems that require an instance of HTTP stack.
        /// Some systems may rely on external reachability code. Thus the method
        /// has a default no-op implementation.
        /// </summary>
        virtual void OnHttpResponse(IHttpResponse*) override{};

        /// <summary>
        /// Optional callback for systems that require an instance of HTTP stack.
        /// Some systems may rely on external reachability code, thus the method
        /// has a default no-op implementation.
        /// </summary>
        virtual void OnHttpStateEvent(HttpStateEvent, void*, size_t) override{};

        /// <summary>
        /// Initialize is called on LogManager::Initialize.
        /// Cannot use Pinger until its owner is inited.
        /// </summary>
        virtual void Initialize(ILogManager* owner) noexcept override
        {
            m_owner = owner;
        }

        /// <summary>
        /// Teardown must be called on LogManager::FlushAndTeardown
        /// when all pending pings have been consolidated in TPM stop.
        /// </summary>
        virtual void Teardown() noexcept override
        {
            m_owner = nullptr;
        }
    };

}
ARIASDK_NS_END

#endif
