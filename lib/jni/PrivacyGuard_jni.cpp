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
    if (PrivacyGuardState::isPrivacyGuardInstanceInitialized()) {
        return false;
    }

    InitializationConfiguration config;
    config.LoggerInstance = reinterpret_cast<ILogger*>(iLoggerNativePtr);
    auto spPrivacyGuard = std::make_shared<PrivacyGuard>(config);
    PrivacyGuardState::setPrivacyGuardInstance((spPrivacyGuard));
    WrapperLogManager::GetInstance()->SetDataInspector(spPrivacyGuard);
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
    if (PrivacyGuardState::isPrivacyGuardInstanceInitialized()) {
        return false;
    }

    InitializationConfiguration config;
    config.CommonContext = PrivacyGuardState::GenerateCommonDataContextObject(env,
                                                           domainName,
                                                           machineName,
                                                           userName,
                                                           userAlias,
                                                           ipAddresses,
                                                           languageIdentifiers,
                                                           machineIds,
                                                           outOfScopeIdentifiers);

    config.LoggerInstance = reinterpret_cast<ILogger *>(iLoggerNativePtr);
    auto spPrivacyGuard = std::make_shared<PrivacyGuard>(config);
    PrivacyGuardState::setPrivacyGuardInstance((spPrivacyGuard));
    WrapperLogManager::GetInstance()->SetDataInspector(spPrivacyGuard);
    return true;
}

JNIEXPORT jboolean JNICALL
        Java_com_microsoft_applications_events_PrivacyGuard_uninitializePrivacyGuard(JNIEnv *env, jclass /*this*/)
{
    if(!PrivacyGuardState::isPrivacyGuardInstanceInitialized())
    {
        return false;
    }

    WrapperLogManager::GetInstance()->RemoveDataInspector(PrivacyGuardState::getPrivacyGuardInstance()->GetName());
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_setEnabled(JNIEnv *env, jclass /*this*/,
                                                               jboolean isEnabled) {
    if (!PrivacyGuardState::isPrivacyGuardInstanceInitialized()) {
        return false;
    }
    PrivacyGuardState::getPrivacyGuardInstance()->SetEnabled(static_cast<bool>(isEnabled));
    return true;
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_isEnabled(JNIEnv *env, jclass /*this*/) {
    return PrivacyGuardState::isPrivacyGuardInstanceInitialized() && PrivacyGuardState::getPrivacyGuardInstance()->IsEnabled();
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
    if (!PrivacyGuardState::isPrivacyGuardInstanceInitialized()) {
        return false;
    }

    PrivacyGuardState::getPrivacyGuardInstance()->AppendCommonDataContext(PrivacyGuardState::GenerateCommonDataContextObject(env,
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
    if (!PrivacyGuardState::isPrivacyGuardInstanceInitialized()) {
        return;
    }

    auto eventNameStr = JStringToStdString(env, eventName);
    auto fieldNameStr = JStringToStdString(env, fieldName);
    auto dataConcernInt = static_cast<uint8_t>(dataConcern);
    PrivacyGuardState::getPrivacyGuardInstance()->AddIgnoredConcern(eventNameStr, fieldNameStr, static_cast<DataConcernType >(dataConcernInt));
}

}

