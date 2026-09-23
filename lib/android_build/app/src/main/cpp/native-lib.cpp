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
#include "config/RuntimeConfig_Default.hpp"
#include "http/HttpClient_Android.hpp"
#include "offline/OfflineStorage_Room.hpp"
#include "pal/PAL.hpp"

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
    // Java HTTP callbacks target the AAR's shared SDK, not this test binary's
    // private static SDK copy. Exercise that transport through instrumentation.
    char filter[] = "--gtest_filter=-HttpClientTests.*";
    char* argv[] = {command_name, filter};
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::TestEventListeners& listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new AndroidLogger(env, java_logger));
    return RUN_ALL_TESTS();
}

extern "C" JNIEXPORT jint JNICALL

Java_com_microsoft_applications_events_maesdktest_TestStub_runNativeTests(
    JNIEnv* env,
    jobject /* stub */,
    jobject logger,
    jobject http_client,
    jobject app_context,
    jstring cache_file_path)
{
    auto path = env->GetStringUTFChars(cache_file_path, nullptr);
    Microsoft::Applications::Events::HttpClient_Android::SetCacheFilePath(path);
    env->ReleaseStringUTFChars(cache_file_path, path);
    Microsoft::Applications::Events::HttpClient_Android::CreateClientInstance(env, http_client);
    Microsoft::Applications::Events::OfflineStorage_Room::ConnectJVM(env, app_context);

    JavaVM* java_vm = nullptr;
    env->GetJavaVM(&java_vm);
    Microsoft::Applications::Events::ILogConfiguration pal_config;
    pal_config[CFG_PTR_ANDROID_JVM] = static_cast<void*>(java_vm);
    pal_config[CFG_JOBJECT_ANDROID_ACTIVITY] = reinterpret_cast<void*>(app_context);
    Microsoft::Applications::Events::RuntimeConfig_Default runtime_config(pal_config);
    PAL::GetPAL().initialize(runtime_config);
    const int result = RunTests::run_all_tests(env, logger);
    PAL::GetPAL().shutdown();
    return result;
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
