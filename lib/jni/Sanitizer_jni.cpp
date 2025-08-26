//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "JniConvertors.hpp"
#include "modules/sanitizer/Sanitizer.hpp"
#include "SanitizerHelper.hpp"

using namespace MAT;

std::shared_ptr<Sanitizer> spSanitizer;

std::shared_ptr<Sanitizer> SanitizerHelper::GetSanitizerPtr() noexcept
{
    return spSanitizer;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Sanitizer_isInitialized(const JNIEnv *env, jclass/* this */) {

    return spSanitizer != nullptr;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Sanitizer_nativeInitialize(
        JNIEnv *env, jclass /* this */,
        jlong iLoggerNativePtr,
        jstring notificationEventName) {

    if (spSanitizer != nullptr) {
        return false;
    }

    SanitizerConfiguration sanitizerConfig(reinterpret_cast<ILogger*>(iLoggerNativePtr));

    if (notificationEventName != nullptr) {
        sanitizerConfig.NotificationEventName = JStringToStdString(env, notificationEventName).c_str();
    }

    spSanitizer = std::make_shared<Sanitizer>(sanitizerConfig);
    return true;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Sanitizer_uninitialize(const JNIEnv *env, jclass /*this*/) {

    if(spSanitizer == nullptr) {
        return false;
    }

    spSanitizer.reset();

    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Sanitizer_isEnabled(JNIEnv *env, jclass clazz) {

    if (spSanitizer == nullptr) {
        return false;
    }

    return spSanitizer.get()->IsEnabled();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Sanitizer_setEnabled(JNIEnv *env, jclass clazz,
                                                          jboolean enabled) {
    if (spDataInspector == nullptr) {
        return false;
    }

    spSanitizer->SetEnabled(static_cast<bool>(enabled));
    return true;
}