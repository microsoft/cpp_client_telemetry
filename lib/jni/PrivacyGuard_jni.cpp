//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "JniConvertors.hpp"
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "PrivacyGuardState.hpp"
#include "WrapperLogManager.hpp"

using namespace MAT;

extern "C"
{
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeInitializePrivacyGuardWithoutCommonDataContext(
        JNIEnv *env, jclass /* this */, jlong iLoggerNativePtr) {
    if (IsPrivacyGuardInstanceInitialized()) {
        return false;
    }

    auto pgInstance = GetOrCreatePrivacyGuardInstance(reinterpret_cast<ILogger *>(iLoggerNativePtr));

    WrapperLogManager::GetInstance()->SetDataInspector(GetOrCreatePrivacyGuardInstance(reinterpret_cast<ILogger*>(iLoggerNativePtr)));
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
    if (IsPrivacyGuardInstanceInitialized()) {
        return false;
    }
    auto pgInstance = GetOrCreatePrivacyGuardInstanceWithDataContext(env,
                                                                     reinterpret_cast<ILogger *>(iLoggerNativePtr),
                                                                     domainName,
                                                                     machineName,
                                                                     userName,
                                                                     userAlias,
                                                                     ipAddresses,
                                                                     languageIdentifiers,
                                                                     machineIds,
                                                                     outOfScopeIdentifiers);

    WrapperLogManager::GetInstance()->SetDataInspector(pgInstance);
    return true;
}

JNIEXPORT jboolean JNICALL
        Java_com_microsoft_applications_events_PrivacyGuard_uninitializePrivacyGuard(JNIEnv *env, jclass /*this*/)
{
    auto pgInstance = GetPrivacyGuardInstance();
    if (pgInstance == nullptr) {
        return false;
    }

    WrapperLogManager::GetInstance()->RemoveDataInspector(pgInstance->GetName());
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_setEnabled(JNIEnv *env, jclass /*this*/,
                                                               jboolean isEnabled) {
    auto pgInstance = GetPrivacyGuardInstance();
    if (pgInstance == nullptr) {
        return false;
    }

    pgInstance->SetEnabled(static_cast<bool>(isEnabled));
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_isEnabled(JNIEnv *env, jclass /*this*/) {
    return IsPrivacyGuardInstanceInitialized() && GetPrivacyGuardInstance()->IsEnabled();
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

    auto pgInstance = GetPrivacyGuardInstance();
    if (pgInstance == nullptr) {
        return false;
    }

    pgInstance->AppendCommonDataContext(GenerateCommonDataContextObject(env,
                                                                                      domainName,
                                                                                      machineName,
                                                                                      userName,
                                                                                      userAlias,
                                                                                      ipAddresses,
                                                                                      languageIdentifiers,
                                                                                      machineIds,
                                                                                      outOfScopeIdentifiers));

    return true;
}

JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeAddIgnoredConcern(JNIEnv *env,
        jclass /* this */,
        jstring eventName,
        jstring fieldName,
        jint dataConcern) {
    auto pgInstance = GetPrivacyGuardInstance();
    if (pgInstance == nullptr) {
        return;
    }

    auto eventNameStr = JStringToStdString(env, eventName);
    auto fieldNameStr = JStringToStdString(env, fieldName);
    auto dataConcernInt = static_cast<uint8_t>(dataConcern);
    pgInstance->AddIgnoredConcern(eventNameStr, fieldNameStr, static_cast<DataConcernType >(dataConcernInt));
}

}

