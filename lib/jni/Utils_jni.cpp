//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include <utils/StringUtils.hpp>
#include <utils/Utils.hpp>
#include "JniConvertors.hpp"

using namespace MAT;

extern "C"
{

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_Utils_validateEventName(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrEventName) {
    auto eventName = JStringToStdString(env, jstrEventName);
    auto sanitizedEventName = sanitizeIdentifier(eventName);
    EventRejectedReason isValidEventName = validateEventName(sanitizedEventName);

    if (isValidEventName == REJECTED_REASON_OK)
        return true;
    else
        return false;
}

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_Utils_validatePropertyName(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrPropertyName) {
    auto propertyName = JStringToStdString(env, jstrPropertyName);
    EventRejectedReason isValidPropertyName = validatePropertyName(propertyName);

    if (isValidPropertyName == REJECTED_REASON_OK)
        return true;
    else
        return false;
}

}
