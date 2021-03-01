//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "HttpClient_Android.hpp"
#include <algorithm>
#include <cstdio>
#include <jni.h>
#include <sstream>
#include <vector>

namespace MAT_NS_BEGIN
{
    constexpr static auto Tag = "HttpClient_Android";

    HttpClient_Android::HttpRequest::~HttpRequest() noexcept
    {
        EraseFromParent();
        if (m_java_request)
        {
            JNIEnv* env = nullptr;
            if (HttpClient_Android::s_java_vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
            {
                return;
            }
            env->DeleteGlobalRef(m_java_request);
        }
    }

    void HttpClient_Android::HttpRequest::SetMethod(std::string const& method)
    {
        m_method = method;
    }

    void HttpClient_Android::HttpRequest::SetUrl(std::string const& url)
    {
        m_url = url;
    }

    HttpHeaders& HttpClient_Android::HttpRequest::GetHeaders()
    {
        return m_headers;
    }

    void HttpClient_Android::HttpRequest::SetBody(std::vector<uint8_t>& body)
    {
        m_body = body;  // copy assignment for Great Copy Fun
    }

    std::vector<uint8_t>& HttpClient_Android::HttpRequest::GetBody()
    {
        return m_body;
    }

    void HttpClient_Android::HttpRequest::SetLatency(EventLatency latency)
    {
        // tbh, probably not ever going to do anything about this
    }

    size_t HttpClient_Android::HttpRequest::GetSizeEstimate() const
    {
        return m_body.size();  // nope, but what the heck
    }

    HttpClient_Android::HttpClient_Android()
    {
        m_id = 0;
    }

    HttpClient_Android::~HttpClient_Android()
    {
        JNIEnv* env;
        s_java_vm->AttachCurrentThread(&env, nullptr);
        env->DeleteGlobalRef(m_client);
        m_client = nullptr;  // well, why not?
    }

    IHttpRequest* HttpClient_Android::CreateRequest()
    {
        HttpRequest* local_request(new HttpRequest(*this));
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        m_requests.push_back(std::move(local_request));
        return local_request;
    }

    // we do NOT own the callback object
    // we will segfault if callback is nullptr
    void HttpClient_Android::SendRequestAsync(IHttpRequest* request,
                                              IHttpResponseCallback* callback)
    {
        JNIEnv* env = nullptr;
        std::string const& target_id(request->GetId());

        if (s_java_vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            return;
        }

        HttpRequest* r = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            for (auto&& u : m_requests)
            {
                if (u->m_id == target_id)
                {
                    if (u == request)
                    {
                        r = u;
                        r->m_callback = callback;
                        bool valid = r->m_state == RequestState::early || r->m_state == RequestState::cancel;
                        if (!valid)
                        {
                            MATSDK_THROW(std::logic_error("neither early nor cancel"));
                        }
                        if (r->m_state == RequestState::early)
                        {
                            r->m_state = RequestState::preparing;
                        }
                    }
                    break;
                }
            }
        }

        if (!r)
        {
            return;
        }

        HttpHeaders const& headers = r->GetHeaders();

        size_t total_size = 0;
        for (auto&& u : headers)
        {
            total_size += u.first.length() + u.second.length();
        }

        static constexpr int FrameSize = 16;  // at most 16 local refs in the frame
        auto pushed = env->PushLocalFrame(FrameSize);
        if (CheckException(env, r))
        {
            return;
        }
        auto header_buffer = env->NewByteArray(static_cast<jsize>(total_size));
        if (CheckException(env, r))
        {
            return;
        }
        size_t offset = 0;
        std::vector<jint> index_buffer;
        index_buffer.reserve(2u * headers.size());
        for (auto&& u : headers)
        {
            auto key_len = u.first.length();
            index_buffer.push_back(key_len);
            if (key_len > 0)
            {
                env->SetByteArrayRegion(header_buffer, offset, key_len,
                                        reinterpret_cast<jbyte const*>(u.first.data()));
                if (CheckException(env, r))
                {
                    return;
                }
            }
            offset += key_len;
            auto value_len = u.second.length();
            index_buffer.push_back(value_len);
            if (value_len > 0)
            {
                env->SetByteArrayRegion(header_buffer, offset, value_len,
                                        reinterpret_cast<jbyte const*>(u.second.data()));
                if (CheckException(env, r))
                {
                    return;
                }
            }
            offset += value_len;
        }
        auto header_lengths = env->NewIntArray(static_cast<jsize>(index_buffer.size()));
        if (CheckException(env, r))
        {
            return;
        }
        env->SetIntArrayRegion(header_lengths, 0, index_buffer.size(), index_buffer.data());
        if (CheckException(env, r))
        {
            return;
        }

        auto body = env->NewByteArray(r->m_body.size());
        if (CheckException(env, r))
        {
            return;
        }
        env->SetByteArrayRegion(body, 0, r->m_body.size(),
                                reinterpret_cast<const jbyte*>(r->m_body.data()));
        if (CheckException(env, r))
        {
            return;
        }
        auto java_id = env->NewStringUTF(request->GetId().c_str());
        if (CheckException(env, r))
        {
            return;
        }
        auto url_id = env->NewStringUTF(r->m_url.c_str());
        if (CheckException(env, r))
        {
            return;
        }
        auto method_id = env->NewStringUTF(r->m_method.c_str());
        if (CheckException(env, r))
        {
            return;
        }

        bool proceed = true;
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            proceed = r->m_state == RequestState::preparing;
        }
        jobject result = nullptr;
        if (proceed)
        {
            result = env->CallObjectMethod(
                m_client,
                m_create_id,
                url_id,
                method_id,
                body,
                java_id,
                header_lengths,
                header_buffer);
        }
        if (pushed == JNI_OK)
        {
            // pass the return value "result" through the local frame pop
            result = env->PopLocalFrame(result);
        }

        HttpRequest* cancel_me = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            for (auto&& u : m_requests)
            {
                if (u->m_id == target_id)
                {
                    if (u->m_callback != callback)
                    {
                        MATSDK_THROW(std::logic_error("callback"));
                    }
                    if (!result || u->m_state != RequestState::preparing)
                    {
                        cancel_me = u;
                        u = m_requests.back();
                        m_requests.pop_back();
                    }
                    else
                    {
                        u->m_java_request = env->NewGlobalRef(result);
                        u->m_state = RequestState::running;
                    }
                    break;
                }
            }
        }
        if (cancel_me)
        {
            CallbackForCancel(env, cancel_me);
        }
        else
        {
            env->CallVoidMethod(m_client, m_execute_id, result);
        }
    }

    void HttpClient_Android::CallbackForCancel(JNIEnv* env, HttpRequest* request)
    {
        if (request->m_java_request && env)
        {
            auto request_class = env->GetObjectClass(request->m_java_request);
            auto cancel_method = env->GetMethodID(request_class, "cancel", "(Z)Z");
            env->CallBooleanMethod(request->m_java_request, cancel_method, static_cast<jboolean>(JNI_TRUE));
        }
        if (request->m_callback)
        {
            auto failure = new HttpResponse(request->m_id);
            request->m_callback->OnHttpResponse(failure);
        }
    }

    void HttpClient_Android::CancelRequestAsync(std::string const& id)
    {
        JNIEnv* env;
        if (s_java_vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            return;
        }
        HttpRequest* cancel_me = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            for (auto&& u : m_requests)
            {
                if (u->m_id == id)
                {
                    switch (u->m_state)
                    {
                    case RequestState::cancel:
                        return;  // SendRequestAsync will cancel this request
                    case RequestState::preparing:
                    case RequestState::early:
                        // tell SendRequestAsync to cancel this request
                        u->m_state = RequestState::cancel;
                        return;
                    case RequestState::running:
                        // SendRequestAsync will not cancel this request
                        // We won the race with DispatchCallback
                        cancel_me = u;
                        u = m_requests.back();
                        m_requests.pop_back();
                        break;
                    default:
                        MATSDK_THROW(std::logic_error("request state"));
                    }
                }
            }
        }

        if (cancel_me)
        {
            CallbackForCancel(env, cancel_me);
        }
    }

    void HttpClient_Android::CancelAllRequests()
    {
        JNIEnv* env;
        if (s_java_vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
        {
            return;
        }
        std::vector<HttpRequest*> toCancel;
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            std::vector<HttpRequest*>::iterator first_cancel = std::partition(m_requests.begin(), m_requests.end(), [](HttpRequest* request) -> bool {
                switch (request->m_state)
                {
                // return false for running requests to move them to the end of the vector
                case RequestState::cancel:
                    return true;
                case RequestState::early:
                case RequestState::preparing:
                    request->m_state = RequestState::cancel;
                    return true;
                case RequestState::running:
                    return false;
                default:
                    return true;
                }
            });
            if (first_cancel != m_requests.end())
            {
                // move running requests to the toCancel vector, remove from m_requests
                toCancel.assign(first_cancel, m_requests.end());
                m_requests.erase(first_cancel, m_requests.end());
            }
        }
        for (auto const& request : toCancel)
        {
            CallbackForCancel(env, request);
        }
    }

    void HttpClient_Android::SetClient(JNIEnv* env, jobject client_)
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        if (m_client)
        {
            env->DeleteGlobalRef(m_client);
        }
        m_client = env->NewGlobalRef(client_);
        m_client_class = env->GetObjectClass(m_client);
        m_create_id = env->GetMethodID(m_client_class, "createTask",
                                       "(Ljava/lang/String;Ljava/lang/String;[BLjava/lang/String;[I[B)Ljava/util/concurrent/FutureTask;");
        m_execute_id = env->GetMethodID(m_client_class, "executeTask",
                                        "(Ljava/util/concurrent/FutureTask;)V");

        env->GetJavaVM(&s_java_vm);
    }

    void HttpClient_Android::EraseRequest(HttpRequest* request)
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        for (auto&& u : m_requests)
        {
            if (u == request)
            {
                u = m_requests.back();
                m_requests.pop_back();
                return;
            }
        }
    }

    HttpClient_Android::HttpRequest* HttpClient_Android::GetAndRemoveRequest(std::string id)
    {
        std::lock_guard<std::mutex> lock(m_requestsMutex);
        for (auto&& u : m_requests)
        {
            if (u->m_id == id)
            {
                auto r = u;
                u = m_requests.back();
                m_requests.pop_back();
                return r;
            }
        }
        return nullptr;
    }

    // since std::to_string() is unavailable (for now), old-school
    // string from number.

    std::string HttpClient_Android::NextIdString()
    {
        auto id_binary = (m_id += 1u);
        constexpr size_t digits = 11;
        constexpr uint64_t shift = 6;
        constexpr uint64_t mask = (static_cast<uint64_t>(1) << shift) - 1u;
        char buffer[digits + 1];
        size_t i;
        for (i = 0; id_binary && i < digits; ++i)
        {
            auto r = id_binary & mask;
            buffer[i] = ' ' + r;
            id_binary >>= shift;
        }
        buffer[i] = 0;
        return std::string(buffer);
    }

    void HttpClient_Android::CreateClientInstance(JNIEnv* env,
                                                  jobject java_client)
    {
        auto client = std::make_shared<HttpClient_Android>();
        s_client = client;

        client->SetClient(env, java_client);
    }

    void HttpClient_Android::SetJavaVM(JavaVM* vm)
    {
        HttpClient_Android::s_java_vm = vm;
    }

    void HttpClient_Android::DeleteClientInstance(JNIEnv* env)
    {
        s_client.reset();
    }

    void HttpClient_Android::SetCacheFilePath(std::string&& path)
    {
        s_cache_file_path = std::move(path);
    }

    const std::string& HttpClient_Android::GetCacheFilePath()
    {
        return s_cache_file_path;
    }

    JavaVM* HttpClient_Android::s_java_vm = nullptr;

    std::shared_ptr<HttpClient_Android>
    HttpClient_Android::GetClientInstance()
    {
        return std::shared_ptr<HttpClient_Android>(s_client);
    }

    bool HttpClient_Android::CheckException(JNIEnv* env, HttpRequest* request)
    {
        if (env->ExceptionCheck() == JNI_FALSE)
        {
            return false;
        }
        env->ExceptionDescribe();
        env->ExceptionClear();
        CallbackForCancel(env, request);
        return true;
    }

    std::shared_ptr<HttpClient_Android> HttpClient_Android::s_client;
    std::string HttpClient_Android::s_cache_file_path;

}
MAT_NS_END

extern "C" JNIEXPORT void

    JNICALL
    Java_com_microsoft_applications_events_HttpClient_createClientInstance(JNIEnv* env,
                                                                           jobject java_client)
{
    Microsoft::Applications::Events::HttpClient_Android::CreateClientInstance(env, java_client);
}

extern "C" JNIEXPORT void

    JNICALL
    Java_com_microsoft_applications_events_HttpClient_deleteClientInstance(JNIEnv* env)
{
    Microsoft::Applications::Events::HttpClient_Android::DeleteClientInstance(env);
}

extern "C" JNIEXPORT void

    JNICALL
    Java_com_microsoft_applications_events_HttpClient_setCacheFilePath(JNIEnv* env,
                                                                       jobject /* this */,
                                                                       jstring path)
{
    auto utf = env->GetStringUTFChars(path, nullptr);
    std::string converted(utf);
    env->ReleaseStringUTFChars(path, utf);
    Microsoft::Applications::Events::HttpClient_Android::SetCacheFilePath(std::move(converted));
}

extern "C" JNIEXPORT void

    JNICALL
    Java_com_microsoft_applications_events_HttpClient_dispatchCallback(
        JNIEnv* env,
        jobject /* this */,
        jstring id,
        jint statusCode,
        jobjectArray headers,
        jbyteArray body)
{
    size_t id_length = env->GetStringUTFLength(id);
    auto id_utf = env->GetStringUTFChars(id, nullptr);
    std::string id_string(id_utf, id_utf + id_length);
    env->ReleaseStringUTFChars(id, id_utf);
    using HttpClient_Android = Microsoft::Applications::Events::HttpClient_Android;
    auto client = HttpClient_Android::GetClientInstance();
    auto request = client->GetAndRemoveRequest(id_string);
    if (!request)
    {
        return;
    }
    auto callback = request->GetCallback();
    auto response = new Microsoft::Applications::Events::HttpClient_Android::HttpResponse(request->GetId());
    response->SetResponse(statusCode);

    size_t n_headers = env->GetArrayLength(headers);
    for (size_t i = 0; (i + 1u) < n_headers; i += 2)
    {
        auto k = static_cast<jstring>(env->GetObjectArrayElement(headers, i));
        auto v = static_cast<jstring>(env->GetObjectArrayElement(headers, i + 1));
        const char* k_utf = env->GetStringUTFChars(k, nullptr);
        std::string key(k_utf, env->GetStringUTFLength(k));
        env->ReleaseStringUTFChars(k, k_utf);
        const char* v_utf = env->GetStringUTFChars(v, nullptr);
        std::string value(v_utf, env->GetStringUTFLength(v));
        env->ReleaseStringUTFChars(v, v_utf);
        response->AddHeader(std::move(key), std::move(value));
    }
    auto body_pointer = env->GetByteArrayElements(body, nullptr);
    response->SetBody(env->GetArrayLength(body),
                      reinterpret_cast<uint8_t*>(body_pointer));
    env->ReleaseByteArrayElements(body, body_pointer, JNI_ABORT);
    // callback will own response
    callback->OnHttpResponse(response);
}
