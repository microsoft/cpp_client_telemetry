// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "api/LogManagerImpl.hpp"

using namespace testing;
using namespace MAT;

class TestLogManagerImpl : public LogManagerImpl
{
public:
   TestLogManagerImpl(ILogConfiguration& configuration, IHttpClient* httpClient)
      : TestLogManagerImpl(configuration, httpClient, false) { }
   TestLogManagerImpl(ILogConfiguration& configuration, IHttpClient* httpClient, bool deferSystemStart)
      : LogManagerImpl(configuration, httpClient, deferSystemStart) { }

   using LogManagerImpl::m_httpClient;
   using LogManagerImpl::m_ownHttpClient;
   using LogManagerImpl::m_modules;
   using LogManagerImpl::TeardownModules;
   using LogManagerImpl::InitializeModules;
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
   TestLogManagerImpl logManager { configuration, nullptr };
   ASSERT_NE(logManager.m_ownHttpClient, nullptr);
#else
   EXPECT_THROW(TestLogManagerImpl(configuration, nullptr), std::invalid_argument);
#endif
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_HttpClientAndOwnHttpClientAreSame)
{
   ILogConfiguration configuration;
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
   TestLogManagerImpl logManager { configuration, nullptr };
   ASSERT_EQ(logManager.m_ownHttpClient.get(), logManager.m_httpClient);
#else
   EXPECT_THROW(TestLogManagerImpl(configuration, nullptr), std::invalid_argument);
#endif
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNotNullptr_DoesNotConstructOwnHttpClient)
{
   ILogConfiguration configuration;
   TestHttpClient httpClient;
   TestLogManagerImpl logManager { configuration, &httpClient, true };
   ASSERT_EQ(logManager.m_ownHttpClient, nullptr);
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNotNullptr_HttpClientIsSet)
{
   ILogConfiguration configuration;
   TestHttpClient httpClient;
   TestLogManagerImpl logManager { configuration, &httpClient, true };
   ASSERT_EQ(logManager.m_httpClient, &httpClient);
}

class LogManagerModuleTests : public ::testing::Test
{
public:
    class TestModule : public IModule
    {
    public:
        TestModule(bool& initialzeCalled, bool& teardownCalled, const ILogManager*& pointerPassedToInitialize) noexcept
            : initializeCalled(initialzeCalled)
            , teardownCalled(teardownCalled)
            , addressPassedToInitialize(pointerPassedToInitialize) { }

        virtual void Initialize(ILogManager* parentManager) noexcept override
        {
            addressPassedToInitialize = parentManager;
            initializeCalled = true;
        }

        virtual void Teardown() noexcept
        {
            teardownCalled = true;
        }

        bool& initializeCalled;
        bool& teardownCalled;
        const ILogManager*& addressPassedToInitialize;
    };

    ILogConfiguration configuration;
    TestLogManagerImpl logManager{ configuration, nullptr };
    const ILogManager* AddressPassedToInitialize{};
    bool InitializeCalled{};
    bool TeardownCalled{};

    void SetUp() override
    {
        logManager.m_modules.push_back(std::unique_ptr<TestModule>(new TestModule(InitializeCalled, TeardownCalled, AddressPassedToInitialize)));
    }
};

TEST_F(LogManagerModuleTests, InitializeModules_OneModuleRegistered_CallsInitialize)
{
    logManager.InitializeModules();
    ASSERT_TRUE(InitializeCalled);
}

TEST_F(LogManagerModuleTests, InitializeModules_OneModuleRegistered_LogManagerPassedIsSame)
{
    logManager.InitializeModules();
    ASSERT_EQ(&logManager, AddressPassedToInitialize);
}

TEST_F(LogManagerModuleTests, TeardownModules_OneModuleRegistered_UnregisterCalled)
{
    logManager.TeardownModules();
    ASSERT_TRUE(TeardownCalled);
}

TEST_F(LogManagerModuleTests, TeardownModules_OneModuleRegistered_SizeOfModulesIsZero)
{
    logManager.TeardownModules();
    ASSERT_EQ(logManager.m_modules.size(), size_t{ 0 });
}

TEST(LogManagerImplTests, Constructor_DataViewerCollectionIsNotNullptr_DataViewerCollectionIsSet)
{
    ILogConfiguration configuration;
    TestHttpClient httpClient;
    TestLogManagerImpl logManager { configuration, &httpClient, true };
    ASSERT_NO_THROW(logManager.GetDataViewerCollection());
}