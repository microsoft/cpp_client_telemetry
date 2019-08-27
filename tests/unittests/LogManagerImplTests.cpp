// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "api/LogManagerImpl.hpp"

using namespace testing;
using namespace MAT;

class TestLogManagerImpl : public LogManagerImpl
{
public:
   TestLogManagerImpl(ILogConfiguration& configuration, IHttpClient* httpClient, IWorkerThread* workerThread)
      : TestLogManagerImpl(configuration, httpClient, workerThread, false) { }
   TestLogManagerImpl(ILogConfiguration& configuration, IHttpClient* httpClient, IWorkerThread* workerThread, bool deferSystemStart)
      : LogManagerImpl(configuration, httpClient, workerThread, deferSystemStart) { }

   using LogManagerImpl::m_httpClient;
   using LogManagerImpl::m_ownHttpClient;
};

class TestHttpClient : public IHttpClient
{
   virtual IHttpRequest* CreateRequest() override { return nullptr; }
   virtual void SendRequestAsync(IHttpRequest*, IHttpResponseCallback*) override { }
   virtual void CancelRequestAsync(std::string const&) override { }
};

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_ConstructsOwnHttpClient)
{
   ILogConfiguration configuration;
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
   TestLogManagerImpl logManager { configuration, nullptr, nullptr };
   ASSERT_NE(logManager.m_ownHttpClient, nullptr);
#else
   EXPECT_THROW(TestLogManagerImpl(configuration, nullptr, nullptr), std::invalid_argument);
#endif
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_HttpClientAndOwnHttpClientAreSame)
{
   ILogConfiguration configuration;
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
   TestLogManagerImpl logManager { configuration, nullptr, nullptr };
   ASSERT_EQ(logManager.m_ownHttpClient.get(), logManager.m_httpClient);
#else
   EXPECT_THROW(TestLogManagerImpl(configuration, nullptr, nullptr), std::invalid_argument);
#endif
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNotNullptr_DoesNotConstructOwnHttpClient)
{
   ILogConfiguration configuration;
   TestHttpClient httpClient;
   TestLogManagerImpl logManager { configuration, &httpClient, nullptr, true };
   ASSERT_EQ(logManager.m_ownHttpClient, nullptr);
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNotNullptr_HttpClientIsSet)
{
   ILogConfiguration configuration;
   TestHttpClient httpClient;
   TestLogManagerImpl logManager { configuration, &httpClient, nullptr, true };
   ASSERT_EQ(logManager.m_httpClient, &httpClient);
}