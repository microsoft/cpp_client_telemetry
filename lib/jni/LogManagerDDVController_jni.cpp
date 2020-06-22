#include "JniConvertors.hpp"
#include "LogManager.hpp"
#include "modules/dataviewer/DefaultDataViewer.hpp"

using namespace MAT;
#include "WrapperLogManager.hpp"

// The static instance of WrapperLogManager is instantiated in LogManager_jni.cpp

extern "C"
{

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_initializeDiagnosticDataViewer(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrMachineIdentifier,
        jstring jstrEndpoint) {
    auto machineIdentifier = JStringToStdString(env, jstrMachineIdentifier);
    auto endpoint = JStringToStdString(env, jstrEndpoint);
    std::shared_ptr<DefaultDataViewer> spDefaultDataViewer = std::make_shared<DefaultDataViewer>(nullptr, machineIdentifier);
    if (spDefaultDataViewer->EnableRemoteViewer(endpoint)) {
        WrapperLogManager::GetDataViewerCollection().UnregisterAllViewers();
        WrapperLogManager::GetDataViewerCollection().RegisterViewer(std::static_pointer_cast<IDataViewer>(spDefaultDataViewer));
        return true;
    }
    else {
        return false;
    }
}

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_LogManager_disableViewer(
        JNIEnv* env,
        jclass /* this */) {
    WrapperLogManager::GetDataViewerCollection().UnregisterAllViewers();
}

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_isViewerEnabled(
        JNIEnv* env,
        jclass /* this */) {
    return WrapperLogManager::GetDataViewerCollection().IsViewerEnabled();
}

}