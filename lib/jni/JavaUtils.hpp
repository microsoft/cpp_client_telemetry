// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <android/log.h>
#include <jni.h>
#include <string>
#include <aria/Version.hpp>


#define LOG_TAG "TelemetryJNI"

#ifdef DEBUG_LOG
    #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
    #define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
    #define LOGD(...)
    #define LOGI(...)
    #define LOGW(...)
    #define LOGE(...)
#endif


namespace ARIASDK_NS_BEGIN {


std::string JStringToStdString(JNIEnv* env, jstring jstr);


} ARIASDK_NS_END
