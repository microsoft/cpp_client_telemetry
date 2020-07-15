#include "JniConvertors.hpp"
#include "LogManagerBase.hpp"
#include "WrapperLogManager.hpp"
#include "android/log.h"

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

       public:
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
        jclass valueClass = nullptr;
        /**
   * Method ID for the method to cast into the primitive type for
   * Long or Boolean
   */
        jmethodID castMethod = nullptr;
    };

    constexpr char lcClassName[] =
        "com/microsoft/applications/events/ILogConfiguration";

    struct VariantTranslator
    {
        std::map<ValueTypes, ValueInfo> classCache;

        JNIEnv* env;

        VariantTranslator() = delete;

        explicit VariantTranslator(JNIEnv* env) :
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
            auto keys = static_cast<jobjectArray>(env->CallObjectMethod(
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

    struct ManagerAndConfig
    {
        ILogConfiguration config;
        ILogManager* manager = nullptr;
    };

    using MCVector = std::vector<ManagerAndConfig>;

    MCVector jniManagers;
    std::mutex jniManagersMutex;
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
    {
        std::lock_guard<std::mutex> lock(jniManagersMutex);
        n = jniManagers.size();
        jniManagers.emplace_back();
    }
    variantTranslator.translateVariantMap(*jniManagers[n].config,
                                          configuration);

    status_t status = status_t::STATUS_SUCCESS;
    jniManagers[n].manager = MAT::LogManagerProvider::CreateLogManager(
        jniManagers[n].config,
        status);
    if (status == status_t::STATUS_SUCCESS && !!jniManagers[n].manager)
    {
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
        mc = &jniManagers[nativeLogManagerIndex];
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
    }
    if (!jniManagers[nativeLogManager].manager)
    {
        return;
    }
    MAT::LogManagerProvider::Release(jniManagers[nativeLogManager].config);
    jniManagers[nativeLogManager].manager = nullptr;
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