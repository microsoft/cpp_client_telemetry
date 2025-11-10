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
        jstring notificationEventName,
        jboolean warningsToSanitization,
        jobjectArray urlDomains,
        jobjectArray emailDomains,
        jint analyzerOptions) {

    if (spSanitizer != nullptr) {
        return false;
    }

    std::vector<std::string> urlDomainsVec;
    std::vector<std::string> emailDomainsVec;
    
    if (urlDomains != nullptr) {
        jsize urlDomainsLength = env->GetArrayLength(urlDomains);
        for (jsize i = 0; i < urlDomainsLength; i++) {
            jstring domain = static_cast<jstring>(env->GetObjectArrayElement(urlDomains, i));
            if (domain != nullptr) {
                urlDomainsVec.push_back(JStringToStdString(env, domain));
                env->DeleteLocalRef(domain);
            }
        }
    }
    
    if (emailDomains != nullptr) {
        jsize emailDomainsLength = env->GetArrayLength(emailDomains);
        for (jsize i = 0; i < emailDomainsLength; i++) {
            jstring domain = static_cast<jstring>(env->GetObjectArrayElement(emailDomains, i));
            if (domain != nullptr) {
                emailDomainsVec.push_back(JStringToStdString(env, domain));
                env->DeleteLocalRef(domain);
            }
        }
    }
    
    SanitizerConfiguration sanitizerConfig(reinterpret_cast<ILogger*>(iLoggerNativePtr), urlDomainsVec, emailDomainsVec, static_cast<size_t>(analyzerOptions));

    if (notificationEventName != nullptr) {
        sanitizerConfig.NotificationEventName = JStringToStdString(env, notificationEventName);
    }

    sanitizerConfig.SetAllWarningsToSanitizations = static_cast<bool>(warningsToSanitization);

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
    if (spSanitizer == nullptr) {
        return false;
    }

    spSanitizer->SetEnabled(static_cast<bool>(enabled));
    return true;
}