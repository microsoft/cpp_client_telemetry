//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "ILogger.hpp"
#include "JniConvertors.hpp"

using namespace MAT;

extern "C"
{

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
        // EventProperties
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
        // EventProperties
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
        jobject/* this */,
        jlong nativeLoggerPtr,
        jstring jstrName) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    logger->LogEvent(JStringToStdString(env, jstrName));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogEventProperties(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeLoggerPtr,
        // EventProperties
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
        jobject /* this */,
        jlong nativeLoggerPtr,
        jstring jstrSignature,
        jstring jstrDetail,
        // EventProperties
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
        // EventProperties
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

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogPageView(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrId,
        jstring jstrPageName,
        // EventProperties
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
    auto id = JStringToStdString(env, jstrId);
    auto pageName = JStringToStdString(env, jstrPageName);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogPageView(id, pageName, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogPageViewWithUri(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrId,
        jstring jstrPageName,
        jstring jstrCategory,
        jstring jstrUri,
        jstring jstrReferrerUri,
        // EventProperties
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
    auto id = JStringToStdString(env, jstrId);
    auto pageName = JStringToStdString(env, jstrPageName);
    auto category = JStringToStdString(env, jstrCategory);
    auto uri = JStringToStdString(env, jstrUri);
    auto referrerUri = JStringToStdString(env, jstrReferrerUri);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogPageView(id, pageName, category, uri, referrerUri, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogPageAction(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrPageViewId,
        jint jActionType,
        // EventProperties
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
    auto pageViewId = JStringToStdString(env, jstrPageViewId);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogPageAction(pageViewId, static_cast<ActionType>(jActionType), properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogPageActionData(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        // PageActionData
        jstring jstrPageViewId,
        jint jActionType,
        jint jRawActionType,
        jint jInputDeviceType,
        jstring jstrTargetItemId,
        jstring jstrTargetItemDataSourceName,
        jstring jstrTargetItemDataSourceCategory,
        jstring jstrTargetItemDataSourceCollection,
        jstring jstrTargetItemLayoutContainer,
        jshort jTargetItemLayoutRank,
        jstring jstrDestinationUri,
        // EventProperties
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto pageViewId = JStringToStdString(env, jstrPageViewId);
    PageActionData pageActionData(pageViewId, static_cast<ActionType>(jActionType));
    pageActionData.rawActionType = static_cast<RawActionType>(jRawActionType);
    pageActionData.inputDeviceType = static_cast<InputDeviceType>(jInputDeviceType);
    pageActionData.targetItemId = JStringToStdString(env, jstrTargetItemId);
    pageActionData.targetItemDataSourceName = JStringToStdString(env, jstrTargetItemDataSourceName);
    pageActionData.targetItemDataSourceCategory = JStringToStdString(env, jstrTargetItemDataSourceCategory);
    pageActionData.targetItemDataSourceCollection = JStringToStdString(env, jstrTargetItemDataSourceCollection);
    pageActionData.targetItemLayoutContainer = JStringToStdString(env, jstrTargetItemLayoutContainer);
    pageActionData.targetItemLayoutRank = static_cast<short>(jTargetItemLayoutRank);
    pageActionData.destinationUri = JStringToStdString(env, jstrDestinationUri);

    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);

    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    logger->LogPageAction(pageActionData, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogSampledMetric(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jdouble jValue,
        jstring jstrUnits,
        // EventProperties
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
    auto name = JStringToStdString(env, jstrName);
    auto units = JStringToStdString(env, jstrUnits);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
                                                    jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogSampledMetric(name, static_cast<double>(jValue), units, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogSampledMetricWithObjectId(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jdouble jValue,
        jstring jstrUnits,
        jstring jstrInstanceName,
        jstring jstrObjectClass,
        jstring jstrObjectId,
        // EventProperties
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
    auto name = JStringToStdString(env, jstrName);
    auto units = JStringToStdString(env, jstrUnits);
    auto instanceName = JStringToStdString(env, jstrInstanceName);
    auto objectClass = JStringToStdString(env, jstrObjectClass);
    auto objectId = JStringToStdString(env, jstrObjectId);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
                                                    jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogSampledMetric(name, static_cast<double>(jValue), units, instanceName, objectClass, objectId, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogAggregatedMetric(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jstring jstrName,
        jlong jDuration,
        jlong jCount,
        // EventProperties
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
    auto name = JStringToStdString(env, jstrName);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
                                                    jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    logger->LogAggregatedMetric(name, static_cast<long>(jDuration), static_cast<long>(jCount), properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogAggregatedMetricData(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        // AggregatedMetricData
        jstring jstrName,
        jlong jDuration,
        jlong jCount,
        jstring jstrUnits,
        jstring jstrInstanceName,
        jstring jstrObjectClass,
        jstring jstrObjectId,
        jintArray jAggregateTypeKeys,
        jdoubleArray jAggregateDoubleValues,
        jlongArray jBucketsLongKeys,
        jlongArray jBucketsLongValues,
        // EventProperties
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto name = JStringToStdString(env, jstrName);
    AggregatedMetricData aggregatedMetricData(name, static_cast<long>(jDuration), static_cast<long>(jCount));
    aggregatedMetricData.units = JStringToStdString(env, jstrUnits);
    aggregatedMetricData.instanceName = JStringToStdString(env, jstrInstanceName);
    aggregatedMetricData.objectClass = JStringToStdString(env, jstrObjectClass);
    aggregatedMetricData.objectId = JStringToStdString(env, jstrObjectId);

    jsize aggregatesLen = env->GetArrayLength(jAggregateTypeKeys);
    jint* aggregateTypeKeys = env->GetIntArrayElements(jAggregateTypeKeys, NULL);
    jdouble* aggregateDoubleValues = env->GetDoubleArrayElements(jAggregateDoubleValues, NULL);
    for (int i = 0; i < aggregatesLen; i++) {
        auto key = static_cast<AggregateType>(aggregateTypeKeys[i]);
        auto value = static_cast<double>(aggregateDoubleValues[i]);
        aggregatedMetricData.aggregates[key] = value;
    }

    jsize bucketsLen = env->GetArrayLength(jBucketsLongKeys);
    jlong* bucketsLongKeys = env->GetLongArrayElements(jBucketsLongKeys, NULL);
    jlong* bucketsLongValues = env->GetLongArrayElements(jBucketsLongValues, NULL);
    for (int i = 0; i < bucketsLen; i++) {
        auto key = static_cast<long>(bucketsLongKeys[i]);
        auto value = static_cast<long>(bucketsLongValues[i]);
        aggregatedMetricData.buckets[key] = value;
    }

    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);

    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    logger->LogAggregatedMetric(aggregatedMetricData, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogTrace(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jint jTraceLevel,
        jstring jstrMessage,
        // EventProperties
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    auto message = JStringToStdString(env, jstrMessage);
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    logger->LogTrace(static_cast<TraceLevel>(jTraceLevel), message, properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeLogUserState(
        JNIEnv* env,
        jclass /* this */,
        jlong nativeLoggerPtr,
        jint jUserState,
        jlong jTimeToLiveInMillis,
        // EventProperties
        jstring jstrEventName,
        jstring jstrEventType,
        jint jEventLatency,
        jint jEventPersistence,
        jdouble jEventPopSample,
        jlong jEventPolicyBitflags,
        jlong jTimestampInMillis,
        jobjectArray jEventPropertyStringKey,
        jobjectArray jEventPropertyValue) {
    EventProperties properties = GetEventProperties(env, jstrEventName, jstrEventType, jEventLatency, jEventPersistence,
            jEventPopSample, jEventPolicyBitflags, jTimestampInMillis, jEventPropertyStringKey, jEventPropertyValue);
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    logger->LogUserState(static_cast<UserState>(jUserState), static_cast<long>(jTimeToLiveInMillis), properties);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_Logger_nativeSetLevel(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeLoggerPtr,
        jint jLevel) {
    auto logger = reinterpret_cast<ILogger*>(nativeLoggerPtr);
    logger->SetLevel(static_cast<int>(jLevel));
}

};

