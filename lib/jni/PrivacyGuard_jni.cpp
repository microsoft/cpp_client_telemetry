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
        if (env->ExceptionCheck()) {
            return cdc;
        }
    }
    if(machineName != nullptr) {
        cdc.MachineName = JStringToStdString(env, machineName);
        if (env->ExceptionCheck()) {
            return cdc;
        }
    }

    cdc.UserNames = ConvertJObjectArrayToStdStringVector(env, userNames);
    if (env->ExceptionCheck()) {
        return cdc;
    }
    cdc.UserAliases = ConvertJObjectArrayToStdStringVector(env, userAliases);
    if (env->ExceptionCheck()) {
        return cdc;
    }
    cdc.IpAddresses = ConvertJObjectArrayToStdStringVector(env, ipAddresses);
    if (env->ExceptionCheck()) {
        return cdc;
    }
    cdc.LanguageIdentifiers = ConvertJObjectArrayToStdStringVector(env, languageIdentifiers);
    if (env->ExceptionCheck()) {
        return cdc;
    }
    cdc.MachineIds = ConvertJObjectArrayToStdStringVector(env, machineIds);
    if (env->ExceptionCheck()) {
        return cdc;
    }
    cdc.OutOfScopeIdentifiers = ConvertJObjectArrayToStdStringVector(env, outOfScopeIdentifiers);
    return cdc;
}

namespace
{
    // PrivacyGuard borrows its configured event names, so keep their storage with the guard.
    struct PrivacyGuardState
    {
        std::string notificationEventName;
        std::string semanticContextEventName;
        std::string summaryEventName;
        std::unique_ptr<PrivacyGuard> privacyGuard;
    };

    std::shared_ptr<PrivacyGuard> spPrivacyGuard;
}

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
        jboolean ScanForUrls,
        jboolean DisableAdvancedScans,
        jboolean StampEventIKeyForConcerns) {
    if (spPrivacyGuard != nullptr) {
        return false;
    }

    InitializationConfiguration config(
            reinterpret_cast<ILogger*>(iLoggerNativePtr),
            CommonDataContext{});
    auto state = std::make_shared<PrivacyGuardState>();
    if (NotificationEventName != nullptr) {
        state->notificationEventName = JStringToStdString(env, NotificationEventName);
        if (env->ExceptionCheck()) {
            return false;
        }
        config.NotificationEventName = state->notificationEventName.c_str();
    }

    if (SemanticContextEventName != nullptr) {
        state->semanticContextEventName = JStringToStdString(env, SemanticContextEventName);
        if (env->ExceptionCheck()) {
            return false;
        }
        config.SemanticContextNotificationEventName = state->semanticContextEventName.c_str();
    }

    if (SummaryEventName != nullptr) {
        state->summaryEventName = JStringToStdString(env, SummaryEventName);
        if (env->ExceptionCheck()) {
            return false;
        }
        config.SummaryEventName = state->summaryEventName.c_str();
    }

    config.UseEventFieldPrefix = static_cast<bool>(UseEventFieldPrefix);
    config.ScanForUrls = static_cast<bool>(ScanForUrls);
    config.DisableAdvancedScans = static_cast<bool>(DisableAdvancedScans);
    config.StampEventIKeyForConcerns = static_cast<bool>(StampEventIKeyForConcerns);

    state->privacyGuard = std::make_unique<PrivacyGuard>(config);
    spPrivacyGuard = std::shared_ptr<PrivacyGuard>(state, state->privacyGuard.get());
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
        jboolean DisableAdvancedScans,
        jboolean StampEventIKeyForConcerns,
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

    auto commonDataContext = GenerateCommonDataContextObject(env,
                                                            domainName,
                                                            machineName,
                                                            userNames,
                                                            userAliases,
                                                            ipAddresses,
                                                            languageIdentifiers,
                                                            machineIds,
                                                            outOfScopeIdentifiers);
    if (env->ExceptionCheck()) {
        return false;
    }
    InitializationConfiguration config(
            reinterpret_cast<ILogger *>(iLoggerNativePtr),
            commonDataContext);

    auto state = std::make_shared<PrivacyGuardState>();
    if (NotificationEventName != NULL) {
        state->notificationEventName = JStringToStdString(env, NotificationEventName);
        if (env->ExceptionCheck()) {
            return false;
        }
        config.NotificationEventName = state->notificationEventName.c_str();
    }

    if (SemanticContextEventName != NULL) {
        state->semanticContextEventName = JStringToStdString(env, SemanticContextEventName);
        if (env->ExceptionCheck()) {
            return false;
        }
        config.SemanticContextNotificationEventName = state->semanticContextEventName.c_str();
    }

    if (SummaryEventName != NULL) {
        state->summaryEventName = JStringToStdString(env, SummaryEventName);
        if (env->ExceptionCheck()) {
            return false;
        }
        config.SummaryEventName = state->summaryEventName.c_str();
    }

    config.UseEventFieldPrefix = static_cast<bool>(UseEventFieldPrefix);
    config.ScanForUrls = static_cast<bool>(ScanForUrls);
    config.DisableAdvancedScans = static_cast<bool>(DisableAdvancedScans);
    config.StampEventIKeyForConcerns = static_cast<bool>(StampEventIKeyForConcerns);

    state->privacyGuard = std::make_unique<PrivacyGuard>(config);
    spPrivacyGuard = std::shared_ptr<PrivacyGuard>(state, state->privacyGuard.get());
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
