#include "JniUtils.hpp"
#include "ISemanticContext.hpp"

using namespace MAT;

extern "C"
{

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetAppExperimentETag(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jstring jstrAppExperimentETag) {
    auto AppExperimentETag = JStringToStdString(env, jstrAppExperimentETag);
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetAppExperimentETag(AppExperimentETag);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetEventExperimentIds(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jstring jstrEventName,
        jstring jstrExperimentIds) {
    auto eventName = JStringToStdString(env, jstrEventName);
    auto experimentIds = JStringToStdString(env, jstrExperimentIds);
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetEventExperimentIds(eventName, experimentIds);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeClearExperimentIds(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr) {
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->ClearExperimentIds();
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetNetworkCost(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jint networkCost) {
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetNetworkCost(static_cast<NetworkCost>(networkCost));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetNetworkType(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jint networkType) {
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetNetworkType(static_cast<NetworkType>(networkType));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetUserId(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jstring jstrUserId,
        jint PiiKind_Identity) {
    auto userId = JStringToStdString(env, jstrUserId);
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetUserId(userId, static_cast<PiiKind>(PiiKind_Identity));
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetCommonStringField(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jstring jstrName,
        jint piiKind,
        jint category,
        jstring jstrEventPropertyValue) {
    auto name = JStringToStdString(env, jstrName);
    auto eventPropertyValue = JStringToStdString(env, jstrEventPropertyValue);
    EventProperty eventProperty(eventPropertyValue, static_cast<PiiKind>(piiKind), static_cast<DataCategory>(category));
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetCommonField(name, eventProperty);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetCustomStringField(
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jstring jstrName,
        jint piiKind,
        jint category,
        jstring jstrEventPropertyValue) {
    auto name = JStringToStdString(env, jstrName);
    auto eventPropertyValue = JStringToStdString(env, jstrEventPropertyValue);
    EventProperty eventProperty(eventPropertyValue, static_cast<PiiKind>(piiKind), static_cast<DataCategory>(category));
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetCustomField(name, eventProperty);
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_ISemanticContext_nativeSetTicket (
        JNIEnv* env,
        jobject /* this */,
        jlong nativeISemanticContextPtr,
        jint ticketType,
        jstring jstrTicketValue) {
    auto ticketValue = JStringToStdString(env, jstrTicketValue);
    reinterpret_cast<ISemanticContext*>(nativeISemanticContextPtr)->SetTicket(static_cast<TicketType>(ticketType), ticketValue);
}

}
