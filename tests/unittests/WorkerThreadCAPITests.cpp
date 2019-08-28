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

        void ValidateQueue(std::function<void(async_task_t*)> fn) { m_validateQueueFn = fn; }
        void ValidateCancel(std::function<void(const char*)> fn) { m_validateCancelFn = fn; }
        void ValidateShutdown(std::function<void()> fn) { m_validateShutdownFn = fn; }
        void ValidateCallback(std::function<void(int, int)> fn) { m_validateCallbackFn = fn; }

        void OnQueue(async_task_t* task)
        {
            if (m_validateQueueFn)
                m_validateQueueFn(task);
        }

        bool OnCancel(const char* taskId)
        {
            if (m_validateCancelFn)
                m_validateCancelFn(taskId);
            return !m_shouldExecute;
        }

        void OnShutdown()
        {
            if (m_validateShutdownFn)
                m_validateShutdownFn();
        }

        void Callback(int param1, int param2)
        {
            if (m_validateCallbackFn)
                m_validateCallbackFn(param1, param2);
        }

    private:
        bool m_shouldExecute = false;
        std::function<void(async_task_t*)> m_validateQueueFn;
        std::function<void(const char*)> m_validateCancelFn;
        std::function<void()> m_validateShutdownFn;
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

void EVTSDK_LIBABI_CDECL OnTaskQueue(async_task_t* task, task_callback_fn_t callback)
{
    s_testHelper->OnQueue(task);

    if (s_testHelper->ShouldExecute())
        callback(task->id);
}

bool EVTSDK_LIBABI_CDECL OnTaskCancel(const char* taskId)
{
    return s_testHelper->OnCancel(taskId);
}

void EVTSDK_LIBABI_CDECL OnTaskShutdown()
{
    s_testHelper->OnShutdown();
}

TEST(WorkerThreadCAPITests, Execute)
{
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnTaskQueue, &OnTaskCancel, &OnTaskShutdown);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(true);

    // Validate C++ -> C transformation of worker thread item
    bool wasQueued = false;
    testHelper->ValidateQueue([&wasQueued](async_task_t* task) {
        wasQueued = true;
        EXPECT_EQ(task->delayMs, 0);
        EXPECT_TRUE(std::string(task->typeName).find("TestHelper") != string::npos);
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
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnTaskQueue, &OnTaskCancel, &OnTaskShutdown);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(true);

    // Validate C++ -> C transformation of worker thread item
    bool wasQueued = false;
    testHelper->ValidateQueue([&wasQueued](async_task_t* task) {
        wasQueued = true;
        EXPECT_EQ(task->delayMs, 100);
        EXPECT_TRUE(std::string(task->typeName).find("TestHelper") != string::npos);
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
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnTaskQueue, &OnTaskCancel, &OnTaskShutdown);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(false);

    // Validate C++ -> C transformation of worker thread item
    string taskIdStr;
    bool wasQueued = false;
    testHelper->ValidateQueue([&wasQueued, &taskIdStr](async_task_t* task) {
        wasQueued = true;
        taskIdStr = task->id;
        EXPECT_EQ(task->delayMs, 100);
        EXPECT_TRUE(std::string(task->typeName).find("TestHelper") != string::npos);
    });

    // Validate cancellation
    bool wasCancelled = false;
    testHelper->ValidateCancel([&wasCancelled, &taskIdStr](const char* taskId) {
        wasCancelled = true;
        EXPECT_EQ(taskIdStr, taskId);
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

TEST(WorkerThreadCAPITests, Shutdown)
{
    auto workerThread = std::make_shared<WorkerThread_CAPI>(&OnTaskQueue, &OnTaskCancel, &OnTaskShutdown);

    AutoTestHelper testHelper;

    // Validate C++ -> C transformation of worker thread item
    bool wasShutdown = false;
    testHelper->ValidateShutdown([&wasShutdown]() {
        wasShutdown = true;
    });

    workerThread->join();
    EXPECT_EQ(wasShutdown, true);
}
