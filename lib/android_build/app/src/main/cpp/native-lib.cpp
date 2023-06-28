//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "gtest/gtest.h"
#include <android/log.h>
#include <jni.h>
#include <string>
#include "jni/JniConvertors.hpp"

#include "LogManager.hpp"
#include "api/LogManagerImpl.hpp"

LOGMANAGER_INSTANCE

class RunTests
{
   public:
    static int run_all_tests(JNIEnv* env, jobject logger);
};

class AndroidLogger : public ::testing::EmptyTestEventListener
{
    jobject m_logger = nullptr;
    JNIEnv* m_env = nullptr;
    constexpr static bool immediateJavaLogging = false;

   public:
    AndroidLogger(JNIEnv* env, jobject logger) :
        m_logger(logger), m_env(env)
    {
    }
    void OnTestPartResult(const ::testing::TestPartResult& test_part_result) override
    {
        int prio = ANDROID_LOG_INFO;
        const char* sf_string = "SUCCESS";
        if (test_part_result.failed())
        {
            prio = ANDROID_LOG_WARN;
            sf_string = "FAILURE";
        }

        __android_log_print(
            prio, "MAE",
            "%s (%s: %d): %s\n",
            sf_string,
            test_part_result.file_name(),
            test_part_result.line_number(),
            test_part_result.summary());
        if (immediateJavaLogging && m_logger && m_env && test_part_result.failed())
        {
            auto logger_class = m_env->GetObjectClass(m_logger);
            if (!logger_class)
            {
                return;
            }

            auto method_id = m_env->GetMethodID(
                logger_class,
                "log_failure",
                "(Ljava/lang/String;ILjava/lang/String;)V");
            if (!method_id)
            {
                return;
            }

            m_env->CallVoidMethod(
                m_logger,
                method_id,
                m_env->NewStringUTF(test_part_result.file_name()),
                test_part_result.line_number(),
                m_env->NewStringUTF(test_part_result.summary()));
        }
    }

    void OnTestStart(const ::testing::TestInfo& test_info) override
    {
        auto param = test_info.value_param();
        __android_log_print(
            ANDROID_LOG_INFO,
            "MAE",
            "Start %s.%s\n",
            test_info.test_case_name(),
            test_info.name());
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override
    {
        __android_log_print(
            ANDROID_LOG_INFO,
            "MAE",
            "End %s.%s: %s\n",
            test_info.test_case_name(),
            test_info.name(),
            test_info.result()->Failed() ? "FAIL" : "OK");
    }

    void OnTestProgramEnd(const ::testing::UnitTest& unit) override
    {
        __android_log_print(ANDROID_LOG_INFO,
                            "MAE",
                            "%zu dead loggers", LogManagerImpl::GetDeadLoggerCount());
        __android_log_print(
            ANDROID_LOG_INFO,
            "MAE",
            "End tests: %d success, %d fail, %d total",
            unit.successful_test_count(),
            unit.failed_test_count(),
            unit.total_test_count());
    }
};

int RunTests::run_all_tests(JNIEnv* env, jobject java_logger)
{
    int argc = 2;
    char command_name[] = "maesdk-test";
    char filter[] = "--gtest_filter=*";
    char* argv[] = {command_name, filter};
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::TestEventListeners& listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new AndroidLogger(env, java_logger));
    auto logger = Microsoft::Applications::Events::LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
    return RUN_ALL_TESTS();
}

extern "C" JNIEXPORT jint JNICALL

Java_com_microsoft_applications_events_maesdktest_TestStub_runNativeTests(
    JNIEnv* env,
    jobject /* stub */,
    jobject logger)
{
    return RunTests::run_all_tests(env, logger);
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_maesdktest_SDKUnitNativeTest_nativeGetPiiType(
    JNIEnv *env,
    jobject thiz,
    jobject jProperty) {
    auto property = GetEventProperty(env, jProperty);
    return static_cast<int>(property.piiKind);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_maesdktest_SDKUnitNativeTest_nativeGetDataCategory(
    JNIEnv *env,
    jobject thiz,
    jobject jProperty) {
    auto property = GetEventProperty(env, jProperty);
    return static_cast<int>(property.dataCategory);
}

