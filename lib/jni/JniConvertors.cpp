//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "JniConvertors.hpp"

namespace MAT_NS_BEGIN
{

std::string JStringToStdString(JNIEnv* env, const jstring& jstr) {
    if (jstr == NULL)
        return "";

    size_t jstr_length = env->GetStringUTFLength(jstr);
    auto jstr_utf = env->GetStringUTFChars(jstr, nullptr);
    std::string str(jstr_utf, jstr_utf + jstr_length);
    env->ReleaseStringUTFChars(jstr, jstr_utf);
    return str;
}

EventProperty GetEventProperty(JNIEnv* env, const jobject& jEventProperty) {
    jclass jEventPropertyClass = env->GetObjectClass(jEventProperty);
    jmethodID getEventPropertyValueMethodID = env->GetMethodID(jEventPropertyClass, "getEventPropertyValue", "()Lcom/microsoft/applications/events/EventPropertyValue;");
    jobject jEventPropertyValue = env->CallObjectMethod(jEventProperty, getEventPropertyValueMethodID);

    jclass jEventPropertyValueClass = env->GetObjectClass(jEventPropertyValue);
    auto getPiiKindValue = env->GetMethodID(jEventPropertyClass, "getPiiKindValue", "()I");
    auto jPiiKind = env->CallIntMethod(jEventProperty, getPiiKindValue);
    auto getDataCategoryValue = env->GetMethodID(jEventPropertyClass, "getDataCategoryValue", "()I");
    auto jDataCategory = env->CallIntMethod(jEventProperty, getDataCategoryValue);

    jmethodID getTypeMethodID = env->GetMethodID(jEventPropertyValueClass, "getType", "()I");
    jint type = env->CallIntMethod(jEventPropertyValue, getTypeMethodID);

    jmethodID methodIdToGetValueForDataTypes;
    EventProperty eventProperty;
    switch(static_cast<int>(type))
    {
        case EventProperty::TYPE_STRING : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getString", "()Ljava/lang/String;");
            auto jValue = static_cast<jstring>(env->CallObjectMethod(jEventPropertyValue, methodIdToGetValueForDataTypes));
            eventProperty = JStringToStdString(env, jValue);
            env->DeleteLocalRef(jValue);
            break;
        }

        case EventProperty::TYPE_INT64: {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getLong", "()J");
            auto jValue = env->CallLongMethod(jEventPropertyValue, methodIdToGetValueForDataTypes);
            eventProperty = static_cast<int64_t>(jValue);
            break;
        }

        case EventProperty::TYPE_DOUBLE: {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getDouble", "()D");
            auto jValue = env->CallDoubleMethod(jEventPropertyValue, methodIdToGetValueForDataTypes);
            eventProperty = static_cast<double>(jValue);
            break;
        }

        case EventProperty::TYPE_TIME : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getTimeTicks", "()J");
            auto jValue = env->CallLongMethod(jEventPropertyValue, methodIdToGetValueForDataTypes);
            eventProperty = time_ticks_t(static_cast<uint64_t>(jValue));
            break;
        }

        case EventProperty::TYPE_BOOLEAN : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getBoolean", "()Z");
            auto jValue = env->CallBooleanMethod(jEventPropertyValue, methodIdToGetValueForDataTypes);
            eventProperty = static_cast<bool>(jValue);
            break;
        }

        case EventProperty::TYPE_GUID : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getGuid", "()Ljava/lang/String;");
            auto jValue = static_cast<jstring>(env->CallObjectMethod(jEventPropertyValue, methodIdToGetValueForDataTypes));
            auto value = JStringToStdString(env, jValue);
            eventProperty = GUID_t(value.c_str());
            env->DeleteLocalRef(jValue);
            break;
        }

        case EventProperty::TYPE_STRING_ARRAY : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getStringArray", "()[Ljava/lang/String;");
            auto jObjArray = static_cast<jobjectArray>(env->CallObjectMethod(jEventPropertyValue, methodIdToGetValueForDataTypes));
            std::vector<std::string> vectorOfProperties;
            for(int i = 0; i < env->GetArrayLength(jObjArray); ++i) {
                auto jValue = static_cast<jstring>(env->GetObjectArrayElement(jObjArray, i));
                auto value = JStringToStdString(env, jValue);
                vectorOfProperties.push_back(value);
                env->DeleteLocalRef(jValue);
            }
            eventProperty = vectorOfProperties;
            env->DeleteLocalRef(jObjArray);
            break;
        }

        case EventProperty::TYPE_INT64_ARRAY : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getLongArray", "()[J");
            auto jArrayOfElements = static_cast<jlongArray>(env->CallObjectMethod(jEventPropertyValue, methodIdToGetValueForDataTypes));
            auto elements = env->GetLongArrayElements(jArrayOfElements, JNI_FALSE);
            std::vector<int64_t> vectorOfProperties;
            for(int i=0; i < env->GetArrayLength(jArrayOfElements); ++i) {
                vectorOfProperties.push_back(static_cast<int64_t>(elements[i]));
            }
            eventProperty = vectorOfProperties;
            env->ReleaseLongArrayElements(jArrayOfElements, elements, 0);
            env->DeleteLocalRef(jArrayOfElements);
            break;
        }

        case EventProperty::TYPE_DOUBLE_ARRAY : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getDoubleArray", "()[D");
            auto jArrayOfElements = static_cast<jdoubleArray>(env->CallObjectMethod(jEventPropertyValue, methodIdToGetValueForDataTypes));
            auto elements = env->GetDoubleArrayElements(jArrayOfElements, JNI_FALSE);
            std::vector<double> vectorOfProperties;
            for(int i=0; i < env->GetArrayLength(jArrayOfElements); ++i) {
                vectorOfProperties.push_back(static_cast<double>(elements[i]));
            }
            eventProperty = vectorOfProperties;
            env->ReleaseDoubleArrayElements(jArrayOfElements, elements, 0);
            env->DeleteLocalRef(jArrayOfElements);
            break;
        }

        case EventProperty::TYPE_GUID_ARRAY : {
            methodIdToGetValueForDataTypes = env->GetMethodID(jEventPropertyValueClass, "getGuidArray", "()[Ljava/lang/String;");
            auto jObjArray = static_cast<jobjectArray>(env->CallObjectMethod(jEventPropertyValue, methodIdToGetValueForDataTypes));
            std::vector<GUID_t> vectorOfProperties;
            for(int i = 0; i < env->GetArrayLength(jObjArray); ++i) {
                auto jValue = static_cast<jstring>(env->GetObjectArrayElement(jObjArray, i));
                auto value = JStringToStdString(env, jValue);
                vectorOfProperties.push_back(GUID_t(value.c_str()));
                env->DeleteLocalRef(jValue);
            }
            eventProperty = vectorOfProperties;
            env->DeleteLocalRef(jObjArray);
            break;
        }

        default :
            MATSDK_THROW(UnsupportedEventPropertyType(static_cast<int>(type)));
    }

    env->DeleteLocalRef(jEventPropertyValueClass);
    env->DeleteLocalRef(jEventPropertyValue);
    env->DeleteLocalRef(jEventPropertyClass);

    // The value assignment above (in the switch() statement) clears piiKind
    // and dataCategory, so we have assign these here, after that assignment.
    eventProperty.piiKind = static_cast<PiiKind>(jPiiKind);
    eventProperty.dataCategory = static_cast<DataCategory>(jDataCategory);
    return eventProperty;
}

EventProperties GetEventProperties(JNIEnv* env, const jstring& jstrEventName, const jstring& jstrEventType, const jint& jEventLatency,
        const jint& jEventPersistence, const jdouble& jEventPopSample, const jlong& jEventPolicyBitflags, const jlong& jTimestampInMillis,
        const jobjectArray& jEventPropertyStringKeyArray, const jobjectArray& jEventPropertyValueArray) {
    EventProperties eventProperties;
    eventProperties.SetName(JStringToStdString(env, jstrEventName));
    if (jstrEventType != NULL)
        eventProperties.SetType(JStringToStdString(env, jstrEventType));
    eventProperties.SetLatency(static_cast<EventLatency>(jEventLatency));
    eventProperties.SetPersistence(static_cast<EventPersistence>(jEventPersistence));
    eventProperties.SetPopsample(static_cast<double>(jEventPopSample));
    eventProperties.SetPolicyBitFlags(static_cast<uint64_t>(jEventPolicyBitflags));
    eventProperties.SetTimestamp(static_cast<int64_t>(jTimestampInMillis));

    for(int i = 0; i < env->GetArrayLength(jEventPropertyStringKeyArray); ++i) {
        auto jStringKey = static_cast<jstring>(env->GetObjectArrayElement(jEventPropertyStringKeyArray, i));
        auto jEventProperty = static_cast<jobject>(env->GetObjectArrayElement(jEventPropertyValueArray, i));
        auto propValue = GetEventProperty(env, jEventProperty);
        eventProperties.SetProperty(JStringToStdString(env, jStringKey), propValue);
        env->DeleteLocalRef(jStringKey);
        env->DeleteLocalRef(jEventProperty);
    }

    return eventProperties;
}

std::vector<std::string> ConvertJObjectArrayToStdStringVector(JNIEnv* env, const jobjectArray& jArrayToConvert)
{
    std::vector<std::string> stringVector;
    stringVector.reserve(env->GetArrayLength(jArrayToConvert));

    for(int i = 0; i < env->GetArrayLength(jArrayToConvert); i++)
    {
        auto jStringValue = static_cast<jstring>(env->GetObjectArrayElement(jArrayToConvert, i));
        auto stringValue = JStringToStdString(env, jStringValue);
        if(!stringValue.empty())
        {
            stringVector.emplace_back(std::move(stringValue));
        }
        env->DeleteLocalRef(jStringValue);
    }

    return stringVector;
}

} MAT_NS_END

