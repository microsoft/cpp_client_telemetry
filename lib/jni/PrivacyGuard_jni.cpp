//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "JniConvertors.hpp"
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "WrapperLogManager.hpp"

using namespace MAT;

extern "C"
{
std::shared_ptr<PrivacyGuard> spPrivacyGuard;

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeInitializePrivacyGuardWithoutCommonDataContext(
        JNIEnv *env, jclass /* this */, jlong iLoggerNativePtr) {
    if (spPrivacyGuard != nullptr) {
        return false;
    }

    auto logger = reinterpret_cast<ILogger *>(iLoggerNativePtr);
    spPrivacyGuard = std::make_shared<PrivacyGuard>(logger, nullptr);
    WrapperLogManager::GetInstance()->AddOrUpdateInspector(spPrivacyGuard);
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeInitializePrivacyGuard(
        JNIEnv *env, jclass /* this */, jlong iLoggerNativePtr,
        jstring domainName,
        jstring machineName,
        jstring userName,
        jstring userAlias,
        jobjectArray ipAddresses,
        jobjectArray languageIdentifiers,
        jobjectArray machineIds,
        jobjectArray outOfScopeIdentifiers) {
    if (spPrivacyGuard != nullptr) {
        return false;
    }

    auto logger = reinterpret_cast<ILogger *>(iLoggerNativePtr);
    auto cdc = std::make_unique<CommonDataContexts>(GenerateCommonDataContextObject(env,
                                                                                    domainName,
                                                                                    machineName,
                                                                                    userName,
                                                                                    userAlias,
                                                                                    ipAddresses,
                                                                                    languageIdentifiers,
                                                                                    machineIds,
                                                                                    outOfScopeIdentifiers));

    spPrivacyGuard = std::make_shared<PrivacyGuard>(logger, std::move(cdc));
    WrapperLogManager::GetInstance()->AddOrUpdateInspector(spPrivacyGuard);
    return true;
}

JNIEXPORT jboolean JNICALL
        Java_com_microsoft_applications_events_PrivacyGuard_uninitializePrivacyGuard(JNIEnv *env, jclass /*this*/)
{
    if(spPrivacyGuard == nullptr)
    {
        return false;
    }

    WrapperLogManager::GetInstance()->ClearInspectors();
    spPrivacyGuard.reset();

    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_setEnabled(JNIEnv *env, jclass /*this*/,
                                                               jboolean isEnabled) {
    if (spPrivacyGuard == nullptr) {
        return false;
    }
    spPrivacyGuard->SetEnabled(static_cast<bool>(isEnabled));
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_isEnabled(JNIEnv *env, jclass /*this*/) {
    return spPrivacyGuard != nullptr && spPrivacyGuard->IsEnabled();
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeAppendCommonDataContext(
        JNIEnv *env, jclass /* this */,
        jstring domainName,
        jstring machineName,
        jstring userName,
        jstring userAlias,
        jobjectArray ipAddresses,
        jobjectArray languageIdentifiers,
        jobjectArray machineIds,
        jobjectArray outOfScopeIdentifiers) {
    if (spPrivacyGuard == nullptr) {
        return false;
    }

    auto cdc = std::make_unique<CommonDataContexts>(GenerateCommonDataContextObject(env,
                                                                                    domainName,
                                                                                    machineName,
                                                                                    userName,
                                                                                    userAlias,
                                                                                    ipAddresses,
                                                                                    languageIdentifiers,
                                                                                    machineIds,
                                                                                    outOfScopeIdentifiers));

    spPrivacyGuard->AppendCommonDataContext(std::move(cdc));

    return true;
}

JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeAddIgnoredConcern(JNIEnv *env,
        jclass /* this */,
        jstring eventName,
        jstring fieldName,
        jint dataConcern) {
    if (spPrivacyGuard == nullptr) {
        return;
    }

    auto eventNameStr = JStringToStdString(env, eventName);
    auto fieldNameStr = JStringToStdString(env, fieldName);
    auto dataConcernInt = static_cast<uint8_t>(dataConcern);
    spPrivacyGuard->AddIgnoredConcern(eventNameStr, fieldNameStr, static_cast<DataConcernType >(dataConcernInt));
}

}

