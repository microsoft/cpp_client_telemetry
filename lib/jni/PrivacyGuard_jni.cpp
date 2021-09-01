//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "JniConvertors.hpp"
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "PrivacyGuardHelper.hpp"

using namespace MAT;

CommonDataContext GenerateCommonDataContextObject(JNIEnv* env,
                                                  jstring domainName,
                                                  jstring machineName,
                                                  jobjectArray userNames,
                                                  jobjectArray userAliases,
                                                  jobjectArray ipAddresses,
                                                  jobjectArray languageIdentifiers,
                                                  jobjectArray machineIds,
                                                  jobjectArray outOfScopeIdentifiers)
{
    CommonDataContext cdc;
    if(domainName != nullptr) {
        cdc.DomainName = JStringToStdString(env, domainName);
    }
    if(machineName != nullptr) {
        cdc.MachineName = JStringToStdString(env, machineName);
    }

    cdc.UserNames = ConvertJObjectArrayToStdStringVector(env, userNames);
    cdc.UserAliases = ConvertJObjectArrayToStdStringVector(env, userAliases);
    cdc.IpAddresses = ConvertJObjectArrayToStdStringVector(env, ipAddresses);
    cdc.LanguageIdentifiers = ConvertJObjectArrayToStdStringVector(env, languageIdentifiers);
    cdc.MachineIds = ConvertJObjectArrayToStdStringVector(env, machineIds);
    cdc.OutOfScopeIdentifiers = ConvertJObjectArrayToStdStringVector(env, outOfScopeIdentifiers);
    return cdc;
}

std::shared_ptr<PrivacyGuard> spPrivacyGuard;

std::shared_ptr<PrivacyGuard> PrivacyGuardHelper::GetPrivacyGuardPtr() noexcept
{
    return spPrivacyGuard;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeInitializePrivacyGuardWithoutCommonDataContext(
        JNIEnv *env, jclass /* this */,
        jlong iLoggerNativePtr,
        jstring NotificationEventName,
        jstring SemanticContextEventName,
        jstring SummaryEventName,
        jboolean UseEventFieldPrefix,
        jboolean ScanForUrls) {
    if (spPrivacyGuard != nullptr) {
        return false;
    }

    InitializationConfiguration config(
            reinterpret_cast<ILogger*>(iLoggerNativePtr),
            CommonDataContext{});
    if (NotificationEventName != nullptr) {
        config.NotificationEventName = JStringToStdString(env, NotificationEventName).c_str();
    }

    if (SemanticContextEventName != nullptr) {
        config.SemanticContextNotificationEventName = JStringToStdString(env, SemanticContextEventName).c_str();
    }

    if (SummaryEventName != nullptr) {
        config.SummaryEventName = JStringToStdString(env, SummaryEventName).c_str();
    }

    config.UseEventFieldPrefix = static_cast<bool>(UseEventFieldPrefix);
    config.ScanForUrls = static_cast<bool>(ScanForUrls);

    spPrivacyGuard = std::make_shared<PrivacyGuard>(config);
    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeInitializePrivacyGuard(
        JNIEnv *env, jclass /* this */,
        jlong iLoggerNativePtr,
        jstring NotificationEventName,
        jstring SemanticContextEventName,
        jstring SummaryEventName,
        jboolean UseEventFieldPrefix,
        jboolean ScanForUrls,
        jstring domainName,
        jstring machineName,
        jobjectArray userNames,
        jobjectArray userAliases,
        jobjectArray ipAddresses,
        jobjectArray languageIdentifiers,
        jobjectArray machineIds,
        jobjectArray outOfScopeIdentifiers) {
    if (spPrivacyGuard != nullptr) {
        return false;
    }

    InitializationConfiguration config(
            reinterpret_cast<ILogger *>(iLoggerNativePtr),
            GenerateCommonDataContextObject(env,
                                            domainName,
                                            machineName,
                                            userNames,
                                            userAliases,
                                            ipAddresses,
                                            languageIdentifiers,
                                            machineIds,
                                            outOfScopeIdentifiers));

    if (NotificationEventName != NULL) {
        config.NotificationEventName = JStringToStdString(env, NotificationEventName).c_str();
    }

    if (SemanticContextEventName != NULL) {
        config.SemanticContextNotificationEventName = JStringToStdString(env, SemanticContextEventName).c_str();
    }

    if (SummaryEventName != NULL) {
        config.SummaryEventName = JStringToStdString(env, SummaryEventName).c_str();
    }

    config.UseEventFieldPrefix = static_cast<bool>(UseEventFieldPrefix);
    config.ScanForUrls = static_cast<bool>(ScanForUrls);

    spPrivacyGuard = std::make_shared<PrivacyGuard>(config);
    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_uninitialize(const JNIEnv *env, jclass /*this*/)
{
    if(spPrivacyGuard == nullptr)
    {
        return false;
    }

    spPrivacyGuard.reset();

    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_setEnabled(const JNIEnv *env, jclass /*this*/, jboolean isEnabled) {
    if (spPrivacyGuard == nullptr) {
        return false;
    }
    spPrivacyGuard->SetEnabled(static_cast<bool>(isEnabled));
    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_isEnabled(const JNIEnv *env, jclass /*this*/) {
    return spPrivacyGuard != nullptr && spPrivacyGuard->IsEnabled();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeAppendCommonDataContext(
        JNIEnv *env, jclass /* this */,
        jstring domainName,
        jstring machineName,
        jobjectArray userNames,
        jobjectArray userAliases,
        jobjectArray ipAddresses,
        jobjectArray languageIdentifiers,
        jobjectArray machineIds,
        jobjectArray outOfScopeIdentifiers) {
    if (spPrivacyGuard == nullptr) {
        return false;
    }

    spPrivacyGuard->AppendCommonDataContext(GenerateCommonDataContextObject(env,
                                                                            domainName,
                                                                            machineName,
                                                                            userNames,
                                                                            userAliases,
                                                                            ipAddresses,
                                                                            languageIdentifiers,
                                                                            machineIds,
                                                                            outOfScopeIdentifiers));

    return true;
}

extern "C"
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

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_isInitialized(const JNIEnv *env, jclass/* this */){
    return spPrivacyGuard != nullptr;
}

