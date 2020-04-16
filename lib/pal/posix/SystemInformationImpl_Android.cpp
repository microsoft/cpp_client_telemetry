// Copyright (c) Microsoft Corporation. All rights reserved.

#include <jni.h>
#include "pal/PAL.hpp"
#include "pal/SystemInformationImpl.hpp"

#include <string>

namespace PAL_NS_BEGIN {

    class AndroidSystemInformationConnector {
      public:
        static std::string s_app_id;
        static std::string s_app_version;
        static std::string s_app_language;

        static std::string s_os_major_version;
        static std::string s_os_full_version;
        static std::string s_os_name;

        static void setValue(JNIEnv *env, std::string & s, jstring js)
        {
            const char *start = env->GetStringUTFChars(js, nullptr);
            auto end = start + env->GetStringUTFLength(js);
            s = std::string(start, end);
            env->ReleaseStringUTFChars(js, start);
        }
    };

    std::string AndroidSystemInformationConnector::s_app_id;
    std::string AndroidSystemInformationConnector::s_app_version;
    std::string AndroidSystemInformationConnector::s_app_language;

    std::string AndroidSystemInformationConnector::s_os_major_version;
    std::string AndroidSystemInformationConnector::s_os_full_version;
    std::string AndroidSystemInformationConnector::s_os_name;

    SystemInformationImpl::SystemInformationImpl() :
        m_info_helper(),
        m_app_id(AndroidSystemInformationConnector::s_app_id),
        m_app_version(AndroidSystemInformationConnector::s_app_version),
        m_app_language(AndroidSystemInformationConnector::s_app_language),
        m_os_major_version(AndroidSystemInformationConnector::s_os_major_version),
        m_os_full_version(AndroidSystemInformationConnector::s_os_full_version),
        m_os_name("Android")
    {
    }

    SystemInformationImpl::~SystemInformationImpl()
    {
    }

    int SystemInformationImpl::RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback)
    {
        return m_info_helper.RegisterInformationChangedCallback(pCallback);
    }

    void SystemInformationImpl::UnRegisterInformationChangedCallback(int callbackToken)
    {
        m_info_helper.UnRegisterInformationChangedCallback(callbackToken);
    }

    ISystemInformation* SystemInformationImpl::Create()
    {
        return new SystemInformationImpl();
    }

} PAL_NS_END

extern "C" JNIEXPORT void JNICALL Java_com_microsoft_applications_events_HttpClient_setSystemInfo(
    JNIEnv* env,
    jobject /* java_client */,
    jstring app_id,
    jstring app_version,
    jstring app_language,

    jstring os_major_version,
    jstring os_full_version
)
{
    PAL::AndroidSystemInformationConnector::setValue(
        env,
        PAL::AndroidSystemInformationConnector::s_app_id,
        app_id);
    PAL::AndroidSystemInformationConnector::setValue(
        env,
        PAL::AndroidSystemInformationConnector::s_app_version,
        app_version);
    PAL::AndroidSystemInformationConnector::setValue(
        env,
        PAL::AndroidSystemInformationConnector::s_app_language,
        app_language);
    PAL::AndroidSystemInformationConnector::setValue(
        env,
        PAL::AndroidSystemInformationConnector::s_os_major_version,
        os_major_version);
    PAL::AndroidSystemInformationConnector::setValue(
        env,
        PAL::AndroidSystemInformationConnector::s_os_full_version,
        os_full_version);
}
