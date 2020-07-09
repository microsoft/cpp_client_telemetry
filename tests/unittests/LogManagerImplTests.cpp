// Copyright (c) Microsoft. All rights reserved.
#include "api/LogManagerImpl.hpp"
#include "common/Common.hpp"

using namespace testing;
using namespace MAT;

class TestLogManagerImpl : public LogManagerImpl
{
   public:
    TestLogManagerImpl(ILogConfiguration& configuration) :
        TestLogManagerImpl(configuration, false)
    {
    }
    TestLogManagerImpl(ILogConfiguration& configuration, bool deferSystemStart) :
        LogManagerImpl(configuration, deferSystemStart)
    {
    }

    using LogManagerImpl::m_httpClient;
    // using LogManagerImpl::m_ownHttpClient;
    using LogManagerImpl::InitializeModules;
    using LogManagerImpl::m_modules;
    using LogManagerImpl::TeardownModules;
};

class TestHttpClient : public IHttpClient
{
   public:
    IHttpRequest* theOnlyRequest = nullptr;
    virtual IHttpRequest* CreateRequest() override
    {
        return theOnlyRequest;
    }
    virtual void SendRequestAsync(IHttpRequest*, IHttpResponseCallback*) override
    {
    }
    virtual void CancelRequestAsync(std::string const&) override
    {
    }
};

TEST(LogManagerImplTests, Constructor_HttpClientIsNullptr_ConstructsOwnHttpClient)
{
    ILogConfiguration configuration;
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
    TestLogManagerImpl logManager{configuration};
    ASSERT_NE(logManager.m_httpClient, nullptr);
#else
    EXPECT_THROW(TestLogManagerImpl{configuration}, std::invalid_argument);
#endif
}

TEST(LogManagerImplTests, Constructor_HttpClientIsNotNullptr_HttpClientIsSet)
{
    ILogConfiguration configuration;
    // TestLogManagerImpl logManager { configuration, &httpClient, true };
    // ASSERT_EQ(logManager.m_httpClient, &httpClient);
    auto httpClient = std::make_shared<TestHttpClient>();
    configuration.AddModule(CFG_MODULE_HTTP_CLIENT, httpClient);
    TestLogManagerImpl logManager{configuration, true};
    ASSERT_EQ(logManager.m_httpClient, httpClient);
}

TEST(LogManagerImplTests, DeadLoggersAreDead)
{
    ILogConfiguration configuration;
    auto httpClient = std::make_shared<TestHttpClient>();
    httpClient->theOnlyRequest = new SimpleHttpRequest("fred");
    configuration.AddModule(CFG_MODULE_HTTP_CLIENT, httpClient);
    size_t onEntry = LogManagerImpl::GetDeadLoggerCount();
    TestLogManagerImpl logManager{configuration};
    logManager.PauseTransmission();
    ASSERT_EQ(onEntry, LogManagerImpl::GetDeadLoggerCount());
    auto logger = logManager.GetLogger("fred");
    logManager.FlushAndTeardown();
    ASSERT_EQ(onEntry + 1, LogManagerImpl::GetDeadLoggerCount());
    logger->LogEvent("DeadLoggerEvent");
}

class LogManagerModuleTests : public ::testing::Test
{
   public:
    class TestModule : public IModule
    {
       public:
        TestModule(bool& initialzeCalled, bool& teardownCalled, const ILogManager*& pointerPassedToInitialize) noexcept
            :
            initializeCalled(initialzeCalled), teardownCalled(teardownCalled), addressPassedToInitialize(pointerPassedToInitialize)
        {
        }

        virtual void Initialize(ILogManager* parentManager) noexcept override
        {
            addressPassedToInitialize = parentManager;
            initializeCalled = true;
        }

        virtual void Teardown() noexcept override
        {
            teardownCalled = true;
        }

        bool& initializeCalled;
        bool& teardownCalled;
        const ILogManager*& addressPassedToInitialize;
    };

    ILogConfiguration configuration;
    TestLogManagerImpl logManager{configuration};
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
    ASSERT_EQ(logManager.m_modules.size(), size_t{0});
}

TEST(LogManagerImplTests, Constructor_DataViewerCollectionIsNotNullptr_DataViewerCollectionIsSet)
{
    ILogConfiguration configuration;
    TestHttpClient httpClient;
    TestLogManagerImpl logManager{configuration, true};
    ASSERT_NO_THROW(logManager.GetDataViewerCollection());
}
