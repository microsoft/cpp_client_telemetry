//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "JavaDataViewerProxy.hpp"

#if defined __has_include
#if __has_include("mat/config.h")
#include "mat/config.h"
#endif
#endif

#ifdef HAVE_MAT_LOGGING
#include <android/log.h>
#endif
#include <limits>
#include <utility>

namespace MAT_NS_BEGIN
{
#ifdef HAVE_MAT_LOGGING
    namespace
    {
        constexpr const char* LOG_TAG = "MAE.JavaDataViewer";
    }
#endif

    std::shared_ptr<JavaDataViewerProxy> JavaDataViewerProxy::Create(
        JNIEnv* env,
        jobject dataViewer) noexcept
    {
        if (env == nullptr || dataViewer == nullptr)
        {
            return nullptr;
        }

        // Create() is noexcept, so the allocation must not propagate. reset() takes
        // ownership immediately: if the control block allocation throws, it deletes the
        // object itself, so there is nothing to release here - and an explicit delete
        // would be a double free. Past the guard proxy is non-null, because the throwing
        // form of new never returns null; a nothrow new here would need a null check
        // before the dereference below.
        std::shared_ptr<JavaDataViewerProxy> proxy;
        MATSDK_TRY
        {
            proxy.reset(new JavaDataViewerProxy());
        }
        MATSDK_CATCH(...)
        {
            return nullptr;
        }
        if (env->GetJavaVM(&proxy->m_javaVm) != JNI_OK)
        {
            return nullptr;
        }

        auto dataViewerClass = env->GetObjectClass(dataViewer);
        if (dataViewerClass == nullptr || env->ExceptionCheck())
        {
            env->ExceptionClear();
            return nullptr;
        }

        proxy->m_receiveData = env->GetMethodID(dataViewerClass, "receiveData", "([B)V");
        if (proxy->ClearPendingException(env, "receiveData lookup"))
        {
            env->DeleteLocalRef(dataViewerClass);
            return nullptr;
        }
        proxy->m_getName = env->GetMethodID(dataViewerClass, "getName", "()Ljava/lang/String;");
        if (proxy->ClearPendingException(env, "getName lookup"))
        {
            env->DeleteLocalRef(dataViewerClass);
            return nullptr;
        }
        proxy->m_isTransmissionEnabled =
            env->GetMethodID(dataViewerClass, "isTransmissionEnabled", "()Z");
        if (proxy->ClearPendingException(env, "isTransmissionEnabled lookup"))
        {
            env->DeleteLocalRef(dataViewerClass);
            return nullptr;
        }
        proxy->m_getCurrentEndpoint =
            env->GetMethodID(dataViewerClass, "getCurrentEndpoint", "()Ljava/lang/String;");
        if (proxy->ClearPendingException(env, "getCurrentEndpoint lookup"))
        {
            env->DeleteLocalRef(dataViewerClass);
            return nullptr;
        }
        env->DeleteLocalRef(dataViewerClass);

        if (proxy->m_receiveData == nullptr ||
            proxy->m_getName == nullptr ||
            proxy->m_isTransmissionEnabled == nullptr ||
            proxy->m_getCurrentEndpoint == nullptr)
        {
            return nullptr;
        }

        proxy->m_dataViewer = env->NewGlobalRef(dataViewer);
        if (proxy->m_dataViewer == nullptr || env->ExceptionCheck())
        {
            env->ExceptionClear();
            return nullptr;
        }

        if (!proxy->ReadString(env, proxy->m_getName, proxy->m_name) || proxy->m_name.empty())
        {
            return nullptr;
        }
        return proxy;
    }

    JavaDataViewerProxy::~JavaDataViewerProxy() noexcept
    {
        if (m_dataViewer == nullptr)
        {
            return;
        }

        bool attached = false;
        auto env = GetEnv(attached);
        if (env != nullptr)
        {
            env->DeleteGlobalRef(m_dataViewer);
        }
        m_dataViewer = nullptr;
        DetachIfNeeded(attached);
    }

    void JavaDataViewerProxy::ReceiveData(const std::vector<uint8_t>& packetData) noexcept
    {
        if (packetData.size() > static_cast<size_t>(std::numeric_limits<jsize>::max()))
        {
#ifdef HAVE_MAT_LOGGING
            __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Packet is too large for a Java byte array");
#endif
            return;
        }

        bool attached = false;
        auto env = GetEnv(attached);
        if (env == nullptr)
        {
            return;
        }

        auto packet = env->NewByteArray(static_cast<jsize>(packetData.size()));
        if (ClearPendingException(env, "receiveData allocation") || packet == nullptr)
        {
            if (packet != nullptr)
            {
                env->DeleteLocalRef(packet);
            }
            DetachIfNeeded(attached);
            return;
        }
        if (!packetData.empty())
        {
            env->SetByteArrayRegion(
                packet,
                0,
                static_cast<jsize>(packetData.size()),
                reinterpret_cast<const jbyte*>(packetData.data()));
        }

        if (!ClearPendingException(env, "receiveData copy"))
        {
            env->CallVoidMethod(m_dataViewer, m_receiveData, packet);
            ClearPendingException(env, "receiveData");
        }
        env->DeleteLocalRef(packet);
        DetachIfNeeded(attached);
    }

    const char* JavaDataViewerProxy::GetName() const noexcept
    {
        return m_name.c_str();
    }

    bool JavaDataViewerProxy::IsTransmissionEnabled() const noexcept
    {
        bool attached = false;
        auto env = GetEnv(attached);
        if (env == nullptr)
        {
            return false;
        }

        auto enabled = env->CallBooleanMethod(m_dataViewer, m_isTransmissionEnabled);
        if (ClearPendingException(env, "isTransmissionEnabled"))
        {
            enabled = JNI_FALSE;
        }
        DetachIfNeeded(attached);
        return enabled == JNI_TRUE;
    }

    const std::string& JavaDataViewerProxy::GetCurrentEndpoint() const noexcept
    {
        // IDataViewer returns the endpoint by reference, so the referent has to outlive the
        // call and must not be mutated by a concurrent caller. A thread_local buffer gives
        // each calling thread its own storage; a shared member guarded by a mutex would not,
        // because the lock is released before the caller reads the reference.
        static thread_local std::string currentEndpoint;

        bool attached = false;
        auto env = GetEnv(attached);
        if (env == nullptr)
        {
            currentEndpoint.clear();
            return currentEndpoint;
        }

        std::string endpoint;
        if (ReadString(env, m_getCurrentEndpoint, endpoint))
        {
            currentEndpoint = std::move(endpoint);
        }
        else
        {
            currentEndpoint.clear();
        }
        DetachIfNeeded(attached);
        return currentEndpoint;
    }

    JNIEnv* JavaDataViewerProxy::GetEnv(bool& attached) const noexcept
    {
        attached = false;
        if (m_javaVm == nullptr)
        {
            return nullptr;
        }

        JNIEnv* env = nullptr;
        auto result = m_javaVm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
        if (result == JNI_OK)
        {
            return env;
        }
        if (result != JNI_EDETACHED || m_javaVm->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            return nullptr;
        }
        attached = true;
        return env;
    }

    void JavaDataViewerProxy::DetachIfNeeded(bool attached) const noexcept
    {
        if (attached && m_javaVm != nullptr)
        {
            m_javaVm->DetachCurrentThread();
        }
    }

    bool JavaDataViewerProxy::ClearPendingException(
        JNIEnv* env,
        const char* methodName) const noexcept
    {
        if (!env->ExceptionCheck())
        {
            return false;
        }
        env->ExceptionClear();
#ifdef HAVE_MAT_LOGGING
        __android_log_print(
            ANDROID_LOG_ERROR,
            LOG_TAG,
            "Java IDataViewer callback failed: %s",
            methodName);
#else
        (void)methodName;
#endif
        return true;
    }

    bool JavaDataViewerProxy::ReadString(
        JNIEnv* env,
        jmethodID method,
        std::string& value) const noexcept
    {
        auto javaValue = static_cast<jstring>(env->CallObjectMethod(m_dataViewer, method));
        if (ClearPendingException(env, "string callback") || javaValue == nullptr)
        {
            return false;
        }

        auto chars = env->GetStringUTFChars(javaValue, nullptr);
        if (chars == nullptr)
        {
            ClearPendingException(env, "string conversion");
            env->DeleteLocalRef(javaValue);
            return false;
        }
        // ReadString is reached from noexcept callers, so the string allocation must not escape.
        bool assigned = false;
        MATSDK_TRY
        {
            value.assign(chars);
            assigned = true;
        }
        MATSDK_CATCH(...)
        {
            assigned = false;
        }
        env->ReleaseStringUTFChars(javaValue, chars);
        env->DeleteLocalRef(javaValue);
        if (!assigned)
        {
            return false;
        }
        return !ClearPendingException(env, "string conversion");
    }

} MAT_NS_END
