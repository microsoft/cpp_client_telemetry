//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef JAVADATAVIEWERPROXY_HPP
#define JAVADATAVIEWERPROXY_HPP

#include "IDataViewer.hpp"

#include <jni.h>
#include <memory>
#include <string>

namespace MAT_NS_BEGIN
{
    class JavaDataViewerProxy final : public IDataViewer
    {
    public:
        static std::shared_ptr<JavaDataViewerProxy> Create(JNIEnv* env, jobject dataViewer) noexcept;

        ~JavaDataViewerProxy() noexcept override;

        void ReceiveData(const std::vector<uint8_t>& packetData) noexcept override;
        const char* GetName() const noexcept override;
        bool IsTransmissionEnabled() const noexcept override;
        const std::string& GetCurrentEndpoint() const noexcept override;

    private:
        JavaDataViewerProxy() = default;

        JNIEnv* GetEnv(bool& attached) const noexcept;
        void DetachIfNeeded(bool attached) const noexcept;
        bool ClearPendingException(JNIEnv* env, const char* methodName) const noexcept;
        bool ReadString(JNIEnv* env, jmethodID method, std::string& value) const noexcept;

        JavaVM* m_javaVm = nullptr;
        jobject m_dataViewer = nullptr;
        jmethodID m_receiveData = nullptr;
        jmethodID m_getName = nullptr;
        jmethodID m_isTransmissionEnabled = nullptr;
        jmethodID m_getCurrentEndpoint = nullptr;
        std::string m_name;
    };

} MAT_NS_END

#endif
