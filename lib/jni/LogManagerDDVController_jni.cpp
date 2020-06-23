#include "JniConvertors.hpp"
#include "LogManager.hpp"
#include "WrapperLogManager.hpp"
#include "modules/dataviewer/DefaultDataViewer.hpp"

using namespace MAT;

// The static instance of WrapperLogManager is instantiated in LogManager_jni.cpp

extern "C"
{
    std::shared_ptr<DefaultDataViewer> spDefaultDataViewer;

    JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_initializeDiagnosticDataViewer(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrMachineIdentifier,
        jstring jstrEndpoint)
    {
        auto machineIdentifier = JStringToStdString(env, jstrMachineIdentifier);
        auto endpoint = JStringToStdString(env, jstrEndpoint);
        spDefaultDataViewer = std::make_shared<DefaultDataViewer>(nullptr, machineIdentifier);
        if (spDefaultDataViewer->EnableRemoteViewer(endpoint))
        {
            WrapperLogManager::GetDataViewerCollection().UnregisterAllViewers();
            WrapperLogManager::GetDataViewerCollection().RegisterViewer(std::static_pointer_cast<IDataViewer>(spDefaultDataViewer));
            return true;
        }
        else
        {
            return false;
        }
    }

    JNIEXPORT void JNICALL Java_com_microsoft_applications_events_LogManager_disableViewer(
        JNIEnv* env,
        jclass /* this */)
    {
        if (spDefaultDataViewer != nullptr)
        {
            spDefaultDataViewer->DisableViewer();
        }
    }

    JNIEXPORT jboolean JNICALL Java_com_microsoft_applications_events_LogManager_isViewerEnabled(
        JNIEnv* env,
        jclass /* this */)
    {
        if (spDefaultDataViewer != nullptr)
        {
            return WrapperLogManager::GetDataViewerCollection().IsViewerEnabled(spDefaultDataViewer->GetName());
        }

        return false;
    }

    JNIEXPORT jString JNICALL Java_com_microsoft_applications_events_LogManager_getCurrentEndpoint(
        JNIEnv* env,
        jclass /* this */)
    {
        if (spDefaultDataViewer != nullptr)
        {
            return spDefaultDataViewer->GetCurrentEndpoint();
        }

        return "";
    }
}