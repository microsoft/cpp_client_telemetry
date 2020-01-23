#include "http/HttpClient_Android.hpp"
#include "LogManagerBase.hpp"
#include "LogManager.hpp"
#include "ILogger.hpp"
#include <algorithm>
#include <jni.h>
#include <cstdio>
#include <sstream>
#include <vector>

constexpr static auto Tag = "LogManagerBase_jni";
using namespace MAT;
LOGMANAGER_INSTANCE

extern "C"
{

	JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_initalizeWithoutTenantToken(
			JNIEnv * /* env */,
			jobject /* this */) {
		ILogger* logger = Microsoft::Applications::Events::LogManager::Initialize();
		return reinterpret_cast<jlong>(logger);
		//return 0;
	}

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_initializeWithTenantToken(
            JNIEnv *env,
            jobject /* this */,
            jstring jTenantToken) {
        size_t jTenantToken_length = env->GetStringUTFLength(jTenantToken);
        auto jTenantToken_utf = env->GetStringUTFChars(jTenantToken, nullptr);
        std::string tenantToken(jTenantToken_utf, jTenantToken_utf + jTenantToken_length);
        env->ReleaseStringUTFChars(jTenantToken, jTenantToken_utf);

        ILogger* logger = Microsoft::Applications::Events::LogManager::Initialize(tenantToken);
        return reinterpret_cast<jlong>(logger);
        //return 0;
    }

}