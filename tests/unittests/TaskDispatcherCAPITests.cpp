//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"

#include "pal/TaskDispatcher.hpp"
#include "pal/TaskDispatcher_CAPI.hpp"
#include "pal/typename.hpp"
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

        void SetQueueValidation(std::function<void(evt_task_t*)> fn)
        {
            m_validateQueueFn = fn;
        }
        void SetCancelValidation(std::function<void(const char*)> fn) { m_validateCancelFn = fn; }
        void SetJoinValidation(std::function<void()> fn) { m_validateJoinFn = fn; }
        void SetCallbackValidation(std::function<void(int, int)> fn) { m_validateCallbackFn = fn; }

        void OnQueue(evt_task_t* task)
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
     std::function<void(evt_task_t*)> m_validateQueueFn;
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

void EVTSDK_LIBABI_CDECL OnTaskDispatcherQueue(evt_task_t* task, task_callback_fn_t callback)
{
    s_testHelper->OnQueue(task);

    if (s_testHelper->ShouldExecute())
        callback(task->id);
}

bool EVTSDK_LIBABI_CDECL OnTaskDispatcherCancel(const char* taskId)
{
    return s_testHelper->OnCancel(taskId);
}

void EVTSDK_LIBABI_CDECL OnTaskDispatcherJoin()
{
    s_testHelper->OnJoin();
}

void CheckTaskTypeNameIsExpectedOrEmptyIfRTTIIsEnabled(evt_task_t* task) noexcept
{
   std::string typeName { task->typeName };
#if HAS_RTTI
   EXPECT_TRUE(typeName.find("TestHelper") != string::npos);
#else
   EXPECT_TRUE(typeName.empty());
#endif // HAS_RTTI
}

TEST(TaskDispatcherCAPITests, Execute)
{
    TaskDispatcher_CAPI taskDispatcher(&OnTaskDispatcherQueue, &OnTaskDispatcherCancel, &OnTaskDispatcherJoin);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(true);

    // Validate C++ -> C transformation of task
    bool wasQueued = false;
    testHelper->SetQueueValidation([&wasQueued](evt_task_t* task) {
        wasQueued = true;
        EXPECT_EQ(task->delayMs, 0);
        CheckTaskTypeNameIsExpectedOrEmptyIfRTTIIsEnabled(task);
    });

    // Validate callback execution
    bool wasExecuted = false;
    testHelper->SetCallbackValidation([&wasExecuted](int param1, int param2) {
        wasExecuted = true;
        EXPECT_EQ(param1, 10);
        EXPECT_EQ(param2, 20);
    });

    dispatchTask(&taskDispatcher, testHelper.get(), &TestHelper::Callback, 10 /*param1*/, 20 /*param2*/);

    EXPECT_EQ(wasQueued, true);
    EXPECT_EQ(wasExecuted, true);
}

TEST(TaskDispatcherCAPITests, Schedule)
{
    TaskDispatcher_CAPI taskDispatcher(&OnTaskDispatcherQueue, &OnTaskDispatcherCancel, &OnTaskDispatcherJoin);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(true);

    // Validate C++ -> C transformation of task
    bool wasQueued = false;
    testHelper->SetQueueValidation([&wasQueued](evt_task_t* task) {
        wasQueued = true;
        EXPECT_NE(task->delayMs, 0);
        CheckTaskTypeNameIsExpectedOrEmptyIfRTTIIsEnabled(task);
    });

    // Validate callback execution
    bool wasExecuted = false;
    testHelper->SetCallbackValidation([&wasExecuted](int param1, int param2) {
        wasExecuted = true;
        EXPECT_EQ(param1, 10);
        EXPECT_EQ(param2, 20);
    });

    scheduleTask(&taskDispatcher, 100 /*delayMs*/, testHelper.get(), &TestHelper::Callback, 10 /*param1*/, 20 /*param2*/);

    EXPECT_EQ(wasQueued, true);
    EXPECT_EQ(wasExecuted, true);
}

TEST(TaskDispatcherCAPITests, Cancel)
{
    TaskDispatcher_CAPI taskDispatcher(&OnTaskDispatcherQueue, &OnTaskDispatcherCancel, &OnTaskDispatcherJoin);

    AutoTestHelper testHelper;
    testHelper->SetShouldExecute(false);

    // Validate C++ -> C transformation of task
    string taskIdStr;
    bool wasQueued = false;
    testHelper->SetQueueValidation([&wasQueued, &taskIdStr](evt_task_t* task) {
        wasQueued = true;
        taskIdStr = task->id;
        EXPECT_NE(task->delayMs, 0);
        CheckTaskTypeNameIsExpectedOrEmptyIfRTTIIsEnabled(task);
    });

    // Validate cancellation
    bool wasCancelled = false;
    testHelper->SetCancelValidation([&wasCancelled, &taskIdStr](const char* taskId) {
        wasCancelled = true;
        EXPECT_EQ(taskIdStr, taskId);
    });

    // Validate callback execution
    testHelper->SetCallbackValidation([](int /*param1*/, int /*param2*/) {
        FAIL() << "Callback should not have been executed";
    });

    auto handle = scheduleTask(&taskDispatcher, 100 /*delayMs*/, testHelper.get(), &TestHelper::Callback, 10 /*param1*/, 20 /*param2*/);
    handle.Cancel();

    EXPECT_EQ(wasQueued, true);
    EXPECT_EQ(wasCancelled, true);
}

TEST(TaskDispatcherCAPITests, Join)
{
    TaskDispatcher_CAPI taskDispatcher(&OnTaskDispatcherQueue, &OnTaskDispatcherCancel, &OnTaskDispatcherJoin);

    AutoTestHelper testHelper;

    // Validate C++ -> C transformation of task
    bool wasJoined = false;
    testHelper->SetJoinValidation([&wasJoined]() {
        wasJoined = true;
    });

    taskDispatcher.Join();
    EXPECT_EQ(wasJoined, true);
}

