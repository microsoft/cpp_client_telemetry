#include <jni.h>
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "JniConvertors.hpp"

using namespace MAT;

extern "C"
{
std::shared_ptr<PrivacyGuard> spPrivacyGuard;

JNIEXPORT void JNICALL
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
        return;
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
}

JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeSetEnabled(JNIEnv *env, jclass /*this*/,
                                                                     jboolean isEnabled) {
    if (spPrivacyGuard != nullptr) {
        spPrivacyGuard->SetEnabled(static_cast<bool>(isEnabled));
    }
}

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeIsEnabled(JNIEnv *env, jclass /*this*/) {
    return spPrivacyGuard != nullptr && spPrivacyGuard->IsEnabled();
}

JNIEXPORT void JNICALL
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
        return;
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
}

JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_PrivacyGuard_nativeAddIgnoredConcern(JNIEnv *env,
                                                                            jclass /* this */,
                                                                            jstring eventName,
                                                                            jstring fieldName,
                                                                            jint dataConcern) {
    if(spPrivacyGuard == nullptr)
    {
        return;
    }

    auto eventNameStr = JStringToStdString(env, eventName);
    auto fieldNameStr = JStringToStdString(env, fieldName);
    auto dataConcernInt = static_cast<uint8_t>(dataConcern);
    spPrivacyGuard->AddIgnoredConcern(eventNameStr, fieldNameStr, static_cast<DataConcernType >(dataConcernInt));
}

}