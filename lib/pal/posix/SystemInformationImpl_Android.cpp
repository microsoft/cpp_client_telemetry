// Copyright (c) Microsoft Corporation. All rights reserved.

#include <jni.h>
#include "pal/PAL.hpp"
#include "pal/SystemInformationImpl.hpp"
#include "jni/JniConvertors.hpp"

#include <string>

#include <android/log.h>

namespace PAL_NS_BEGIN {

    class AndroidSystemInformationConnector {
      public:
        static std::string s_app_id;
        static std::string s_app_version;
        static std::string s_app_language;

        static std::string s_os_major_version;
        static std::string s_os_full_version;
        static std::string s_os_name;

        static std::string s_time_zone;

        static void setValue(JNIEnv *env, std::string & s, jstring js)
        {
            const char *start = env->GetStringUTFChars(js, nullptr);
            auto end = start + env->GetStringUTFLength(js);
            s = std::string(start, end);
            env->ReleaseStringUTFChars(js, start);
        }

        static void PopulateSystemInfo(JavaVM* pJVM, const jobject activity)
        {
            JNIEnv* pEnv;
            if (pJVM->GetEnv(reinterpret_cast<void**>(&pEnv), JNI_VERSION_1_6) != JNI_OK)
            {
                LOG_ERROR("Failed to get JNIEnv from JavaVM");
                return;
            }

            jclass buildVersionClass = pEnv->FindClass("android/os/Build$VERSION");
            jclass contextClass = pEnv->FindClass("android/content/Context");
            jclass localeClass = pEnv->FindClass("java/util/Locale");
            jclass packageInfoClass = pEnv->FindClass("android/content/pm/PackageInfo");
            jclass packageManagerClass = pEnv->FindClass("android/content/pm/PackageManager");

            jfieldID sdkIntFid = pEnv->GetStaticFieldID(buildVersionClass, "SDK_INT", "I");
            int SDK_INT = pEnv->GetStaticIntField(buildVersionClass, sdkIntFid);

            jfieldID versionNameFid = pEnv->GetFieldID(packageInfoClass, "versionName", "Ljava/lang/String;");
            jfieldID releaseFid = pEnv->GetStaticFieldID(buildVersionClass, "RELEASE", "Ljava/lang/String;");
            jfieldID incrementalFid = pEnv->GetStaticFieldID(buildVersionClass, "INCREMENTAL", "Ljava/lang/String;");

            // public String getPackageName ()
            jmethodID getPackageNameMid = pEnv->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");

            // public abstract PackageManager getPackageManager ()
            jmethodID getPackageManagerMid = pEnv->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");

            // public abstract PackageInfo getPackageInfo ()
            jmethodID getPackageInfoMid = pEnv->GetMethodID(packageManagerClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");

            // SDK_INT >= 21: public String toLanguageTag ()
            // SDK_INT <>=> 21: public String toString ()
            jmethodID toLanguageTagMid = pEnv->GetMethodID(localeClass, SDK_INT >= 21 ? "toLanguageTag" : "toString", "()Ljava/lang/String;");

            // public static Locale getDefault ()
            jmethodID getDefaultLocaleMid = pEnv->GetStaticMethodID(localeClass, "getDefault", "()Ljava/util/Locale;");

            const char* jStr;
            jboolean isCopy;

            jstring packageNameJstr = reinterpret_cast<jstring>(pEnv->CallObjectMethod(activity, getPackageNameMid));
            jStr = pEnv->GetStringUTFChars(packageNameJstr, &isCopy);
            std::string packageName = jStr;
            pEnv->ReleaseStringUTFChars(packageNameJstr, jStr);

            std::string versionName;
            jobject packageManager = pEnv->CallObjectMethod(activity, getPackageManagerMid);
            jobject packageInfo = pEnv->CallObjectMethod(packageManager, getPackageInfoMid, packageNameJstr, 0);
            if (!pEnv->ExceptionCheck())
            {
                jstring versionNameJstr = reinterpret_cast<jstring>(
                    pEnv->GetObjectField(packageInfo, versionNameFid));

                jStr = pEnv->GetStringUTFChars(versionNameJstr, &isCopy);
                versionName = jStr;
                pEnv->ReleaseStringUTFChars(versionNameJstr, jStr);
            }
            pEnv->ExceptionClear();

            jobject defaultLocale = pEnv->CallStaticObjectMethod(localeClass, getDefaultLocaleMid);
            jstring launguageTagJstr = reinterpret_cast<jstring>(pEnv->CallObjectMethod(defaultLocale, toLanguageTagMid));
            jStr = pEnv->GetStringUTFChars(launguageTagJstr, &isCopy);
            std::string languageTag = jStr;
            pEnv->ReleaseStringUTFChars(launguageTagJstr, jStr);
            if (SDK_INT < 21)
            {
                std::replace(languageTag.begin(), languageTag.end(), '_', '-');
            }

            jstring versionReleaseJstr = reinterpret_cast<jstring>(pEnv->GetStaticObjectField(buildVersionClass, releaseFid));
            jStr = pEnv->GetStringUTFChars(versionReleaseJstr, &isCopy);
            std::string versionRelease = jStr;
            pEnv->ReleaseStringUTFChars(versionReleaseJstr, jStr);

            jstring versionIncrementalJstr = reinterpret_cast<jstring>(pEnv->GetStaticObjectField(buildVersionClass, incrementalFid));
            jStr = pEnv->GetStringUTFChars(versionIncrementalJstr, &isCopy);
            std::string versionIncremental = jStr;
            pEnv->ReleaseStringUTFChars(versionIncrementalJstr, jStr);
            std::string osVersion = versionRelease + " " + versionIncremental;

            AndroidSystemInformationConnector::s_app_id = std::move(packageName);
            AndroidSystemInformationConnector::s_app_language = std::move(languageTag);
            AndroidSystemInformationConnector::s_app_version = std::move(versionName);
            AndroidSystemInformationConnector::s_os_full_version = std::move(osVersion);
            AndroidSystemInformationConnector::s_os_major_version = std::move(versionRelease);
        }

    };

    std::string AndroidSystemInformationConnector::s_app_id;
    std::string AndroidSystemInformationConnector::s_app_version;
    std::string AndroidSystemInformationConnector::s_app_language;

    std::string AndroidSystemInformationConnector::s_os_major_version;
    std::string AndroidSystemInformationConnector::s_os_full_version;
    std::string AndroidSystemInformationConnector::s_os_name;

    std::string AndroidSystemInformationConnector::s_time_zone;

    SystemInformationImpl::SystemInformationImpl(IRuntimeConfig& configuration) :
        m_info_helper(),
        m_os_name("Android")
    {
        if (configuration.HasConfig(CFG_PTR_ANDROID_JVM) && configuration.HasConfig(CFG_JOBJECT_ANDROID_ACTIVITY))
        {
            AndroidSystemInformationConnector::PopulateSystemInfo(
                reinterpret_cast<JavaVM*>((void*)configuration[CFG_PTR_ANDROID_JVM]),
                reinterpret_cast<jobject>((void*)configuration[CFG_JOBJECT_ANDROID_ACTIVITY]));
        }

        m_app_id = AndroidSystemInformationConnector::s_app_id;
        m_app_version = AndroidSystemInformationConnector::s_app_version;
        m_app_language = AndroidSystemInformationConnector::s_app_language;
        m_os_major_version = AndroidSystemInformationConnector::s_os_major_version;
        m_os_full_version = AndroidSystemInformationConnector::s_os_full_version;
        m_user_timezone = AndroidSystemInformationConnector::s_time_zone;
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

    std::shared_ptr<ISystemInformation> SystemInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<SystemInformationImpl>(configuration);
    }
} PAL_NS_END

extern "C" JNIEXPORT void JNICALL Java_com_microsoft_applications_events_HttpClient_setSystemInfo(
    JNIEnv* env,
    jobject /* java_client */,
    jstring app_id,
    jstring app_version,
    jstring app_language,

    jstring os_major_version,
    jstring os_full_version,
    jstring time_zone
)
{
    auto capp_id = JStringToStdString(env, app_id);
    auto capp_version = JStringToStdString(env, app_version);
    auto capp_language = JStringToStdString(env, app_language);
    auto cos_major_version = JStringToStdString(env, os_major_version);
    auto cos_full_version = JStringToStdString(env, os_full_version);
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
    PAL::AndroidSystemInformationConnector::setValue(
        env,
        PAL::AndroidSystemInformationConnector::s_time_zone,
        time_zone);
}
