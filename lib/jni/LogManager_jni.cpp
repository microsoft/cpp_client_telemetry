//#include "http/HttpClient_Android.hpp"
#include "LogManagerBase.hpp"
#include "LogManager.hpp"
#include "ILogger.hpp"
#include "JniUtils.hpp"
//#include <algorithm>
//#include <cstdio>
//#include <sstream>
//#include <vector>

//constexpr static auto Tag = "LogManagerBase_jni";
using namespace MAT;
LOGMANAGER_INSTANCE

extern "C"
{

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_initializeWithoutTenantToken(
        JNIEnv* /* env */,
        jclass /* this */) {
    ILogger* logger = Microsoft::Applications::Events::LogManager::Initialize();
    return reinterpret_cast<jlong>(logger);
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_initializeWithTenantToken(
        JNIEnv* env,
        jclass /* this */,
        jstring jTenantToken) {
    auto tenantToken = JStringToStdString(env, jTenantToken);
    ILogger* logger = Microsoft::Applications::Events::LogManager::Initialize(tenantToken);
    return reinterpret_cast<jlong>(logger);
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_flushAndTeardown(
        JNIEnv* /* env */,
        jclass /* this */) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::FlushAndTeardown());
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_uploadNow(
        JNIEnv* /* env */,
        jclass /* this */) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::UploadNow());
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_flush(
        JNIEnv* /* env */,
        jclass /* this */) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::Flush());
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_pauseTransmission(
        JNIEnv* /* env */,
        jclass /* this */) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::PauseTransmission());
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_resumeTransmission(
        JNIEnv* /* env */,
        jclass /* this */) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::ResumeTransmission());
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_setIntTransmitProfile(
        JNIEnv* /* env */,
        jclass /* this */,
        jint jProfile) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetTransmitProfile(
            static_cast<TransmitProfile>(jProfile)));
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_setTransmitProfileString(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrProfile) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetTransmitProfile(JStringToStdString(env, jstrProfile)));
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_loadTransmitProfilesString(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrProfilesJson) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::LoadTransmitProfiles(JStringToStdString(env, jstrProfilesJson)));
}

JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_resetTransmitProfiles(
        JNIEnv* /* env */,
        jclass /* this */) {
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::ResetTransmitProfiles());
}

JNIEXPORT jstring JNICALL Java_com_microsoft_applications_events_LogManager_getTransmitProfileName(
        JNIEnv* env,
        jclass /* this */) {
    std::string profileName = Microsoft::Applications::Events::LogManager::GetTransmitProfileName();
    return static_cast<jstring>(env->NewStringUTF(profileName.c_str()));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetSemanticContext(
        JNIEnv* env,
        jclass /* this */) {
    return reinterpret_cast<jlong>(Microsoft::Applications::Events::LogManager::GetSemanticContext());
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextStringValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind) {
    auto name = JStringToStdString(env, jstrName);
    auto value = JStringToStdString(env, jstrValue);
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetContext(name, value, static_cast<PiiKind>(piiKind)));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextIntValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jint jValue,
        jint piiKind) {
    auto name = JStringToStdString(env, jstrName);
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetContext(name, static_cast<int32_t>(jValue), static_cast<PiiKind>(piiKind)));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextLongValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jlong jValue,
        jint piiKind) {
    auto name = JStringToStdString(env, jstrName);
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetContext(name, static_cast<int64_t>(jValue), static_cast<PiiKind>(piiKind)));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextDoubleValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jdouble jValue,
        jint piiKind) {
    auto name = JStringToStdString(env, jstrName);
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetContext(name, static_cast<double>(jValue), static_cast<PiiKind>(piiKind)));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextBoolValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jboolean jValue,
        jint piiKind) {
    auto name = JStringToStdString(env, jstrName);
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetContext(name, static_cast<bool>(jValue), static_cast<PiiKind>(piiKind)));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextTimeTicksValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jlong jValue,
        jint piiKind) {
    auto name = JStringToStdString(env, jstrName);
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetContext(name, time_ticks_t(static_cast<uint64_t>(jValue)), static_cast<PiiKind>(piiKind)));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextGuidValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind) {
    auto name = JStringToStdString(env, jstrName);
    auto value = JStringToStdString(env, jstrValue);
    return static_cast<jint>(Microsoft::Applications::Events::LogManager::SetContext(name, GUID_t(value.c_str()), static_cast<PiiKind>(piiKind)));
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetLogger(
        JNIEnv* /* env */,
        jclass /* this */){
    ILogger* logger = (Microsoft::Applications::Events::LogManager::GetLogger());
    return reinterpret_cast<jlong>(logger);
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetLoggerWithSource(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrSource){
    auto source = JStringToStdString(env, jstrSource);
    ILogger* logger = (Microsoft::Applications::Events::LogManager::GetLogger(source));
    return reinterpret_cast<jlong>(logger);
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetLoggerWithTenantTokenAndSource(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrTenantToken,
        jstring jstrSource){
    auto tenantToken = JStringToStdString(env, jstrTenantToken);
    auto source = JStringToStdString(env, jstrSource);
    ILogger* logger = (Microsoft::Applications::Events::LogManager::GetLogger(tenantToken, source));
    return reinterpret_cast<jlong>(logger);
}

}