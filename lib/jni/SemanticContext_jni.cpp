//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "JniConvertors.hpp"
#include "ISemanticContext.hpp"

using namespace MAT;

extern "C"
{

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetAppExperimentETag(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jstring jstrAppExperimentETag) {
    auto AppExperimentETag = JStringToStdString(env, jstrAppExperimentETag);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetAppExperimentETag(AppExperimentETag);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetEventExperimentIds(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jstring jstrEventName,
        jstring jstrExperimentIds) {
    auto eventName = JStringToStdString(env, jstrEventName);
    auto experimentIds = JStringToStdString(env, jstrExperimentIds);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetEventExperimentIds(eventName, experimentIds);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeClearExperimentIds(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr) {
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->ClearExperimentIds();
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetNetworkCost(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jint networkCost) {
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetNetworkCost(static_cast<NetworkCost>(networkCost));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetNetworkType(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jint networkType) {
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetNetworkType(static_cast<NetworkType>(networkType));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetUserId(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jstring jstrUserId,
        jint piiKind) {
    auto userId = JStringToStdString(env, jstrUserId);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetUserId(userId, static_cast<PiiKind>(piiKind));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetCommonFieldString(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jstring jstrName,
        jstring jstrValue) {
    auto name = JStringToStdString(env, jstrName);
    auto value = JStringToStdString(env, jstrValue);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetCommonField(name, value);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetCommonField(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jstring jstrName,
        jobject jEventProperty) {
    auto name = JStringToStdString(env, jstrName);
    auto prop = GetEventProperty(env, jEventProperty);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetCommonField(name, prop);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetCustomFieldString(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jstring jstrName,
        jstring jstrValue) {
    auto name = JStringToStdString(env, jstrName);
    auto value = JStringToStdString(env, jstrValue);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetCustomField(name, value);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetCustomField(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jstring jstrName,
        jobject jEventProperty) {
    auto name = JStringToStdString(env, jstrName);
    auto prop = GetEventProperty(env, jEventProperty);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetCustomField(name, prop);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_SemanticContext_nativeSetTicket (
        JNIEnv* env,
        jobject /* this */,
        jlong nativeSemanticContextPtr,
        jint ticketType,
        jstring jstrTicketValue) {
    auto ticketValue = JStringToStdString(env, jstrTicketValue);
    reinterpret_cast<ISemanticContext*>(nativeSemanticContextPtr)->SetTicket(static_cast<TicketType>(ticketType), ticketValue);
}

}

