#include "ILogger.hpp"
#include "JniUtils.hpp"

using namespace MAT;

extern "C"
{
JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_Logger_getSemanticContext(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    return reinterpret_cast<jlong>(logger->GetSemanticContext());
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextStringValue(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    auto value = JStringToStdString(env, jstrValue);
    logger->SetContext(name, value, static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextDoubleValue(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jdouble jValue,
        jint piiKind) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    logger->SetContext(name, static_cast<double>(jValue), static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextLongValue(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jlong jValue,
        jint piiKind) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    logger->SetContext(name, static_cast<int64_t>(jValue), static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextIntValue(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jint jValue,
        jint piiKind) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    logger->SetContext(name, static_cast<int32_t>(jValue), static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextBoolValue(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jboolean jValue,
        jint piiKind) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    logger->SetContext(name, static_cast<bool>(jValue), static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextTimeTicksValue(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jlong jValue,
        jint piiKind) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    logger->SetContext(name, time_ticks_t(static_cast<uint64_t>(jValue)), static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextGuidValue(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    auto value = JStringToStdString(env, jstrValue);
    logger->SetContext(name, GUID_t(value.c_str()), static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetContextEventProperty(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jobject jEventProperty) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto name = JStringToStdString(env, jstrName);
    logger->SetContext(name, GetEventProperty(env, jEventProperty));
}

};