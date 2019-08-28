// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"

#include "pal/WorkerThread.hpp"
#include "pal/WorkerThread_CAPI.hpp"
#include "mat.h"

using namespace testing;
using namespace MAT;
using namespace PAL;
using std::string;

namespace
{
    class TestHelper {
    public:
        void SetShouldExecute(bool shouldExecute) { m_shouldExecute = shouldExecute; }
        bool ShouldExecute() { return m_shouldExecute; }

        void ValidateQueue(std::function<void(work_item_t*)> fn) { m_validateQueueFn = fn; }
        void ValidateCancel(std::function<void(const char*)> fn) { m_validateCancelFn = fn; }
        void ValidateJoin(std::function<void()> fn) { m_validateJoinFn = fn; }
        void ValidateCallback(std::function<void(int, int)> fn) { m_validateCallbackFn = fn; }

        void OnQueue(work_item_t* workItem)
        {
            if (m_validateQueueFn)
                m_validateQueueFn(workItem);
        }

        bool OnCancel(const char* workItemId)
        {
            if (m_validateCancelFn)
                m_validateCancelFn(workItemId);
            return !m_shouldExecute;
        }

        void OnJoin()
        {
            if (m_validateJoinFn)
                m_validateJoinFn();
        }

        void Callback(int param1, int param2)
        {
            if (m_validateCallbackFn)
                m_validateCallbackFn(param1, param2);
        }

    private:
        bool m_shouldExecute = false;
        std::function<void(work_item_t*)> m_validateQueueFn;
        std::function<void(const char*)> m_validateCancelFn;
        std::function<void()> m_validateJoinFn;
        std::function<void(int, int)> m_validateCallbackFn;
    };

    static std::unique_ptr<TestHelper> s_testHelper;

    // RAII helper that automatically uninstalls static TestHelper instance upon destruction
    class AutoTestHelper {
    public:
        AutoTestHelper()
        {
            s_testHelper.reset(new TestHelper());
        }

        ~AutoTestHelper()
        {
            s_testHelper = nullptr;
        }
        
        TestHelper* operator->()
        {
            return s_testHelper.get();
        }

        TestHelper* get()
        {
            return s_testHelper.get();
        }
    };
} // namespace

void EVTSDK_LIBABI_CDECL OnWorkerThreadQueue(work_item_t* workItem, work_item_callback_fn_t callback)
{
    s_testHelper->OnQueue(workItem);

    if (s_testHelper->ShouldExecute())
        callback(workItem->id);
}

bool EVTSDK_LIBABI_CDECL OnWorkerThreadCancel(const char* workItemId)
{
    return s_testHelper->OnCancel(workItemId);
}

void EVTSDK_LIBABI_CDECL OnWorkerThreadJoin()
{
    s_testHelper->OnJoin();
}

TEST(WorkerThreadCAPITests, Execute)
{
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnWorkerThreadQueue, &OnWorkerThreadCancel, &OnWorkerThreadJoin);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(true);

    // Validate C++ -> C transformation of worker thread item
    bool wasQueued = false;
    testHelper->ValidateQueue([&wasQueued](work_item_t* workItem) {
        wasQueued = true;
        EXPECT_EQ(workItem->delayMs, 0);
        EXPECT_TRUE(std::string(workItem->typeName).find("TestHelper") != string::npos);
    });

    // Validate callback execution
    bool wasExecuted = false;
    testHelper->ValidateCallback([&wasExecuted](int param1, int param2) {
        wasExecuted = true;
        EXPECT_EQ(param1, 10);
        EXPECT_EQ(param2, 20);
    });

    executeOnWorkerThread(workerThread.get(), testHelper.get(), &TestHelper::Callback, 10 /*param1*/, 20 /*param2*/);

    EXPECT_EQ(wasQueued, true);
    EXPECT_EQ(wasExecuted, true);
}

TEST(WorkerThreadCAPITests, Schedule)
{
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnWorkerThreadQueue, &OnWorkerThreadCancel, &OnWorkerThreadJoin);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(true);

    // Validate C++ -> C transformation of worker thread item
    bool wasQueued = false;
    testHelper->ValidateQueue([&wasQueued](work_item_t* workItem) {
        wasQueued = true;
        EXPECT_EQ(workItem->delayMs, 100);
        EXPECT_TRUE(std::string(workItem->typeName).find("TestHelper") != string::npos);
    });

    // Validate callback execution
    bool wasExecuted = false;
    testHelper->ValidateCallback([&wasExecuted](int param1, int param2) {
        wasExecuted = true;
        EXPECT_EQ(param1, 10);
        EXPECT_EQ(param2, 20);
    });

    scheduleOnWorkerThread(workerThread.get(), 100 /*delayMs*/, testHelper.get(), &TestHelper::Callback, 10 /*param1*/, 20 /*param2*/);

    EXPECT_EQ(wasQueued, true);
    EXPECT_EQ(wasExecuted, true);
}

TEST(WorkerThreadCAPITests, Cancel)
{
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnWorkerThreadQueue, &OnWorkerThreadCancel, &OnWorkerThreadJoin);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(false);

    // Validate C++ -> C transformation of worker thread item
    string workItemIdStr;
    bool wasQueued = false;
    testHelper->ValidateQueue([&wasQueued, &workItemIdStr](work_item_t* workItem) {
        wasQueued = true;
        workItemIdStr = workItem->id;
        EXPECT_EQ(workItem->delayMs, 100);
        EXPECT_TRUE(std::string(workItem->typeName).find("TestHelper") != string::npos);
    });

    // Validate cancellation
    bool wasCancelled = false;
    testHelper->ValidateCancel([&wasCancelled, &workItemIdStr](const char* workItemId) {
        wasCancelled = true;
        EXPECT_EQ(workItemIdStr, workItemId);
    });

    // Validate callback execution
    testHelper->ValidateCallback([](int /*param1*/, int /*param2*/) {
        FAIL() << "Callback should not have been executed";
    });

    auto handle = scheduleOnWorkerThread(workerThread.get(), 100 /*delayMs*/, testHelper.get(), &TestHelper::Callback, 10 /*param1*/, 20 /*param2*/);
    handle.cancel();

    EXPECT_EQ(wasQueued, true);
    EXPECT_EQ(wasCancelled, true);
}

TEST(WorkerThreadCAPITests, Join)
{
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnWorkerThreadQueue, &OnWorkerThreadCancel, &OnWorkerThreadJoin);

    AutoTestHelper testHelper;

    // Validate C++ -> C transformation of worker thread item
    bool wasJoin = false;
    testHelper->ValidateJoin([&wasJoin]() {
        wasJoin = true;
    });

    workerThread->join();
    EXPECT_EQ(wasJoin, true);
}
