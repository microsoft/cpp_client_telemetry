//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
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
                auto cstringKey = env->GetStringUTFChars(key, nullptr);
                rethrow(env);
                std::string stringKey(cstringKey);
                env->ReleaseStringUTFChars(key, cstringKey);
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
                        auto cString = env->GetStringUTFChars(
                            s,
                            nullptr);
                        rethrow(env);
                        std::string cppString(cString);
                        env->ReleaseStringUTFChars(s, cString);
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
            auto cName = env->GetStringUTFChars(jName, nullptr);
            std::string className(cName);
            env->ReleaseStringUTFChars(jName, cName);
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

    auto tokenUTF = env->GetStringUTFChars(tenant_token, nullptr);
    rethrow(env);
    std::string token(tokenUTF);
    env->ReleaseStringUTFChars(tenant_token, tokenUTF);
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
        if (nativeLogManagerIndex < 0 || nativeLogManagerIndex >= jniManagers.size())
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
        if (nativeLogManager < 0 || nativeLogManager >= jniManagers.size())
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
        if (nativeLogManagerIndex < 0 || nativeLogManagerIndex >= jniManagers.size())
        {
            return 0;
        }
        mc = jniManagers[nativeLogManagerIndex].get();
        if (!mc)
            return 0;
    }
    auto tokenUtf = env->GetStringUTFChars(jToken, nullptr);
    std::string token{tokenUtf};
    env->ReleaseStringUTFChars(jToken, tokenUtf);
    auto sourceUtf = env->GetStringUTFChars(jSource, nullptr);
    std::string source{sourceUtf};
    env->ReleaseStringUTFChars(jSource, sourceUtf);
    auto scopeUtf = env->GetStringUTFChars(jScope, nullptr);
    std::string scope{scopeUtf};
    env->ReleaseStringUTFChars(jScope, scopeUtf);
    return reinterpret_cast<jlong>(mc->manager->GetLogger(
        token,
        source,
        scope));
}

static ILogManager* getLogManager(jlong nativeLogManager)
{
    std::lock_guard<std::mutex> lock(jniManagersMutex);
    if (nativeLogManager < 0 || nativeLogManager >= jniManagers.size())
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
    auto profile_string = env->GetStringUTFChars(profile, nullptr);
    std::string stringyProfile(profile_string);
    env->ReleaseStringUTFChars(profile, profile_string);
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
    auto chars = env->GetStringUTFChars(json, nullptr);
    std::string cppJson(chars);
    env->ReleaseStringUTFChars(json, chars);
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
    auto chars = env->GetStringUTFChars(name, nullptr);
    std::string cppName(chars);
    env->ReleaseStringUTFChars(name, chars);
    chars = env->GetStringUTFChars(value, nullptr);
    std::string cppValue(chars);
    env->ReleaseStringUTFChars(value, chars);

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
    auto chars = env->GetStringUTFChars(name, nullptr);
    std::string cppName(chars);
    env->ReleaseStringUTFChars(name, chars);
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
    auto chars = env->GetStringUTFChars(name, nullptr);
    std::string cppName(chars);
    env->ReleaseStringUTFChars(name, chars);
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
    auto chars = env->GetStringUTFChars(name, nullptr);
    std::string cppName(chars);
    env->ReleaseStringUTFChars(name, chars);
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
    auto chars = env->GetStringUTFChars(name, nullptr);
    std::string cppName(chars);
    env->ReleaseStringUTFChars(name, chars);
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
    auto chars = env->GetStringUTFChars(name, nullptr);
    std::string cppName(chars);
    env->ReleaseStringUTFChars(name, chars);
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
    auto chars = env->GetStringUTFChars(name, nullptr);
    std::string cppName(chars);
    env->ReleaseStringUTFChars(name, chars);
    chars = env->GetStringUTFChars(value, nullptr);
    auto result = logManager->SetContext(cppName, value,
                                         static_cast<PiiKind>(pii_kind));
    env->ReleaseStringUTFChars(value, chars);
    return result;
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
        JavaVM* javaVm;
        jobject javaListener;

        JniDebugEventListener() = delete;

        JniDebugEventListener(JavaVM* _javaVm, jobject _javaListener) :
            javaVm(_javaVm)
        {
            JNIEnv* env = nullptr;
            _javaVm->AttachCurrentThread(&env, nullptr);
            javaListener = env->NewGlobalRef(_javaListener);
        }

        ~JniDebugEventListener()
        {
            JNIEnv* env = nullptr;
            javaVm->AttachCurrentThread(&env, nullptr);
            env->DeleteGlobalRef(javaListener);
        }

        void OnDebugEvent(DebugEvent& evt) override
        {
            JNIEnv* env = nullptr;
            javaVm->AttachCurrentThread(&env, nullptr);
            auto eventClassId =
                env->FindClass("com/microsoft/applications/events/DebugEvent");
            auto constructorId = env->GetMethodID(eventClassId, "<init>",
                                                  "(JJJJJLjava/lang/Object;J)V");
            jobject eventLocal;
            eventLocal = env->NewObject(eventClassId,
                                        constructorId,
                                        static_cast<jlong>(evt.seq),
                                        static_cast<jlong>(evt.ts),
                                        static_cast<jlong>(evt.type),
                                        static_cast<jlong>(evt.param1),
                                        static_cast<jlong>(evt.param2),
                                        static_cast<jobject>(nullptr),
                                        static_cast<jlong>(evt.size));
            auto classId = env->GetObjectClass(javaListener);
            auto methodId = env->GetMethodID(classId,
                                             "onDebugEvent",
                                             "(Lcom/microsoft/applications/events/DebugEvent;)V");
            env->CallVoidMethod(javaListener, methodId, eventLocal);
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
    JavaVM* vm;
    env->GetJavaVM(&vm);
    std::unique_ptr<JniDebugEventListener> callback = std::make_unique<JniDebugEventListener>(vm, listener);
    auto logManager = getLogManager(native_log_manager);
    logManager->AddEventListener(static_cast<DebugEventType>(event_type), *callback);
    if (current_identity >= 0) {
        return current_identity;
    }
    std::lock_guard<std::mutex> l(listeners_mutex);
    listeners.emplace_back(std::move(callback));
    return listeners.size();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_LogManagerProvider_00024LogManagerImpl_nativeRemoveEventListener(
    JNIEnv *env,
    jobject thiz,
    jlong native_log_manager,
    jlong eventType,
    jlong identity) {
    std::lock_guard<std::mutex> l(listeners_mutex);
    if (identity < 0 || identity >= listeners.size() || !listeners[identity]) {
        return;
    }
    auto logManager = getLogManager(native_log_manager);
    logManager->RemoveEventListener(static_cast<DebugEventType>(eventType), *listeners[identity]);
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

