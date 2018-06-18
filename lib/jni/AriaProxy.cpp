// Copyright (c) Microsoft. All rights reserved.

#include "JavaUtils.hpp"
#include "com_microsoft_applications_telemetry_AriaProxy.h"
#include <aria/ILogManager.hpp>


using namespace MAT;

namespace ARIASDK_NS_BEGIN {


ILogManager* g_jniLogManager;


} ARIASDK_NS_END


/*
 * Class:     com_microsoft_applications_telemetry_AriaProxy
 * Method:    setAppIdForLoggerNative
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
extern "C" JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_telemetry_AriaProxy_setAppIdForLoggerNative(JNIEnv* env, jclass clazz, jstring tenantToken, jstring appId)
{
    std::string tenantToken_ = JStringToStdString(env, tenantToken);
    std::string appId_       = JStringToStdString(env, appId);
    LOGI("AriaProxy: setAppIdForLoggerNative(tenantToken=%s, appId=%s)", tenantToken_.c_str(), appId_.c_str());

    if (g_jniLogManager == NULL) {
        LOGE("AriaProxy: g_jniLogManager is NULL");
        return JNI_FALSE;
    }

    ILogger* pLogger = g_jniLogManager->GetLogger(tenantToken_, "");
    if (pLogger == NULL) {
        LOGE("AriaProxy: pLogger is NULL");
        return JNI_FALSE;
    }

    pLogger->GetSemanticContext().SetAppId(appId_);
    return JNI_TRUE;
}

/*
 * Class:     com_microsoft_applications_telemetry_AriaProxy
 * Method:    createEventPropertiesNative
 * Signature: (Ljava/lang/String;I)J
 */
extern "C" JNIEXPORT jlong JNICALL Java_com_microsoft_applications_telemetry_AriaProxy_createEventPropertiesNative(JNIEnv* env, jclass clazz, jstring eventName, jint priority)
{
    std::string   eventName_ = JStringToStdString(env, eventName);
    EventPriority priority_  = static_cast<EventPriority>(priority);
    LOGI("AriaProxy: createEventPropertiesNative(name=%s, priority=%d)", eventName_.c_str(), priority_);

    EventProperties* properties = new EventProperties(eventName_);
    properties->SetPriority(priority_);

    LOGI("AriaProxy: createEventPropertiesNative() returning %p", properties);
    return reinterpret_cast<jlong>(properties);
}

/*
 * Class:     com_microsoft_applications_telemetry_AriaProxy
 * Method:    logEventAndDeleteEventPropertiesNative
 * Signature: (Ljava/lang/String;J)Z
 */
extern "C" JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_telemetry_AriaProxy_logEventAndDeleteEventPropertiesNative(JNIEnv* env, jclass clazz, jstring tenantToken, jlong eventProperties)
{
    std::string      tenantToken_     = JStringToStdString(env, tenantToken);
    EventProperties* eventProperties_ = reinterpret_cast<EventProperties*>(eventProperties);
    LOGI("AriaProxy: logEventAndDeleteEventPropertiesNative(tenantToken=%s, eventProperties=%p)", tenantToken_.c_str(), eventProperties_);

    if (g_jniLogManager == NULL) {
        LOGE("AriaProxy: g_jniLogManager is NULL");
        return JNI_FALSE;
    }

    if (eventProperties_ == NULL) {
        LOGE("AriaProxy: eventProperties is NULL");
        return JNI_FALSE;
    }

    ILogger* pLogger = g_jniLogManager->GetLogger(tenantToken_, "");
    if (pLogger == NULL) {
        LOGE("AriaProxy: pLogger is NULL");
        return JNI_FALSE;
    }

    pLogger->LogEvent(*eventProperties_);
    delete eventProperties_;
    return JNI_TRUE;
}

/*
 * Class:     com_microsoft_applications_telemetry_AriaProxy
 * Method:    setPropertyNative
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)V
 */
extern "C" JNIEXPORT void JNICALL Java_com_microsoft_applications_telemetry_AriaProxy_setPropertyNative(JNIEnv* env, jclass clazz, jlong eventProperties, jstring name, jstring value, jint piiKind)
{
    EventProperties* eventProperties_ = reinterpret_cast<EventProperties*>(eventProperties);
    std::string      name_            = JStringToStdString(env, name);
    std::string      value_           = JStringToStdString(env, value);
    PiiKind          piiKind_         = static_cast<PiiKind>(piiKind);
    LOGI("AriaProxy: setPropertyNative(name=%s, value=%s, piiKind=%d)", name_.c_str(), value_.c_str(), piiKind_);

    eventProperties_->SetProperty(name_, value_, piiKind_);
}

/*
 * Class:     com_microsoft_applications_telemetry_AriaProxy
 * Method:    deleteEventPropertiesNative
 * Signature: (J)V
 */
extern "C" JNIEXPORT void JNICALL Java_com_microsoft_applications_telemetry_AriaProxy_deleteEventPropertiesNative(JNIEnv* env, jclass clazz, jlong eventProperties)
{
    EventProperties* eventProperties_ = reinterpret_cast<EventProperties*>(eventProperties);
    LOGI("AriaProxy: deleteEventPropertiesNative(eventProperties=%p)", eventProperties_);

    delete eventProperties_;
}

/*
 * Class:     com_microsoft_applications_telemetry_AriaProxy
 * Method:    isInitializedNative
 * Signature: ()Z
 */
extern "C" JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_telemetry_AriaProxy_isInitializedNative(JNIEnv* env, jclass clazz)
{
    bool isInitialized = (g_jniLogManager != NULL);
    LOGI("AriaProxy: isInitializedNative -> %s", isInitialized ? "true" : "false");
    return isInitialized ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     com_microsoft_applications_telemetry_AriaProxy
 * Method:    setContextFieldsNative
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
extern "C" JNIEXPORT void JNICALL Java_com_microsoft_applications_telemetry_AriaProxy_setContextFieldsNative(JNIEnv* env, jclass clazz, jstring appId, jstring appLanguage, jstring osBuild, jstring timeZone)
{
    std::string appId_       = JStringToStdString(env, appId);
    std::string appLanguage_ = JStringToStdString(env, appLanguage);
    std::string osBuild_     = JStringToStdString(env, osBuild);
    std::string timeZone_    = JStringToStdString(env, timeZone);
    LOGI("AriaProxy: setContextFieldsNative(appId=%s, appLanguage=%s, osBuild=%s, timeZone=%s)", appId_.c_str(), appLanguage_.c_str(), osBuild_.c_str(), timeZone_.c_str());

    if (g_jniLogManager == NULL) {
        LOGE("AriaProxy: g_jniLogManager is NULL");
        return;
    }

    ISemanticContext& semanticContext = g_jniLogManager->GetSemanticContext();
    semanticContext.SetAppId(appId_);
    semanticContext.SetOsBuild(osBuild_);
    semanticContext.SetUserTimeZone(timeZone_);
    if (!appLanguage_.empty()) {
        semanticContext.SetAppLanguage(appLanguage_);
    }
}
