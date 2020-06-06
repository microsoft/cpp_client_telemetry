#include "JniConvertors.hpp"
#include "LogManager.hpp"
#include "modules/dataviewer/DefaultDataViewer.hpp"

using namespace MAT;

extern "C"
{
std::shared_ptr<DefaultDataViewer> g_spDefaultDataViewer;

JNIEXPORT void JNICALL Java_com_microsoft_applications_events_LogManager_initializeDiagnosticDataViewer(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrMachineIdentifier) {
    auto machineIdentifier = JStringToStdString(env, jstrMachineIdentifier);
    g_spDefaultDataViewer = std::make_shared<DefaultDataViewer>(nullptr, machineIdentifier);
    LogManager::GetDataViewerCollection().RegisterViewer(std::static_pointer_cast<IDataViewer>(g_spDefaultDataViewer));
}

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_enableRemoteViewer(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrEndpoint) {
    if (g_spDefaultDataViewer == nullptr)
        return false;

    auto endpoint = JStringToStdString(env, jstrEndpoint);
    return g_spDefaultDataViewer->EnableRemoteViewer(endpoint);
}

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_disableViewer(
        JNIEnv* env,
        jclass /* this */) {
    if (g_spDefaultDataViewer == nullptr)
        return false;

    return g_spDefaultDataViewer->DisableViewer();
}

JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_isViewerEnabled(
        JNIEnv* env,
        jclass /* this */) {
    return LogManager::GetDataViewerCollection().IsViewerEnabled();
}

}