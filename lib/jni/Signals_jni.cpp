//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "JniConvertors.hpp"
#include "modules/signals/Signals.hpp"
#include "SignalsHelper.hpp"
#include "WrapperLogManager.hpp"

using namespace MAT;


std::shared_ptr<IDataInspector> spDataInspector;

std::shared_ptr<IDataInspector> SignalsHelper::GetSignalsInspector() noexcept
{
    return spDataInspector;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Signals_sendSignal(JNIEnv *env,
                                                          jclass clazz,
                                                          jlong nativeLoggerPtr,
                                                          jstring signal_item_json) {
    jboolean isCopy = true;
    if (signal_item_json == nullptr) {
        return false;
    }
    const char *signalItemJson = (env)->GetStringUTFChars(signal_item_json, &isCopy);
    if (signalItemJson == nullptr) {
        // GetStringUTFChars returned null (e.g. OOM), which leaves a pending Java
        // exception. Clear it so the caller observes a clean `false` return instead
        // of the exception being thrown at the Java call site. ExceptionClear is one
        // of the few JNI calls that is safe to make with an exception in flight.
        env->ExceptionClear();
        return false;
    }

    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    EventProperties eventProperties = Signals::CreateEventProperties(signalItemJson);
    env->ReleaseStringUTFChars(signal_item_json, signalItemJson);
    logger->LogEvent(eventProperties);
    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Signals_isInitialized(JNIEnv *env, jclass clazz) {
    return spDataInspector != nullptr;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Signals_nativeInitialize(JNIEnv *env, jclass clazz,
                                                                jstring base_url,
                                                                jint timeout_ms,
                                                                jint retry_times,
                                                                jint retry_time_to_wait,
                                                                jintArray retry_status_codes) {
    if (spDataInspector != nullptr) {
        return false;
    }

    SubstrateSignalsConfiguration config;

    jboolean isCopy = true;
    if (base_url != nullptr) {
        const char *convertedValue = (env)->GetStringUTFChars(base_url, &isCopy);
        if (convertedValue == nullptr) {
            // GetStringUTFChars failed (e.g. OOM), leaving a pending Java exception.
            // Clear it so the caller observes a clean `false` instead of the
            // exception being thrown at the Java call site. ExceptionClear is safe
            // to call with an exception in flight, and we make no other JNI calls
            // before returning.
            env->ExceptionClear();
            return false;
        }
        if (strlen(convertedValue) > 0) {
            config.ServiceRequestConfig.BaseUrl = convertedValue;
        }
        env->ReleaseStringUTFChars(base_url, convertedValue);
    }

    config.ServiceRequestConfig.TimeoutMs = reinterpret_cast<int>(timeout_ms);
    config.ServiceRequestConfig.RetryTimes = reinterpret_cast<int>(retry_times);
    config.ServiceRequestConfig.RetryTimesToWait = reinterpret_cast<int>(retry_time_to_wait);

    jsize size = env->GetArrayLength(retry_status_codes);
    std::vector<int> retryStatusCodes(size);
    env->GetIntArrayRegion(retry_status_codes, jsize{0}, size, &retryStatusCodes[0] );
    config.ServiceRequestConfig.RetryStatusCodes = std::vector<int64_t>(retryStatusCodes.begin(), retryStatusCodes.end());

    spDataInspector = Signals::CreateSignalsEventInspector(nullptr, config);
    return true;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Signals_nativeUnitialize(JNIEnv *env, jclass clazz) {
    if (spDataInspector == nullptr) {
        return false;
    }

    spDataInspector.reset();
    return true;
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Signals_isEnabled(JNIEnv *env, jclass clazz) {
    if (spDataInspector == nullptr) {
        return false;
    }

    return spDataInspector.get()->IsEnabled();
}
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_Signals_setEnabled(JNIEnv *env, jclass clazz,
                                                          jboolean enabled) {
    if (spDataInspector == nullptr) {
        return false;
    }

    spDataInspector->SetEnabled(static_cast<bool>(enabled));
    return true;
}
