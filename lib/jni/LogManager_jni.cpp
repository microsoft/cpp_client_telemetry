#include "JniConvertors.hpp"
#include "LogManagerBase.hpp"

using namespace MAT;

class WrapperConfig : public ILogConfiguration
{
};
class WrapperLogManager : public LogManagerBase<WrapperConfig>
{
};

template <>
ILogManager* LogManagerBase<WrapperConfig>::instance{};

extern "C"
{
    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeInitializeWithoutTenantToken(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        ILogger* logger = WrapperLogManager::Initialize();
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeInitializeWithTenantToken(
        JNIEnv* env,
        jclass /* this */,
        jstring jTenantToken)
    {
        auto tenantToken = JStringToStdString(env, jTenantToken);
        ILogger* logger = WrapperLogManager::Initialize(tenantToken);
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeFlushAndTeardown(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::FlushAndTeardown());
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeFlush(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::Flush());
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeUploadNow(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::UploadNow());
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativePauseTransmission(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::PauseTransmission());
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeResumeTransmission(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::ResumeTransmission());
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetIntTransmitProfile(
        JNIEnv* /* env */,
        jclass /* this */,
        jint jProfile)
    {
        return static_cast<jint>(WrapperLogManager::SetTransmitProfile(
            static_cast<TransmitProfile>(jProfile)));
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetTransmitProfileString(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrProfile)
    {
        return static_cast<jint>(WrapperLogManager::SetTransmitProfile(JStringToStdString(env, jstrProfile)));
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeLoadTransmitProfilesString(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrProfilesJson)
    {
        return static_cast<jint>(WrapperLogManager::LoadTransmitProfiles(JStringToStdString(env, jstrProfilesJson)));
    }

    JNIEXPORT jint JNICALL Java_com_microsoft_applications_events_LogManager_nativeResetTransmitProfiles(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::ResetTransmitProfiles());
    }

    JNIEXPORT jstring JNICALL Java_com_microsoft_applications_events_LogManager_getTransmitProfileName(
        JNIEnv* env,
        jclass /* this */)
    {
        std::string profileName = WrapperLogManager::GetTransmitProfileName();
        return static_cast<jstring>(env->NewStringUTF(profileName.c_str()));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetSemanticContext(
        JNIEnv* env,
        jclass /* this */)
    {
        return reinterpret_cast<jlong>(WrapperLogManager::GetSemanticContext());
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextStringValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        auto value = JStringToStdString(env, jstrValue);
        return static_cast<jint>(WrapperLogManager::SetContext(name, value, static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextIntValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jint jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name, static_cast<int32_t>(jValue), static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextLongValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jlong jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name, static_cast<int64_t>(jValue), static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextDoubleValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jdouble jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name, static_cast<double>(jValue), static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextBoolValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jboolean jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name, static_cast<bool>(jValue), static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextTimeTicksValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jlong jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name, time_ticks_t(static_cast<uint64_t>(jValue)), static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeSetContextGuidValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        auto value = JStringToStdString(env, jstrValue);
        return static_cast<jint>(WrapperLogManager::SetContext(name, GUID_t(value.c_str()), static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetLogger(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        ILogger* logger = WrapperLogManager::GetLogger();
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetLoggerWithSource(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrSource)
    {
        auto source = JStringToStdString(env, jstrSource);
        ILogger* logger = WrapperLogManager::GetLogger(source);
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jlong JNICALL Java_com_microsoft_applications_events_LogManager_nativeGetLoggerWithTenantTokenAndSource(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrTenantToken,
        jstring jstrSource)
    {
        auto tenantToken = JStringToStdString(env, jstrTenantToken);
        auto source = JStringToStdString(env, jstrSource);
        ILogger* logger = WrapperLogManager::GetLogger(tenantToken, source);
        return reinterpret_cast<jlong>(logger);
    }
}
