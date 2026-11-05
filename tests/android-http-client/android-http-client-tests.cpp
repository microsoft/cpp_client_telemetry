//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "http/HttpClient_Android.hpp"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

using namespace Microsoft::Applications::Events;

namespace
{
    JavaVM* vm = nullptr;
    std::thread worker;
    std::mutex mutex;
    std::condition_variable changed;
    bool ready = false;
    bool finish = false;
    bool passed = false;

    class Callback : public IHttpResponseCallback
    {
       public:
        void OnHttpResponse(IHttpResponse* response) override
        {
            std::unique_ptr<IHttpResponse> owned(response);
            result = response->GetResult();
            if (deleteRequest)
            {
                delete request;
                request = nullptr;
            }
            ++calls;
        }

        IHttpRequest* request = nullptr;
        bool deleteRequest = false;
        std::atomic<int> calls{0};
        HttpResult result = HttpResult_LocalFailure;
    };

    void RunBatch(jint mode, jint count)
    {
        JNIEnv* attached = nullptr;
        vm->AttachCurrentThread(&attached, nullptr);
        auto nativeClient = HttpClient_Android::GetClientInstance();
        std::vector<std::unique_ptr<Callback>> callbacks;
        for (int i = 0; i < count; ++i)
        {
            std::unique_ptr<Callback> callback(new Callback);
            callback->request = nativeClient->CreateRequest();
            callback->request->SetUrl("https://unused.invalid/");
            callback->request->SetMethod("POST");
            callback->request->GetHeaders().set("test", "value");
            std::vector<uint8_t> body{1, 2, 3};
            callback->request->SetBody(body);
            callback->deleteRequest = mode == 8;
            if (mode == 4)
            {
                nativeClient->CancelRequestAsync(callback->request->GetId());
            }
            nativeClient->SendRequestAsync(callback->request, callback.get());
            if (mode == 2 || mode == 9 || mode == 11)
            {
                nativeClient->CancelRequestAsync(callback->request->GetId());
            }
            if (mode == 3)
            {
                callbacks.push_back(std::move(callback));
            }
            else
            {
                if (mode == 10 || mode == 11)
                {
                    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
                    while (callback->calls == 0 && std::chrono::steady_clock::now() < deadline)
                    {
                        std::this_thread::yield();
                    }
                }
                auto expected = HttpResult_Aborted;
                if (mode == 0 || mode == 8 || mode == 10)
                {
                    expected = HttpResult_OK;
                }
                else if (mode == 1)
                {
                    expected = HttpResult_NetworkFailure;
                }
                passed = passed && callback->calls == 1 &&
                         (callback->result == expected || (mode == 11 && callback->result == HttpResult_OK));
                callbacks.push_back(std::move(callback));
            }
            passed = passed && !attached->ExceptionCheck();
        }
        if (mode == 3)
        {
            nativeClient->CancelAllRequests();
            for (auto const& callback : callbacks)
            {
                passed = passed && callback->calls == 1 && callback->result == HttpResult_Aborted;
            }
        }
        {
            std::unique_lock<std::mutex> lock(mutex);
            ready = true;
            changed.notify_all();
            while (!finish)
            {
                changed.wait(lock);
            }
        }
        // Keep native requests AND the attached thread alive until Java checks weak references.
        for (auto const& callback : callbacks)
        {
            delete callback->request;
        }
        nativeClient.reset();
        vm->DetachCurrentThread();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_HttpClient_cancelPreparing(JNIEnv* env, jobject, jstring id)
{
    const char* text = env->GetStringUTFChars(id, nullptr);
    HttpClient_Android::GetClientInstance()->CancelRequestAsync(text);
    env->ReleaseStringUTFChars(id, text);
}

extern "C" JNIEXPORT void JNICALL
Java_com_microsoft_applications_events_HttpClient_startBatch(JNIEnv* env, jobject client, jint mode, jint count)
{
    env->GetJavaVM(&vm);
    HttpClient_Android::CreateClientInstance(env, client);
    ready = false;
    finish = false;
    passed = true;
    worker = std::thread(RunBatch, mode, count);
    std::unique_lock<std::mutex> lock(mutex);
    while (!ready)
    {
        changed.wait(lock);
    }
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_microsoft_applications_events_HttpClient_finishBatch(JNIEnv* env, jobject client)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        finish = true;
        changed.notify_all();
    }
    worker.join();
    HttpClient_Android::DeleteClientInstance(env, client);
    return passed ? JNI_TRUE : JNI_FALSE;
}
