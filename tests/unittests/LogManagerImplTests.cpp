//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "api/LogManagerImpl.hpp"
#include "common/Common.hpp"
#include <future>

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

TEST(LogManagerImplTests, NotPausedInitially)
{
    ILogConfiguration configuration;
    TestLogManagerImpl logManager{configuration};
    ASSERT_TRUE(logManager.StartActivity());
    logManager.EndActivity();
}

TEST(LogManagerImplTests, ImmediateNotify)
{
    ILogConfiguration configuration;
    TestLogManagerImpl logManager{configuration};
    auto waiter = std::async([&logManager]() -> void {
        logManager.WaitPause();
    });
    auto result = waiter.wait_for(std::chrono::milliseconds(50));
    ASSERT_EQ(std::future_status::ready, result);
}

TEST(LogManagerImplTests, DeferredNotify)
{
    ILogConfiguration configuration;
    TestLogManagerImpl logManager{configuration};
    ASSERT_TRUE(logManager.StartActivity());
    logManager.PauseActivity();
    auto waiter = std::async([&logManager]() -> void {
                      logManager.WaitPause();
                  });
    auto pausing_result = waiter.wait_for(std::chrono::milliseconds(50));
    ASSERT_EQ(std::future_status::timeout, pausing_result);
    logManager.EndActivity();
    auto subsequent = std::async([&logManager]() -> void {
        logManager.WaitPause();
    });
    auto paused_result = subsequent.wait_for(std::chrono::milliseconds(50));
    ASSERT_EQ(std::future_status::ready, paused_result);
}

TEST(LogManagerImplTests, DoesPause)
{
    ILogConfiguration configuration;
    TestLogManagerImpl logManager{configuration};
    logManager.PauseActivity();
    ASSERT_FALSE(logManager.StartActivity());
}

TEST(LogManagerImplTests, WaitOnResume)
{
    ILogConfiguration configuration;
    TestLogManagerImpl logManager{configuration};
    ASSERT_TRUE(logManager.StartActivity());
    logManager.PauseActivity();
    logManager.ResumeActivity();
    auto waiter = std::async([&logManager]() -> void {
        logManager.WaitPause();
    });
    auto activeResult = waiter.wait_for(std::chrono::milliseconds(50));
    ASSERT_EQ(std::future_status::ready, activeResult);
    logManager.EndActivity();
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
