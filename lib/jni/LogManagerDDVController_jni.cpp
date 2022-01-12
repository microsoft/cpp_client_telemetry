//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "JniConvertors.hpp"
#include "WrapperLogManager.hpp"
#include "modules/dataviewer/DefaultDataViewer.hpp"

using namespace MAT;

// The static instance of WrapperLogManager is instantiated in LogManager_jni.cpp

extern "C"
{
std::shared_ptr<DefaultDataViewer> spDefaultDataViewer;

JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManager_initializeDiagnosticDataViewer(
        JNIEnv *env,
        jclass /* this */,
        jstring jstrMachineIdentifier,
        jstring jstrEndpoint) {
    if (spDefaultDataViewer != nullptr) {
        WrapperLogManager::GetDataViewerCollection().UnregisterViewer(spDefaultDataViewer->GetName());
    }

    auto machineIdentifier = JStringToStdString(env, jstrMachineIdentifier);
    auto endpoint = JStringToStdString(env, jstrEndpoint);
    std::shared_ptr<DefaultDataViewer> defaultDataViewer = std::make_shared<DefaultDataViewer>(nullptr, machineIdentifier);
    if (defaultDataViewer->EnableRemoteViewer(endpoint)) {
        spDefaultDataViewer = defaultDataViewer;
        WrapperLogManager::GetDataViewerCollection().RegisterViewer(std::static_pointer_cast<IDataViewer>(spDefaultDataViewer));
        return true;
    } else {
        spDefaultDataViewer = nullptr;
        return false;
    }
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_LogManager_disableViewer(
        JNIEnv *env,
        jclass /* this */) {
    if (spDefaultDataViewer != nullptr) {
        WrapperLogManager::GetDataViewerCollection().UnregisterViewer(spDefaultDataViewer->GetName());
        spDefaultDataViewer = nullptr;
    }
}

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_isViewerEnabled(
        JNIEnv *env,
        jclass /* this */) {
    if (spDefaultDataViewer != nullptr) {
        return WrapperLogManager::GetDataViewerCollection().IsViewerEnabled(spDefaultDataViewer->GetName());
    } else {
        return false;
    }
}

JNIEXPORT jstring JNICALL Java_com_microsoft_applications_events_LogManager_getCurrentEndpoint(
        JNIEnv *env,
        jclass /* this */) {
    std::string endpoint = "";
    if (spDefaultDataViewer != nullptr) {
        endpoint = spDefaultDataViewer->GetCurrentEndpoint();
    }
    return env->NewStringUTF(endpoint.c_str());
}

}
