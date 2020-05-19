#include "ILogger.hpp"
#include "JniUtils.hpp"

using namespace MAT;

extern "C"
{
JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_EventProperties_nativeIsEventNameValid(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName) {
    auto name = JStringToStdString(env, jstrName);
    EventProperties properties;
    return properties.SetName(name);
}

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_EventProperties_nativeIsTypeValid(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrRecordType) {
    auto recordType = JStringToStdString(env, jstrRecordType);
    EventProperties properties;
    return properties.SetName(recordType);
}

JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_Logger_nativeGetSemanticContext(
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

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetParentContext(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jlong nativeSemanticContextPtr) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto semanticContext = reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr);
    logger->SetParentContext(semanticContext);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogAppLifecycle(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jint jAppLifecycleState,
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogAppLifecycle(static_cast<AppLifecycleState>(jAppLifecycleState), properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogSession(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jint jSessionState,
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogSession(static_cast<SessionState>(jSessionState), properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogEventName(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    logger->LogEvent(JStringToStdString(env, jstrName));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogEventProperties(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogEvent(properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogFailure(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrSignature,
        jstring jstrDetail,
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto signature = JStringToStdString(env, jstrSignature);
    auto detail = JStringToStdString(env, jstrDetail);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogFailure(signature, detail, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogFailureWithCategoryId(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrSignature,
        jstring jstrDetail,
        jstring jstrCategory,
        jstring jstrId,
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    auto signature = JStringToStdString(env, jstrSignature);
    auto detail = JStringToStdString(env, jstrDetail);
    auto category = JStringToStdString(env, jstrCategory);
    auto id = JStringToStdString(env, jstrId);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
                                               jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogFailure(signature, detail, category, id, properties);
}
};