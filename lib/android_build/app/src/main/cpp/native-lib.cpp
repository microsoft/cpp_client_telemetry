#include "gtest/gtest.h"
#include <android/log.h>
#include <jni.h>
#include <string>
#include <jni/RunTests.hpp>
#include "jni/JniConvertors.hpp"

#include "LogManager.hpp"
#include "api/LogManagerImpl.hpp"

LOGMANAGER_INSTANCE

JavaVM *RunTests::javaVm = nullptr;

class AndroidLogger: public ::testing::EmptyTestEventListener {
  jobject m_logger = nullptr;
  JNIEnv *m_env = nullptr;
  constexpr static bool immediateJavaLogging = false;

 public:
  AndroidLogger(JNIEnv *env, jobject logger) :
      m_logger(logger), m_env(env) {
  }
  void
  OnTestPartResult(const ::testing::TestPartResult &test_part_result) override {
      int prio = ANDROID_LOG_INFO;
      const char *sf_string = "SUCCESS";
      if (test_part_result.failed()) {
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
      if (immediateJavaLogging && m_logger && m_env
          && test_part_result.failed()) {
          auto logger_class = m_env->GetObjectClass(m_logger);
          if (!logger_class) {
              return;
          }

          auto method_id = m_env->GetMethodID(
              logger_class,
              "log_failure",
              "(Ljava/lang/String;ILjava/lang/String;)V");
          if (!method_id) {
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

  void OnTestStart(const ::testing::TestInfo &test_info) override {
      auto param = test_info.value_param();
      __android_log_print(
          ANDROID_LOG_INFO,
          "MAE",
          "Start %s.%s\n",
          test_info.test_case_name(),
          test_info.name());
  }

  void OnTestEnd(const ::testing::TestInfo &test_info) override {
      __android_log_print(
          ANDROID_LOG_INFO,
          "MAE",
          "End %s.%s: %s\n",
          test_info.test_case_name(),
          test_info.name(),
          test_info.result()->Failed() ? "FAIL" : "OK");
  }

  void OnTestProgramEnd(const ::testing::UnitTest &unit) override {
      __android_log_print(ANDROID_LOG_INFO,
                          "MAE",
                          "%zu dead loggers",
                          LogManagerImpl::GetDeadLoggerCount());
      __android_log_print(
          ANDROID_LOG_INFO,
          "MAE",
          "End tests: %d success, %d fail, %d total",
          unit.successful_test_count(),
          unit.failed_test_count(),
          unit.total_test_count());
  }
};

int RunTests::run_all_tests(JNIEnv *env, jobject java_logger) {
    int argc = 2;
    char command_name[] = "maesdk-test";
    char filter[] = "--gtest_filter=*";
    char *argv[] = {command_name, filter};

    env->GetJavaVM(&javaVm);

    ::testing::InitGoogleTest(&argc, argv);
    ::testing::TestEventListeners &listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new AndroidLogger(env, java_logger));
    auto logger = Microsoft::Applications::Events::LogManager::Initialize(
        "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
    return RUN_ALL_TESTS();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_microsoft_applications_events_maesdktest_MainActivity_stringFromJNI(
    JNIEnv *env,
    jobject /* this */,
    jstring path) {
    auto result = RunTests::run_all_tests(env, path);
    std::string hello = std::to_string(result);
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL

Java_com_microsoft_applications_events_maesdktest_TestStub_runNativeTests(
    JNIEnv *env,
    jobject /* stub */,
    jobject logger) {
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

jlong nativeSample(size_t eventCount, ILogger * logger) {
    auto then = std::chrono::steady_clock::now();
    for (auto j = eventCount; j > 0; --j) {
        logger->LogEvent("StatisticsTestEvent");
    }
    auto sample = std::chrono::steady_clock::now() - then;
    return sample.count();
}

jlong javaSample(
    JNIEnv *env,
    size_t eventCount,
    jclass generatorClass,
    jmethodID logEventsId,
    jobject javaLogger) {
    auto then = std::chrono::steady_clock::now();
    env->CallStaticVoidMethod(generatorClass,
                              logEventsId,
                              javaLogger,
                              static_cast<jlong>(eventCount));
    auto sample = std::chrono::steady_clock::now() - then;
    return sample.count();
}

extern "C"
JNIEXPORT jlongArray JNICALL
Java_com_microsoft_applications_events_maesdktest_TestStub_00024CollectStats_generateStatistics(
    JNIEnv *env,
    jobject thiz,
    jboolean fromJava) {
    constexpr jlong sampleCount = 1000;
    constexpr size_t eventCount = 5000;

    auto myClass = env->GetObjectClass(thiz);
    auto statMethod = env->GetMethodID(myClass, "showIteration", "(ZJJD)V");
    static ILogger *logger = nullptr;
    if (!logger) {
        logger = Microsoft::Applications::Events::LogManager::Initialize(
            "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
    }
    std::vector<jlong> durations;
    double total = 0.0;
    if (!fromJava) {
        // warm things up
        for (auto i = eventCount; i > 0; --i) {
            logger->LogEvent("StatisticsTestEvent");
        }
        for (auto i = sampleCount; i > 0; --i) {
            auto n = nativeSample(eventCount, logger);
            durations.push_back(n);
            total += n;
            env->CallVoidMethod(thiz, statMethod, fromJava, 1 + sampleCount - i, sampleCount, total / durations.size());
            LogManager::Flush();
        }
    } else {
        jobject javaLogger = nullptr;
        auto generatorClass = env->FindClass(
            "com/microsoft/applications/events/maesdktest/EventGenerator");
        auto logEventsId = env->GetStaticMethodID(generatorClass,
                                                  "logEvents",
                                                  "(Lcom/microsoft/applications/events/ILogger;J)V");
        env->CallStaticVoidMethod(generatorClass, logEventsId, javaLogger,
                                  static_cast<jlong>(eventCount));
        for (auto i = sampleCount; i > 0; --i) {
            auto n = javaSample(env, eventCount, generatorClass, logEventsId, javaLogger);
            durations.push_back(n);
            total += n;
            env->CallVoidMethod(thiz, statMethod, fromJava, 1 + sampleCount - i, sampleCount, total / durations.size());
            LogManager::Flush();
        }
    }
    auto results = env->NewLongArray(sampleCount);
    env->SetLongArrayRegion(results, 0, sampleCount, durations.data());
    return results;
}

extern "C"
JNIEXPORT jlongArray JNICALL
Java_com_microsoft_applications_events_maesdktest_TestStub_00024CollectStats_generatePairedObservation(
    JNIEnv *env,
    jobject thiz) {
    constexpr jlong sampleCount = 1000;
    constexpr size_t eventCount = 5000;
    jobject javaLogger = nullptr;

    std::vector<jlong> nativeSamples;
    std::vector<jlong> javaSamples;
    nativeSamples.reserve(sampleCount);
    javaSamples.reserve(sampleCount);
    static ILogger *logger = nullptr;
    if (!logger) {
        logger = Microsoft::Applications::Events::LogManager::Initialize(
            "0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
    }

    auto myClass = env->GetObjectClass(thiz);
    auto statMethod = env->GetMethodID(myClass, "showIteration", "(ZJJD)V");
    auto generatorClass = env->FindClass(
        "com/microsoft/applications/events/maesdktest/EventGenerator");
    auto logEventsId = env->GetStaticMethodID(generatorClass,
                                              "logEvents",
                                              "(Lcom/microsoft/applications/events/ILogger;J)V");
    env->CallStaticVoidMethod(generatorClass, logEventsId, javaLogger,
                              static_cast<jlong>(eventCount));
    double total = 0.0;
    for (size_t i = sampleCount; i > 0; --i) {
        auto n = nativeSample(eventCount, logger);
        auto j = javaSample(env, eventCount, generatorClass, logEventsId, javaLogger);
        nativeSamples.push_back(n);
        javaSamples.push_back(j);
        total += n;
        env->CallVoidMethod(thiz, statMethod, false, 1 + sampleCount - i, sampleCount, total / nativeSamples.size());
        LogManager::Flush();
    }
    auto results = env->NewLongArray(2 * sampleCount);
    env->SetLongArrayRegion(results, 0, sampleCount, nativeSamples.data());
    env->SetLongArrayRegion(results, sampleCount, sampleCount, javaSamples.data());
    return results;
}