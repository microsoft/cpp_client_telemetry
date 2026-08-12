//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "JniConvertors.hpp"
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "PrivacyGuardHelper.hpp"

#include <mutex>
#include <utility>

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

namespace
{
    std::shared_ptr<PrivacyGuard> spPrivacyGuard;
    std::mutex privacyGuardMutex;

    struct EventNameStorage
    {
        std::string notification;
        std::string semanticContext;
        std::string summary;
    };

    void SetEventNames(
        JNIEnv* env,
        jstring notificationEventName,
        jstring semanticContextEventName,
        jstring summaryEventName,
        EventNameStorage& storage,
        InitializationConfiguration& config)
    {
        if (notificationEventName != nullptr) {
            storage.notification = JStringToStdString(env, notificationEventName);
            config.NotificationEventName = storage.notification.c_str();
        }

        if (semanticContextEventName != nullptr) {
            storage.semanticContext = JStringToStdString(env, semanticContextEventName);
            config.SemanticContextNotificationEventName = storage.semanticContext.c_str();
        }

        if (summaryEventName != nullptr) {
            storage.summary = JStringToStdString(env, summaryEventName);
            config.SummaryEventName = storage.summary.c_str();
        }
    }

    std::shared_ptr<PrivacyGuard> CreatePrivacyGuard(
        const InitializationConfiguration& config,
        std::shared_ptr<EventNameStorage> eventNameStorage)
    {
        // Log managers can retain the guard after JNI uninitialization. Keep the
        // strings backing its raw event-name pointers alive until the last owner
        // releases the guard.
        return std::shared_ptr<PrivacyGuard>(
            new PrivacyGuard(config),
            [eventNameStorage](PrivacyGuard* privacyGuard) {
                (void)eventNameStorage;
                delete privacyGuard;
            });
    }
}

std::shared_ptr<PrivacyGuard> PrivacyGuardHelper::GetPrivacyGuardPtr() noexcept
{
    std::lock_guard<std::mutex> lock(privacyGuardMutex);
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
    std::lock_guard<std::mutex> lock(privacyGuardMutex);
    if (spPrivacyGuard != nullptr) {
        return false;
    }

    InitializationConfiguration config(
            reinterpret_cast<ILogger*>(iLoggerNativePtr),
            CommonDataContext{});
    auto eventNameStorage = std::make_shared<EventNameStorage>();
    SetEventNames(env, NotificationEventName, SemanticContextEventName, SummaryEventName, *eventNameStorage, config);

    config.UseEventFieldPrefix = static_cast<bool>(UseEventFieldPrefix);
    config.ScanForUrls = static_cast<bool>(ScanForUrls);
    config.DisableAdvancedScans = static_cast<bool>(DisableAdvancedScans);
    config.StampEventIKeyForConcerns = static_cast<bool>(StampEventIKeyForConcerns);

    spPrivacyGuard = CreatePrivacyGuard(config, std::move(eventNameStorage));
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
    std::lock_guard<std::mutex> lock(privacyGuardMutex);
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

    auto eventNameStorage = std::make_shared<EventNameStorage>();
    SetEventNames(env, NotificationEventName, SemanticContextEventName, SummaryEventName, *eventNameStorage, config);

    config.UseEventFieldPrefix = static_cast<bool>(UseEventFieldPrefix);
    config.ScanForUrls = static_cast<bool>(ScanForUrls);
    config.DisableAdvancedScans = static_cast<bool>(DisableAdvancedScans);
    config.StampEventIKeyForConcerns = static_cast<bool>(StampEventIKeyForConcerns);

    spPrivacyGuard = CreatePrivacyGuard(config, std::move(eventNameStorage));
    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_uninitialize(const JNIEnv *env, jclass /*this*/)
{
    std::lock_guard<std::mutex> lock(privacyGuardMutex);
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
    auto privacyGuard = PrivacyGuardHelper::GetPrivacyGuardPtr();
    if (privacyGuard == nullptr) {
        return false;
    }
    privacyGuard->SetEnabled(static_cast<bool>(isEnabled));
    return true;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_isEnabled(const JNIEnv *env, jclass /*this*/) {
    auto privacyGuard = PrivacyGuardHelper::GetPrivacyGuardPtr();
    return privacyGuard != nullptr && privacyGuard->IsEnabled();
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
    auto privacyGuard = PrivacyGuardHelper::GetPrivacyGuardPtr();
    if (privacyGuard == nullptr) {
        return false;
    }

    privacyGuard->AppendCommonDataContext(GenerateCommonDataContextObject(env,
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
    auto privacyGuard = PrivacyGuardHelper::GetPrivacyGuardPtr();
    if (privacyGuard == nullptr) {
        return;
    }

    auto eventNameStr = JStringToStdString(env, eventName);
    auto fieldNameStr = JStringToStdString(env, fieldName);
    auto dataConcernInt = static_cast<uint8_t>(dataConcern);
    privacyGuard->AddIgnoredConcern(eventNameStr, fieldNameStr, static_cast<DataConcernType >(dataConcernInt));
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_isInitialized(const JNIEnv *env, jclass/* this */){
    return PrivacyGuardHelper::GetPrivacyGuardPtr() != nullptr;
}
