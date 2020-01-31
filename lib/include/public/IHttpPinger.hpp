// Copyright (c) Microsoft. All rights reserved.
#ifndef IHTTPPINGER_HPP
#define IHTTPPINGER_HPP

#include "Version.hpp"

#include "IModule.hpp"
#include "Enums.hpp"
#include "IHttpClient.hpp"
#include "IModule.hpp"

#include <string>

///@cond INTERNAL_DOCS
namespace ARIASDK_NS_BEGIN
{
    enum HttpPingResult
    {
        HttpPingResult_Unknown    = -1, // Connection not attempted yet
        HttpPingResult_OK         =  0, // Connection successful: server responded with 200 OK
        HttpPingResult_ConnFailed =  2, // Connection attempted: timeout or not 200 OK
        HttpPingResult_CertFailed =  3  // Connection attempted: Secure Socket failure
    };

    /// <summary>
    /// IHttpPinger override module
    /// </summary>
    class IHttpPinger : public IHttpResponseCallback, public IModule
    {

     protected:
        ILogManager* m_owner { nullptr };
        HttpPingResult m_lastResult { HttpPingResult_Unknown };

     public:

      /**
       * Construct connectivity checker.
       */
      IHttpPinger() = default;

      /**
       * Destroy connectivity checker.
       */
      virtual ~IHttpPinger() {};

      /**
       * Performs sync ping of a default URL passed to HttpPinger constructor.
       * This method would block if there is a Ping pending.
       * Returns HttpResult_OK if ping is successful.
       */
      virtual HttpPingResult Ping() = 0;

      /**
       * Thread-safe way to obtain the last ping result.
       * This method would block if there is a Ping pending.
       */
      virtual HttpPingResult GetLastResult() = 0;

      /**
       * Optional callback for systems that require an instance of HTTP stack.
       * Some systems may rely on external reachability code, thus the method
       * has a default no-op implementation.
       */
      virtual void OnHttpResponse(IHttpResponse* response) override {};

      /**
       * Optional callback for systems that require an instance of HTTP stack.
       * Some systems may rely on external reachability code, thus the method
       * has a default no-op implementation.
       */
      virtual void OnHttpStateEvent(HttpStateEvent state, void* data, size_t size) override {};

      /**
       * Initialize is called on LogManager::Initialize.
       * Cannot use Pinger until its owner is inited.
       */
      virtual void Initialize(ILogManager *owner) noexcept override
      {
          m_owner = owner;
      }

      /**
       * Teardown must be called on LogManager::FlushAndTeardown
       * when all pending pings have been consolidated in TPM stop.
       */
      virtual void Teardown() noexcept override
      {
          m_owner = nullptr;
      }

    };

}
ARIASDK_NS_END

#endif
