//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#if defined(__has_include)
#if __has_include("modules/dataviewer/DefaultDataViewer.hpp")
#include "modules/dataviewer/DefaultDataViewer.hpp"
#define HAS_DDV true
#endif
#if __has_include("modules/privacyguard/PrivacyGuard.hpp")
#include "modules/privacyguard/PrivacyGuard.hpp"
#include "PrivacyGuardHelper.hpp"
#define HAS_PG true
#endif
#if __has_include("modules/signals/Signals.hpp")
#include "SignalsHelper.hpp"
#include "modules/signals/Signals.hpp"
#define HAS_SS true
#endif
#if __has_include("modules/sanitizer/Sanitizer.hpp")
#include "SanitizerHelper.hpp"
#include "modules/sanitizer/Sanitizer.hpp"
#define HAS_SAN true
#endif
#endif

#include <utils/Utils.hpp>
#include "JniConvertors.hpp"
#include "LogManagerBase.hpp"
#include "WrapperLogManager.hpp"
#include "android/log.h"
#include "config/RuntimeConfig_Default.hpp"

using namespace MAT;

template <>
ILogManager* LogManagerBase<WrapperConfig>::instance{};

extern "C"
{
    JNIEXPORT jlong JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeInitializeWithoutTenantToken(
        JNIEnv* /* env */,
        jclass /* LogManager.class */)
    {
        ILogger* logger = WrapperLogManager::Initialize();
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jlong JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeInitializeWithTenantToken(
        JNIEnv* env,
        jclass /* LogManager.class */,
        jstring jTenantToken)
    {
        auto tenantToken = JStringToStdString(env, jTenantToken);
        ILogger* logger = WrapperLogManager::Initialize(tenantToken);
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeFlushAndTeardown(
        JNIEnv* /* env */,
        jclass /* LogManager.class */)
    {
        return static_cast<jint>(WrapperLogManager::FlushAndTeardown());
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeFlush(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::Flush());
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeUploadNow(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::UploadNow());
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativePauseTransmission(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::PauseTransmission());
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeResumeTransmission(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::ResumeTransmission());
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetIntTicketToken(
        JNIEnv* env,
        jclass /* this */,
        jint jType,
        jstring jstrTokenValue)
    {
        auto ticketValue = JStringToStdString(env, jstrTokenValue);
        return static_cast<jint>(WrapperLogManager::SetTicketToken(
            static_cast<TicketType>(jType), ticketValue));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetIntTransmitProfile(
        JNIEnv* /* env */,
        jclass /* this */,
        jint jProfile)
    {
        return static_cast<jint>(WrapperLogManager::SetTransmitProfile(
            static_cast<TransmitProfile>(jProfile)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetTransmitProfileString(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrProfile)
    {
        return static_cast<jint>(WrapperLogManager::SetTransmitProfile(
            JStringToStdString(env, jstrProfile)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeLoadTransmitProfilesString(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrProfilesJson)
    {
        return static_cast<jint>(WrapperLogManager::LoadTransmitProfiles(
            JStringToStdString(env, jstrProfilesJson)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeResetTransmitProfiles(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        return static_cast<jint>(WrapperLogManager::ResetTransmitProfiles());
    }

    JNIEXPORT jstring JNICALL
    Java_com_microsoft_applications_events_LogManager_getTransmitProfileName(
        JNIEnv* env,
        jclass /* this */)
    {
        std::string profileName = WrapperLogManager::GetTransmitProfileName();
        return static_cast<jstring>(env->NewStringUTF(profileName.c_str()));
    }

    JNIEXPORT jlong JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeGetSemanticContext(
        JNIEnv* env,
        jclass /* this */)
    {
        return reinterpret_cast<jlong>(WrapperLogManager::GetSemanticContext());
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetContextStringValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        auto value = JStringToStdString(env, jstrValue);
        return static_cast<jint>(WrapperLogManager::SetContext(name,
                                                               value,
                                                               static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetContextIntValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jint jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name,
                                                               static_cast<int32_t>(jValue),
                                                               static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetContextLongValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jlong jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name,
                                                               static_cast<int64_t>(jValue),
                                                               static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetContextDoubleValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jdouble jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name,
                                                               static_cast<double>(jValue),
                                                               static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetContextBoolValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jboolean jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name,
                                                               static_cast<bool>(jValue),
                                                               static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetContextTimeTicksValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jlong jValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        return static_cast<jint>(WrapperLogManager::SetContext(name,
                                                               time_ticks_t(
                                                                   static_cast<uint64_t>(jValue)),
                                                               static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jint JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeSetContextGuidValue(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrName,
        jstring jstrValue,
        jint piiKind)
    {
        auto name = JStringToStdString(env, jstrName);
        auto value = JStringToStdString(env, jstrValue);
        return static_cast<jint>(WrapperLogManager::SetContext(name,
                                                               GUID_t(value.c_str()),
                                                               static_cast<PiiKind>(piiKind)));
    }

    JNIEXPORT jlong JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeGetLogger(
        JNIEnv* /* env */,
        jclass /* this */)
    {
        ILogger* logger = WrapperLogManager::GetLogger();
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jlong JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeGetLoggerWithSource(
        JNIEnv* env,
        jclass /* this */,
        jstring jstrSource)
    {
        auto source = JStringToStdString(env, jstrSource);
        ILogger* logger = WrapperLogManager::GetLogger(source);
        return reinterpret_cast<jlong>(logger);
    }

    JNIEXPORT jlong JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeGetLoggerWithTenantTokenAndSource(
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

    JNIEXPORT jboolean JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeRegisterPrivacyGuardOnDefaultLogManager(
            JNIEnv* env,
            jclass /* this */) {
#if HAS_PG
        auto pg = PrivacyGuardHelper::GetPrivacyGuardPtr();
        if (pg != nullptr) {
            WrapperLogManager::GetInstance()->SetDataInspector(pg);
            return true;
        }
#endif
        return false;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeUnregisterPrivacyGuardOnDefaultLogManager(
            JNIEnv* env,
            jclass /* this */) {
#if HAS_PG
        auto pg = PrivacyGuardHelper::GetPrivacyGuardPtr();
            if (pg != nullptr) {
                WrapperLogManager::GetInstance()->RemoveDataInspector(pg->GetName());
                return true;
            }
#endif
        return false;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeRegisterSignalsOnDefaultLogManager(JNIEnv *env, jclass clazz) {
#if HAS_SS
        auto logManager = WrapperLogManager::GetInstance();
        auto ss = SignalsHelper::GetSignalsInspector();
        if (ss != nullptr) {
            logManager->SetDataInspector(ss);
            return true;
        }
#endif
        return false;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeUnregisterSignalsOnDefaultLogManager(
            JNIEnv *env, jclass clazz) {
#if HAS_SS
        auto logManager = WrapperLogManager::GetInstance();
        auto ss = SignalsHelper::GetSignalsInspector();
        if (ss != nullptr) {
            logManager->RemoveDataInspector(ss->GetName());
            return true;
        }
#endif
        return false;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeRegisterSanitizerOnDefaultLogManager(
        JNIEnv *env, jclass clazz) {
#if HAS_SAN
        auto logManager = WrapperLogManager::GetInstance();
        auto ss = SanitizerHelper::GetSanitizerPtr();
        if (ss != nullptr) {
            logManager->SetDataInspector(ss);
            return true;
        }
#endif
        return false;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_microsoft_applications_events_LogManager_nativeUnregisterSanitizerOnDefaultLogManager(
            JNIEnv *env, jclass clazz) {
#if HAS_SAN
        auto logManager = WrapperLogManager::GetInstance();
        auto ss = SanitizerHelper::GetSanitizerPtr();
        if (ss != nullptr) {
            logManager->RemoveDataInspector(ss->GetName());
            return true;
        }
#endif
        return false;
    }
}

namespace
{
    /**
* helper function: rethrow any exceptions from reverse-JNI calls
* @param env
*/
    void rethrow(JNIEnv* env)
    {
        if (env->ExceptionCheck())
        {
            env->Throw(env->ExceptionOccurred());
            throw std::runtime_error("JNI exception");
        }
    }

    bool TryJStringToStdString(JNIEnv* env, jstring value, std::string& result)
    {
        if (value == nullptr)
        {
            return false;
        }

        result = JStringToStdString(env, value);
        return !env->ExceptionCheck();
    }

    /**
* Smart object to manage PushLocalFrame/PopLocalFrame
*/

    class FrameWrapper
    {
        JNIEnv* env;
        size_t frameSize;
        jobject* result = nullptr;

        FrameWrapper() = delete;

       public:
        /*
* Constructor: takes JNIEnv* and the desired LocalStack frame depth
*/
        FrameWrapper(JNIEnv* e, size_t s) :
            env(e),
            frameSize(s)
        {
            env->PushLocalFrame(frameSize);
            rethrow(env);
        }

        /**
* Set the reference that will survive PopLocalFrame (as a new
* reference in the outer frame).
* @param r Object that should survive
* @return Previous result value
*/
        jobject* setResult(jobject* r)
        {
            jobject* t = result;
            result = r;
            return t;
        }

        /**
* On destruction, pop the frame with an optional result object.
*/
        virtual ~FrameWrapper()
        {
            jobject localRef = nullptr;
            if (!!result)
            {
                localRef = *result;
            }
            localRef = env->PopLocalFrame(localRef);
            rethrow(env);
            if (!!result)
            {
                *result = localRef;
            }
        }
    };

    /**
* Enum of the types we know how to convert into a VariantMap or
* VariantArray.
*/
    enum class ValueTypes
    {
        BOOLEAN,
        LONG,
        STRING,
        VARIANT_MAP,
        VARIANT_ARRAY
    };

    /**
* POD to record how we handle each known value type
*/
    struct ValueInfo
    {
        /**
* JNI class reference for a known type
*/
        jclass valueClass;
        /**
* Method ID for the method to cast into the primitive type for
* Long or Boolean
*/
        jmethodID castMethod;
    };

    static constexpr char lcClassName[] =
        "com/microsoft/applications/events/ILogConfiguration";

    struct VariantTranslator
    {
        std::map<ValueTypes, ValueInfo> classCache;

        JNIEnv* env;

        VariantTranslator() = delete;

        VariantTranslator(JNIEnv* env) :
            env(env)
        {
            ValueInfo vi;
            vi.valueClass = env->FindClass("java/lang/Boolean");
            rethrow(env);
            vi.castMethod =
                env->GetMethodID(vi.valueClass, "booleanValue", "()Z");
            rethrow(env);
            classCache[ValueTypes::BOOLEAN] = vi;
            vi.valueClass = env->FindClass("java/lang/Long");
            rethrow(env);
            vi.castMethod =
                env->GetMethodID(vi.valueClass, "longValue", "()J");
            rethrow(env);
            classCache[ValueTypes::LONG] = vi;
            vi.valueClass = env->FindClass("java/lang/String");
            rethrow(env);
            vi.castMethod = nullptr;
            classCache[ValueTypes::STRING] = vi;
            vi.valueClass = env->FindClass(lcClassName);
            rethrow(env);
            vi.castMethod = nullptr;
            classCache[ValueTypes::VARIANT_MAP] = vi;
            vi.valueClass = env->FindClass("[Ljava/lang/Object;");
            vi.castMethod = nullptr;
            classCache[ValueTypes::VARIANT_ARRAY] = vi;
        }

        void translateVariantArray(VariantArray& array, jobjectArray value)
        {
            jsize count = env->GetArrayLength(value);
            array.clear();
            array.reserve(count);
            for (jsize i = 0; i < count; ++i)
            {
                auto element = env->GetObjectArrayElement(value, i);
                rethrow(env);
                array.emplace_back(std::move(translateVariant(element)));
            }
        }

        void translateVariantMap(VariantMap& variantMap, jobject configuration)
        {
            static std::map<ValueTypes, ValueInfo> classCache;
            auto stringClass = env->FindClass("java/lang/String");
            rethrow(env);
            auto configClass = env->GetObjectClass(configuration);
            auto gkaMethod =
                env->GetMethodID(configClass,
                                 "getKeyArray",
                                 "()[Ljava/lang/String;");
            rethrow(env);
            jobjectArray keys = static_cast<jobjectArray>(env->CallObjectMethod(
                configuration,
                gkaMethod));
            rethrow(env);
            auto getMethod = env->GetMethodID(
                configClass,
                "getObject",
                "(Ljava/lang/String;)Ljava/lang/Object;");
            rethrow(env);
            jsize keyCount = env->GetArrayLength(keys);
            for (jsize i = 0; i < keyCount; ++i)
            {
                FrameWrapper wrapper(env, 32);
                rethrow(env);
                auto k = env->GetObjectArrayElement(keys, i);
                rethrow(env);
                if (k == nullptr)
                {
                    __android_log_print(ANDROID_LOG_ERROR,
                                        "MAE",
                                        "Null configuration key");
                    continue;
                }
                if (!env->IsInstanceOf(k, stringClass))
                {
                    __android_log_print(ANDROID_LOG_ERROR, "MAE",
                                        "Configuration key is not a string");
                    continue;
                }
                auto key = static_cast<jstring>(k);
                std::string stringKey;
                if (!TryJStringToStdString(env, key, stringKey))
                {
                    rethrow(env);
                    throw std::runtime_error("Unable to convert configuration key");
                }
                auto value = env->CallObjectMethod(configuration, getMethod, key);
                rethrow(env);
                if (!value)
                {
                    __android_log_print(
                        ANDROID_LOG_WARN,
                        "MAE",
                        "Null value for key %s in translateVariantMap",
                        stringKey.c_str());
                }
                auto v = translateVariant(value);
                auto emplace = variantMap.emplace(stringKey, std::move(v));
                if (!emplace.second)
                {
                    auto& it = emplace.first;
                    it->second.move(std::move(v));
                }
            }  // for (... configuration keys)
        }

        Variant translateVariant(jobject value)
        {
            if (!value)
            {
                return Variant();
            }
            for (auto& kv : classCache)
            {
                if (env->IsInstanceOf(value, kv.second.valueClass))
                {
                    switch (kv.first)
                    {
                    case ValueTypes::LONG:
                    {
                        auto longValue =
                            env->CallLongMethod(value,
                                                kv.second.castMethod);
                        rethrow(env);
                        return Variant(longValue);
                    }
                    case ValueTypes::BOOLEAN:
                    {
                        bool booleanValue =
                            (env->CallBooleanMethod(value,
                                                    kv.second.castMethod) ==
                             JNI_TRUE);
                        rethrow(env);
                        return Variant(booleanValue);
                    }
                    case ValueTypes::STRING:
                    {
                        auto s = static_cast<jstring>(value);
                        std::string cppString;
                        if (!TryJStringToStdString(env, s, cppString))
                        {
                            rethrow(env);
                            throw std::runtime_error("Unable to convert string value");
                        }
                        return Variant(std::move(cppString));
                    }
                    case ValueTypes::VARIANT_MAP:
                    {
                        VariantMap subMap;
                        translateVariantMap(subMap, value);
                        return Variant(std::move(subMap));
                    }
                    case ValueTypes::VARIANT_ARRAY:
                    {
                        VariantArray subArray;
                        translateVariantArray(subArray,
                                              static_cast<jobjectArray>(value));
                        return Variant(std::move(subArray));
                    }
                    default:
                        throw std::logic_error("Unknown enum value");
                    }
                }  // if class matches
            }      // for (... classCache){
            auto actual = env->GetObjectClass(value);
            auto meta = env->GetObjectClass(actual);
            rethrow(env);
            auto gnMethod =
                env->GetMethodID(meta,
                                 "getName",
                                 "()Ljava/lang/String;");
            rethrow(env);
            auto jName =
                static_cast<jstring>(env->CallObjectMethod(actual,
                                                           gnMethod));
            std::string className;
            if (!TryJStringToStdString(env, jName, className))
            {
                rethrow(env);
                throw std::runtime_error("Unable to convert class name");
            }
            __android_log_print(ANDROID_LOG_ERROR,
                                "MAE",
                                "Unsupported class %s",
                                className.c_str());
            auto errorClass = env->FindClass("java/lang/Error");
            rethrow(env);
            env->ThrowNew(errorClass, "Unsupported class");
            MATSDK_THROW(std::logic_error("Unsupported class"));
        }
    };

    struct ConfigConstructor
    {
        JNIEnv* env;

        jobject boolTrue = nullptr;
        jobject boolFalse = nullptr;
        jclass doubleClass = nullptr;
        jmethodID doubleInit = nullptr;
        jclass longClass = nullptr;
        jmethodID longInit = nullptr;
        jclass objectClass = nullptr;
        jclass configClass = nullptr;
        jmethodID configInit = nullptr;
        jmethodID setMethod = nullptr;

        ConfigConstructor() = delete;
        ConfigConstructor(JNIEnv* env)
        {
            this->env = env;
            auto boolClass = env->FindClass("java/lang/Boolean");
            rethrow(env);
            auto truthField =
                env->GetStaticFieldID(boolClass, "TRUE", "Ljava/lang/Boolean;");
            rethrow(env);
            boolTrue = env->GetStaticObjectField(boolClass, truthField);
            auto untruthField =
                env->GetStaticFieldID(boolClass, "FALSE", "Ljava/lang/Boolean;");
            rethrow(env);
            boolFalse = env->GetStaticObjectField(boolClass, untruthField);
            doubleClass = env->FindClass("java/lang/Double");
            rethrow(env);
            doubleInit = env->GetMethodID(doubleClass, "<init>", "(D)V");
            rethrow(env);
            longClass = env->FindClass("java/lang/Long");
            rethrow(env);
            longInit = env->GetMethodID(longClass, "<init>", "(J)V");
            rethrow(env);
            objectClass = env->FindClass("java/lang/Object");
            rethrow(env);
            configClass = env->FindClass(
                "com/microsoft/applications/events/LogManager$LogConfigurationImpl");
            rethrow(env);
            configInit = env->GetMethodID(configClass, "<init>", "()V");
            rethrow(env);
            setMethod = env->GetMethodID(configClass,
                                         "set",
                                         "(Ljava/lang/String;Ljava/lang/Object;)V");
            rethrow(env);
        }

        jobject valueTranslate(Variant& variant)
        {
            jobject result = nullptr;
            {
                FrameWrapper frameWrapper(env, 8);
                frameWrapper.setResult(&result);
                switch (variant.type)
                {
                case Variant::Type::TYPE_BOOL:
                {
                    bool const v = variant;
                    if (v)
                    {
                        result = boolTrue;
                    }
                    else
                    {
                        result = boolFalse;
                    }
                    break;
                }
                case Variant::Type::TYPE_DOUBLE:
                {
                    jdouble const v = variant;
                    result = env->NewObject(doubleClass, doubleInit, v);
                    rethrow(env);
                    break;
                }
                case Variant::Type::TYPE_INT:
                {
                    jlong const v = variant;
                    result = env->NewObject(longClass, longInit, v);
                    rethrow(env);
                    break;
                }
                case Variant::Type::TYPE_NULL:
                    break;
                case Variant::Type::TYPE_OBJ:
                {
                    VariantMap variantMap = variant;
                    result = mapTranslate(variantMap);
                    break;
                }
                case Variant::Type::TYPE_STRING:
                case Variant::Type::TYPE_STRING2:
                {
                    const char* v = variant;
                    result = env->NewStringUTF(v);
                    break;
                }
                case Variant::Type::TYPE_ARR:
                {
                    VariantArray& variantArray = variant;
                    {
                        FrameWrapper
                            innerWrapper(env, variantArray.size() + 4);
                        auto array = env->NewObjectArray(variantArray.size(),
                                                         objectClass,
                                                         nullptr);
                        for (size_t i = 0; i < variantArray.size(); ++i)
                        {
                            jobject element = valueTranslate(variantArray[i]);
                            env->SetObjectArrayElement(array, i, element);
                            rethrow(env);
                        }
                        result = array;
                        innerWrapper.setResult(&result);
                    }
                    break;
                }
                default:
                    auto errorClass = env->FindClass("java/lang/Error");
                    rethrow(env);
                    env->ThrowNew(errorClass, "Unsupported class");
                    MATSDK_THROW(std::logic_error("Unsupported class"));
                }
            }
            return result;
        }

        jobject mapTranslate(VariantMap& variantMap)
        {
            auto map = env->NewObject(configClass, configInit);
            rethrow(env);
            for (auto& kv : variantMap)
            {
                FrameWrapper frameWrapper(env, 8);
                auto const& key = kv.first;
                auto& value = kv.second;
                auto keyString = env->NewStringUTF(key.c_str());
                rethrow(env);
                auto valueObject = valueTranslate(value);
                env->CallVoidMethod(map, setMethod, keyString, valueObject);
                rethrow(env);
            }
            return map;
        }
    };

#ifdef HAS_DDV
    struct ManagerAndConfig
    {
        ILogConfiguration config;
        ILogManager* manager;
        std::shared_ptr<DefaultDataViewer> ddv;
    };
#else
    struct ManagerAndConfig
    {
        ILogConfiguration config;
        ILogManager* manager;
    };
#endif

    using MCVector = std::vector<std::unique_ptr<ManagerAndConfig>>;

    static MCVector jniManagers;
    static std::mutex jniManagersMutex;
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_microsoft_applications_events_LogManager_nativeInitializeConfig(JNIEnv* env,
                                                                         jclass clazz,
                                                                         jstring tenant_token,
                                                                         jobject configuration)
{
    ILogConfiguration logConfiguration;
    VariantTranslator variantTranslator(env);
    variantTranslator.translateVariantMap(*logConfiguration, configuration);
    std::string cereal;
    Variant::serialize(*logConfiguration, cereal);
    __android_log_print(ANDROID_LOG_INFO, "MAE", "Translated map: %s",
                        cereal.c_str());

    std::string token;
    if (!TryJStringToStdString(env, tenant_token, token))
    {
        return 0;
    }
    auto logger = WrapperLogManager::Initialize(token, logConfiguration);
    return reinterpret_cast<jlong>(logger);
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_microsoft_applications_events_LogManager_nativeGetLogConfiguration(
    JNIEnv* env,
    jclass /* LogManager.class */)
{
    ConfigConstructor config(env);
    return config.mapTranslate(*WrapperLogManager::GetLogConfiguration());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_nativeCreateLogManager(
    JNIEnv* env,
    jclass /* LogManagerProvider */,
    jobject configuration)
{
    VariantTranslator variantTranslator(env);
    size_t n;
    auto mcPointer = std::make_unique<ManagerAndConfig>();

    variantTranslator.translateVariantMap(*(mcPointer->config),
                                          configuration);

    status_t status = status_t::STATUS_SUCCESS;
    mcPointer->manager = MAT::LogManagerProvider::CreateLogManager(
        mcPointer->config,
        status);
    if (status == status_t::STATUS_SUCCESS && !!mcPointer->manager)
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        n = jniManagers.size();
        jniManagers.emplace_back(std::move(mcPointer));
        return n;
    }
    __android_log_print(ANDROID_LOG_ERROR,
                        "MAE",
                        "Failed to create log manager");
    return -1;
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeGetLogConfigurationCopy(
    JNIEnv* env,
    jobject thiz,
    jlong nativeLogManagerIndex)
{
    ManagerAndConfig const* mc;
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        if (nativeLogManagerIndex < 0 || nativeLogManagerIndex >= static_cast<jlong>(jniManagers.size()))
        {
            return nullptr;
        }
        // mc will outlive this method call because jniManagers
        // is static, and we never destroy individual array
        // vector elements
        mc = jniManagers[nativeLogManagerIndex].get();
    }
    ConfigConstructor builder(env);
    auto vm = mc->config;
    return builder.mapTranslate(*vm);
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeClose(
    JNIEnv* env,
    jobject /* this */,
    jlong nativeLogManager)
{
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        if (nativeLogManager < 0 || nativeLogManager >= static_cast<jlong>(jniManagers.size()))
        {
            return;
        }
        // we reset the manager member of the ManagerAndConfig,
        // but the ManagerAndConfig itself will survive until
        // the static jniManagers array is destroyed.
        jniManagers[nativeLogManager]->manager = nullptr;
    }
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_microsoft_applications_events_LogManager_00024LogConfigurationImpl_roundTrip(
    JNIEnv* env,
    jobject thiz)
{
    ILogConfiguration logConfiguration;
    VariantTranslator variantTranslator(env);
    variantTranslator.translateVariantMap(*logConfiguration, thiz);

    ConfigConstructor builder(env);
    return builder.mapTranslate(*logConfiguration);
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeGetLogger(
    JNIEnv* env,
    jobject thiz,
    jstring jToken,
    jstring jSource,
    jstring jScope)
{
    if (!thiz) {
        return 0;
    }
    auto LogManagerProviderClassID = env->GetObjectClass(thiz);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        return 0;
    }
    if (!LogManagerProviderClassID) {
        return 0;
    }
    auto nativeLogManagerID =
        env->GetFieldID(LogManagerProviderClassID, "nativeLogManager", "J");
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        return 0;
    }
    auto nativeLogManagerIndex = env->GetLongField(thiz, nativeLogManagerID);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        return 0;
    }
    ManagerAndConfig* mc;
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        if (nativeLogManagerIndex < 0 || nativeLogManagerIndex >= static_cast<jlong>(jniManagers.size()))
        {
            return 0;
        }
        mc = jniManagers[nativeLogManagerIndex].get();
        if (!mc)
            return 0;
    }
    std::string token;
    std::string source;
    std::string scope;
    if (!TryJStringToStdString(env, jToken, token) ||
        !TryJStringToStdString(env, jSource, source) ||
        !TryJStringToStdString(env, jScope, scope))
    {
        return 0;
    }
    return reinterpret_cast<jlong>(mc->manager->GetLogger(
        token,
        source,
        scope));
}

static ILogManager* getLogManager(jlong nativeLogManager)
{
    std::lock_guard<std::mutex> lock(jniManagersMutex);
    if (nativeLogManager < 0 || nativeLogManager >= static_cast<jlong>(jniManagers.size()))
    {
        return nullptr;
    }

    return jniManagers[nativeLogManager]->manager;
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeFlushAndTeardown(
    JNIEnv* env,
    jobject thiz,
    jlong nativeLogManager)
{
    auto logManager = getLogManager(nativeLogManager);
    if (!logManager)
    {
        return;
    }
    logManager->FlushAndTeardown();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeFlush(
    JNIEnv* env,
    jobject thiz,
    jlong nativeLogManager)
{
    auto logManager = getLogManager(nativeLogManager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    return logManager->Flush();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeUploadNow(
    JNIEnv* env,
    jobject thiz,
    jlong nativeLogManager)
{
    auto logManager = getLogManager(nativeLogManager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    return logManager->UploadNow();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativePauseTransmission(
    JNIEnv* env,
    jobject thiz,
    jlong nativeLogManager)
{
    auto logManager = getLogManager(nativeLogManager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    return logManager->PauseTransmission();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeResumeTransmission(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    return logManager->ResumeTransmission();
}
extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetTransmitProfileTP(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jint profile)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    return logManager->SetTransmitProfile(static_cast<TransmitProfile>(profile));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetTransmitProfileS(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring profile)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string stringyProfile;
    if (!TryJStringToStdString(env, profile, stringyProfile))
    {
        return STATUS_EFAIL;
    }
    return logManager->SetTransmitProfile(stringyProfile);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeLoadTransmitProfiles(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring json)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppJson;
    if (!TryJStringToStdString(env, json, cppJson))
    {
        return STATUS_EFAIL;
    }
    return logManager->LoadTransmitProfiles(cppJson);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeResetTransmitProfiles(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    return logManager->ResetTransmitProfiles();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeGetTransmitProfileName(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return nullptr;
    }
    auto name = logManager->GetTransmitProfileName();
    return env->NewStringUTF(name.c_str());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeGetSemanticContext(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return 0;
    }
    return reinterpret_cast<uint64_t>(&logManager->GetSemanticContext());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetContextString(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring name,
    jstring value,
    jint pii_kind)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppName;
    std::string cppValue;
    if (!TryJStringToStdString(env, name, cppName) ||
        !TryJStringToStdString(env, value, cppValue))
    {
        return STATUS_EFAIL;
    }

    return logManager->SetContext(cppName, cppValue,
                                  static_cast<PiiKind>(pii_kind));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetContextInt(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring name,
    jint value,
    jint pii_kind)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppName;
    if (!TryJStringToStdString(env, name, cppName))
    {
        return STATUS_EFAIL;
    }
    return logManager->SetContext(cppName, value,
                                  static_cast<PiiKind>(pii_kind));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetContextLong(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring name,
    jlong value,
    jint pii_kind)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppName;
    if (!TryJStringToStdString(env, name, cppName))
    {
        return STATUS_EFAIL;
    }
    return logManager->SetContext(cppName, value,
                                  static_cast<PiiKind>(pii_kind));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetContextDouble(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring name,
    jdouble value,
    jint pii_kind)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppName;
    if (!TryJStringToStdString(env, name, cppName))
    {
        return STATUS_EFAIL;
    }
    return logManager->SetContext(cppName, value,
                                  static_cast<PiiKind>(pii_kind));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetContextBoolean(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring name,
    jboolean value,
    jint pii_kind)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppName;
    if (!TryJStringToStdString(env, name, cppName))
    {
        return STATUS_EFAIL;
    }
    return logManager->SetContext(cppName, value,
                                  static_cast<PiiKind>(pii_kind));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetContextDate(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring name,
    jobject value,
    jint pii_kind)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppName;
    if (!TryJStringToStdString(env, name, cppName))
    {
        return STATUS_EFAIL;
    }
    auto dateClass = env->GetObjectClass(value);
    auto getTimeID = env->GetMethodID(dateClass, "getTime", "()J");
    auto javaMilliseconds = env->CallLongMethod(value, getTimeID);
    constexpr uint64_t ticksPerMillisecond = ticksPerSecond / 1000ull;
    time_ticks_t
        sdkTicks = (javaMilliseconds * ticksPerMillisecond) + ticksUnixEpoch;
    return logManager->SetContext(cppName, sdkTicks,
                                  static_cast<PiiKind>(pii_kind));
}

extern "C" JNIEXPORT jint JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetContextUUID(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring name,
    jstring value,
    jint pii_kind)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return STATUS_EFAIL;
    }
    std::string cppName;
    std::string cppValue;
    if (!TryJStringToStdString(env, name, cppName) ||
        !TryJStringToStdString(env, value, cppValue))
    {
        return STATUS_EFAIL;
    }
    return logManager->SetContext(cppName, GUID_t(cppValue.c_str()),
                                  static_cast<PiiKind>(pii_kind));
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeInitializeDDV(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jstring jmachine_identifier,
    jstring jendpoint)
{
#ifndef HAS_DDV
    return false;
#else
    auto log_manager = getLogManager(native_log_manager);
    if (!log_manager)
    {
        return false;
    }
    auto machine_identifier = JStringToStdString(env, jmachine_identifier);
    auto endpoint = JStringToStdString(env, jendpoint);
    std::shared_ptr<DefaultDataViewer>
        ddv = std::make_shared<DefaultDataViewer>(nullptr, machine_identifier);
    if (!ddv->EnableRemoteViewer(endpoint))
    {
        return false;
    }
    std::shared_ptr<DefaultDataViewer> to_register = ddv;
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        ddv.swap(jniManagers[native_log_manager]->ddv);
    }
    if (ddv)
    {
        log_manager->GetDataViewerCollection().UnregisterViewer(ddv->GetName());
    }
    log_manager->GetDataViewerCollection().RegisterViewer(to_register);
    return true;
#endif
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeDisableViewer(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager)
{
#ifndef HAS_DDV
    return;
#else
    auto log_manager = getLogManager(native_log_manager);
    if (!log_manager)
    {
        return;
    }
    std::shared_ptr<DefaultDataViewer> to_unregister;
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        to_unregister.swap(jniManagers[native_log_manager]->ddv);
    }
    if (!to_unregister)
    {
        return;
    }
    log_manager->GetDataViewerCollection().UnregisterViewer(to_unregister->GetName());
#endif
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeIsViewerEnabled(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager)
{
#ifndef HAS_DDV
    return false;
#else
    auto log_manager = getLogManager(native_log_manager);
    if (!log_manager)
    {
        return false;
    }
    std::shared_ptr<DefaultDataViewer> ddv;
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        ddv = jniManagers[native_log_manager]->ddv;
    }
    return (!!ddv) && log_manager->GetDataViewerCollection().IsViewerEnabled(ddv->GetName());
#endif
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeGetCurrentEndpoint(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager)
{
#ifndef HAS_DDV
    return env->NewStringUTF("");
#else
    auto log_manager = getLogManager(native_log_manager);
    if (!log_manager)
    {
        return env->NewStringUTF("");
    }
    std::shared_ptr<DefaultDataViewer> ddv;
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        ddv = jniManagers[native_log_manager]->ddv;
    }
    if (ddv)
    {
        return env->NewStringUTF(ddv->GetCurrentEndpoint().c_str());
    }
    return env->NewStringUTF("");
#endif
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeGetLogSessionData(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jobject result)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return;
    }
    auto sessionData = logManager->GetLogSessionData();

    auto resultClassId = env->GetObjectClass(result);
    auto timeId = env->GetFieldID(resultClassId, "m_first_time", "J");
    env->SetLongField(result,
                      timeId,
                      static_cast<jlong>(sessionData->getSessionFirstTime()));

    auto
        uuidId = env->GetFieldID(resultClassId, "m_uuid", "Ljava/lang/String;");
    auto uuidUtf = env->NewStringUTF(sessionData->getSessionSDKUid().c_str());
    env->SetObjectField(result, uuidId, uuidUtf);
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeSetLevelFilter(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jint default_level,
    jintArray allowed_levels)
{
    auto logManager = getLogManager(native_log_manager);
    if (!logManager)
    {
        return;
    }
    std::set<uint8_t> allowedSet;
    auto length = env->GetArrayLength(allowed_levels);
    if (length > 0)
    {
        std::vector<jint> things(length, 0);
        env->GetIntArrayRegion(allowed_levels, 0, length, things.data());
        for (const auto& level : things)
        {
            if (level >= 0 && level < 256)
            {
                allowedSet.insert(level);
            }
        }
    }
    if (default_level >= 0 && default_level < 256)
    {
        logManager->SetLevelFilter(default_level, allowedSet);
    }
}

namespace
{
    struct JniDebugEventListener : DebugEventListener
    {
        struct Registration
        {
            jlong logManager;
            DebugEventType eventType;
        };

        JavaVM* javaVm;
        jobject javaListener = nullptr;
        jclass eventClass = nullptr;
        jmethodID eventConstructor = nullptr;
        jmethodID listenerMethod = nullptr;
        std::vector<Registration> registrations;

        JniDebugEventListener() = delete;

        JniDebugEventListener(JNIEnv* env, JavaVM* vm, jobject listener) :
            javaVm(vm)
        {
            auto localEventClass =
                env->FindClass("com/microsoft/applications/events/DebugEvent");
            rethrow(env);
            eventConstructor = env->GetMethodID(
                localEventClass, "<init>", "(JJJJJLjava/lang/Object;J)V");
            rethrow(env);

            auto localListenerClass = env->GetObjectClass(listener);
            rethrow(env);
            listenerMethod = env->GetMethodID(
                localListenerClass,
                "onDebugEvent",
                "(Lcom/microsoft/applications/events/DebugEvent;)V");
            rethrow(env);

            eventClass = static_cast<jclass>(env->NewGlobalRef(localEventClass));
            rethrow(env);
            javaListener = env->NewGlobalRef(listener);
            if (javaListener == nullptr)
            {
                env->DeleteGlobalRef(eventClass);
                eventClass = nullptr;
                rethrow(env);
                throw std::runtime_error("Unable to retain debug event listener");
            }

            env->DeleteLocalRef(localListenerClass);
            env->DeleteLocalRef(localEventClass);
        }

        ~JniDebugEventListener() override
        {
            bool detach = false;
            auto env = GetEnv(detach);
            if (env == nullptr)
            {
                return;
            }

            env->DeleteGlobalRef(javaListener);
            env->DeleteGlobalRef(eventClass);
            if (detach)
            {
                javaVm->DetachCurrentThread();
            }
        }

        void OnDebugEvent(DebugEvent& evt) override
        {
            bool detach = false;
            auto env = GetEnv(detach);
            if (env == nullptr)
            {
                return;
            }

            if (env->PushLocalFrame(1) == JNI_OK)
            {
                auto eventLocal = env->NewObject(
                    eventClass,
                    eventConstructor,
                    static_cast<jlong>(evt.seq),
                    static_cast<jlong>(evt.ts),
                    static_cast<jlong>(evt.type),
                    static_cast<jlong>(evt.param1),
                    static_cast<jlong>(evt.param2),
                    static_cast<jobject>(nullptr),
                    static_cast<jlong>(evt.size));
                if (eventLocal != nullptr && !env->ExceptionCheck())
                {
                    env->CallVoidMethod(javaListener, listenerMethod, eventLocal);
                }
                env->PopLocalFrame(nullptr);
            }

            if (env->ExceptionCheck())
            {
                env->ExceptionDescribe();
                env->ExceptionClear();
            }
            if (detach)
            {
                javaVm->DetachCurrentThread();
            }
        }

        bool IsSameListener(JNIEnv* env, jobject listener) const
        {
            return env->IsSameObject(javaListener, listener) == JNI_TRUE;
        }

        bool AddRegistration(jlong logManager, DebugEventType eventType)
        {
            auto existing = std::find_if(
                registrations.begin(),
                registrations.end(),
                [logManager, eventType](const Registration& registration) {
                    return registration.logManager == logManager &&
                           registration.eventType == eventType;
                });
            if (existing != registrations.end())
            {
                return false;
            }

            registrations.push_back({logManager, eventType});
            return true;
        }

        bool RemoveRegistration(jlong logManager, DebugEventType eventType)
        {
            auto existing = std::find_if(
                registrations.begin(),
                registrations.end(),
                [logManager, eventType](const Registration& registration) {
                    return registration.logManager == logManager &&
                           registration.eventType == eventType;
                });
            if (existing == registrations.end())
            {
                return false;
            }

            registrations.erase(existing);
            return true;
        }

       private:
        JNIEnv* GetEnv(bool& detach) const
        {
            detach = false;
            JNIEnv* env = nullptr;
            auto status = javaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
            if (status == JNI_EDETACHED)
            {
                if (javaVm->AttachCurrentThread(&env, nullptr) != JNI_OK)
                {
                    return nullptr;
                }
                detach = true;
            }
            else if (status != JNI_OK)
            {
                return nullptr;
            }

            return env;
        }
    };

    static std::vector<std::unique_ptr<JniDebugEventListener>> listeners;
    static std::mutex listeners_mutex;
}  // anonymous namespace

extern "C" JNIEXPORT jlong JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeAddEventListener(
    JNIEnv* env,
    jobject thiz,
    jlong native_log_manager,
    jlong event_type,
    jobject listener,
    jlong current_identity)
{
    auto logManager = getLogManager(native_log_manager);
    if (logManager == nullptr || listener == nullptr)
    {
        return -1;
    }

    auto eventType = static_cast<DebugEventType>(event_type);
    JniDebugEventListener* callback = nullptr;
    jlong identity = -1;
    {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        if (current_identity >= 0 &&
            current_identity < static_cast<jlong>(listeners.size()) &&
            listeners[current_identity] &&
            listeners[current_identity]->IsSameListener(env, listener))
        {
            callback = listeners[current_identity].get();
            identity = current_identity;
        }
    }

    if (!callback)
    {
        JavaVM* vm = nullptr;
        if (env->GetJavaVM(&vm) != JNI_OK)
        {
            return -1;
        }

        try
        {
            auto newCallback = std::make_unique<JniDebugEventListener>(env, vm, listener);
            callback = newCallback.get();

            std::lock_guard<std::mutex> lock(listeners_mutex);
            auto available = std::find(listeners.begin(), listeners.end(), nullptr);
            if (available == listeners.end())
            {
                identity = static_cast<jlong>(listeners.size());
                listeners.emplace_back(std::move(newCallback));
            }
            else
            {
                identity = static_cast<jlong>(std::distance(listeners.begin(), available));
                *available = std::move(newCallback);
            }
        }
        catch (const std::exception& e)
        {
            if (!env->ExceptionCheck())
            {
                auto exceptionClass = env->FindClass("java/lang/RuntimeException");
                if (exceptionClass != nullptr)
                {
                    env->ThrowNew(exceptionClass, e.what());
                    env->DeleteLocalRef(exceptionClass);
                }
            }
            return -1;
        }
    }

    {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        if (!callback->AddRegistration(native_log_manager, eventType))
        {
            return identity;
        }
    }
    logManager->AddEventListener(eventType, *callback);
    return identity;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeRemoveEventListener(
    JNIEnv *env,
    jobject thiz,
    jlong native_log_manager,
    jlong eventType,
    jlong identity,
    jobject listener) {
    JniDebugEventListener* callback = nullptr;
    auto newIdentity = identity;
    {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        if (identity < 0 ||
            identity >= static_cast<jlong>(listeners.size()) ||
            !listeners[identity] ||
            !listeners[identity]->IsSameListener(env, listener))
        {
            return identity;
        }

        callback = listeners[identity].get();
        if (!callback->RemoveRegistration(
                native_log_manager,
                static_cast<DebugEventType>(eventType)))
        {
            return identity;
        }
        if (callback->registrations.empty())
        {
            newIdentity = -1;
        }
    }

    auto logManager = getLogManager(native_log_manager);
    if (logManager != nullptr)
    {
        logManager->RemoveEventListener(
            static_cast<DebugEventType>(eventType),
            *callback);
    }
    if (newIdentity < 0)
    {
        std::lock_guard<std::mutex> lock(listeners_mutex);
        if (listeners[identity].get() == callback &&
            callback->registrations.empty())
        {
            listeners[identity].reset();
        }
    }
    return newIdentity;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_microsoft_applications_events_ILogConfiguration_getDefaultConfiguration(
    JNIEnv *env,
    jclass clazz) {
    ILogConfiguration emptyConfig;
    RuntimeConfig_Default defaultConfig(emptyConfig);
    ConfigConstructor builder(env);
    return builder.mapTranslate(*emptyConfig);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeRegisterPrivacyGuard(
        JNIEnv *env,
        jobject thiz,
        jlong native_log_manager) {
#if HAS_PG
    auto logManager = getLogManager(native_log_manager);
    auto pg = PrivacyGuardHelper::GetPrivacyGuardPtr();
    if(pg != nullptr) {
        logManager->SetDataInspector(pg);
        return true;
    }
#endif
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeRegisterSignals(
        JNIEnv *env,
        jobject thiz,
        jlong native_log_manager) {
#if HAS_SS
    auto logManager = getLogManager(native_log_manager);
    auto ss = SignalsHelper::GetSignalsInspector();
    if(ss != nullptr) {
        logManager->SetDataInspector(ss);
        return true;
    }
#endif
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeRegisterSanitizer(
        JNIEnv *env,
        jobject thiz,
        jlong native_log_manager) {
#if HAS_SAN
    auto logManager = getLogManager(native_log_manager);
    auto sa = SanitizerHelper::GetSanitizerPtr();
    if (sa != nullptr) {
        logManager->SetDataInspector(sa);
        return true;
    }
#endif
    return false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManager_pauseActivity(JNIEnv *env, jclass clazz) {
    WrapperLogManager::PauseActivity();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManager_resumeActivity(JNIEnv *env, jclass clazz) {
    WrapperLogManager::ResumeActivity();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManager_waitPause(JNIEnv *env, jclass clazz) {
    WrapperLogManager::WaitPause();
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManager_startActivity(JNIEnv *env, jclass clazz) {
    return WrapperLogManager::StartActivity() ? JNI_TRUE : JNI_FALSE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManager_endActivity(JNIEnv *env, jclass clazz) {
    WrapperLogManager::EndActivity();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativePauseActivity(
        JNIEnv *env, jobject thiz, jlong native_log_manager) {
    auto logManager = getLogManager(native_log_manager);
    if (logManager) {
        logManager->PauseActivity();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeResumeActivity(
        JNIEnv *env, jobject thiz, jlong native_log_manager) {
    auto logManager = getLogManager(native_log_manager);
    if (logManager) {
        logManager->ResumeActivity();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeWaitPause(
        JNIEnv *env, jobject thiz, jlong native_log_manager) {
    auto logManager = getLogManager(native_log_manager);
    if (logManager) {
        logManager->WaitPause();
    }
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeUnregisterPrivacyGuard(
        JNIEnv *env,
        jobject thiz,
        jlong native_log_manager) {
#if HAS_PG
    auto logManager = getLogManager(native_log_manager);
    auto pg = PrivacyGuardHelper::GetPrivacyGuardPtr();
    if(pg != nullptr) {
        logManager->RemoveDataInspector(pg->GetName());
        return true;
    }
#endif
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeUnregisterSignals(
        JNIEnv *env,
        jobject thiz,
        jlong native_log_manager) {
#if HAS_SS
    auto logManager = getLogManager(native_log_manager);
    auto ss = SignalsHelper::GetSignalsInspector();
    if(ss != nullptr) {
        logManager->RemoveDataInspector(ss->GetName());
        return true;
    }
#endif
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeUnregisterSanitizer(
        JNIEnv *env,
        jobject thiz,
        jlong native_log_manager) {
#if HAS_SAN
    auto logManager = getLogManager(native_log_manager);
    auto sa = SanitizerHelper::GetSanitizerPtr();
    if (sa != nullptr) {
        logManager->RemoveDataInspector(sa->GetName());
        return true;
    }
#endif
    return false;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeStartActivity(
        JNIEnv *env, jobject thiz, jlong native_log_manager) {
    auto logManager = getLogManager(native_log_manager);
    if (logManager) {
        return logManager->StartActivity() ? JNI_TRUE : JNI_FALSE;
    }
    return JNI_FALSE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeEndActivity(
        JNIEnv *env, jobject thiz, jlong native_log_manager) {
    auto logManager = getLogManager(native_log_manager);
    if (logManager) {
        logManager->EndActivity();
    }
}
