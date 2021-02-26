//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include <IHttpClient.hpp>
#include <atomic>
#include <jni.h>
#include <mutex>
#include <string>

namespace MAT_NS_BEGIN
{
    class HttpClient_Android : public IHttpClient
    {
        enum class RequestState : int8_t
        {
            early = 0,
            preparing,
            running,
            cancel
        };

       public:
        class HttpResponse : public IHttpResponse
        {
           public:
            HttpResponse(const std::string& id) :
                m_id(id)
            {
            }

            const std::string& GetId() const override
            {
                return m_id;
            }

            HttpResult GetResult() const override
            {
                switch (m_response)
                {
                case 0:
                    return HttpResult_LocalFailure;
                case -1:
                    return HttpResult_NetworkFailure;
                default:
                    return HttpResult_OK;
                }
            }

            unsigned int GetStatusCode() const override
            {
                return m_response;
            }

            const HttpHeaders& GetHeaders() const override
            {
                return m_headers;
            }

            const std::vector<uint8_t, std::allocator<uint8_t>>& GetBody() const override
            {
                return m_body;
            }

            void SetResponse(int response)
            {
                m_response = response;
            }

            void AddHeader(std::string&& key, std::string&& value)
            {
                m_headers.emplace(std::move(key), std::move(value));
            }

            void SetBody(size_t length, const uint8_t* body)
            {
                m_body.assign(body, body + length);
            }

           private:
            std::string m_id;
            HttpHeaders m_headers;
            std::vector<uint8_t, std::allocator<uint8_t>> m_body;
            int m_response = 0;
        };

       public:
        class HttpRequest : public IHttpRequest
        {
           public:
            HttpRequest() = delete;

            HttpRequest(HttpClient_Android& parent_) :
                m_parent(parent_), m_id(parent_.NextIdString())
            {
            }

            ~HttpRequest() noexcept;

            /// <summary>
            /// Gets the request ID.
            /// </summary>
            const std::string& GetId() const override
            {
                return m_id;
            }

            /// <summary>
            /// Sets the request method
            /// </summary>
            /// <param name="method">A string that contains the the name of the method to set (e.g., <i>GET</i>).</param>
            void SetMethod(std::string const& method) override;

            /// <summary>
            /// Sets the request URI.
            /// </summary>
            /// <param name="url">A string that contains the URI to set.</param>
            void SetUrl(std::string const& url) override;

            /// <summary>
            /// Gets the request headers.
            /// </summary>
            /// <returns>The HTTP headers in an HttpHeaders object.</returns>
            HttpHeaders& GetHeaders() override;

            /// <summary>
            /// Sets the request body.
            /// </summary>
            /// <param name="body">A standard vector that contains the message body.</param>
            void SetBody(std::vector<uint8_t>& body) override;

            /// <summary>
            /// Gets the request body.
            /// </summary>
            std::vector<uint8_t>& GetBody() override;

            /// <summary>
            /// Sets the request latency.
            /// </summary>
            /// <param name="priority">The event latency, as one of the EventLatency enumeration values.</param>
            void SetLatency(EventLatency latency) override;

            /// <summary>
            /// Gets the size of the request message body.
            /// </summary>
            /// <returns>The size of the request message body, in bytes.</returns>
            size_t GetSizeEstimate() const override;

            IHttpResponseCallback* GetCallback() const
            {
                return m_callback;
            }

            void EraseFromParent()
            {
                m_parent.EraseRequest(this);
            }

           protected:
            HttpClient_Android& m_parent;
            HttpHeaders m_headers;
            IHttpResponseCallback* m_callback = nullptr;
            std::string m_id;
            std::string m_method;
            std::string m_url;
            std::vector<uint8_t> m_body;
            jobject m_java_request = nullptr;
            RequestState m_state = RequestState::early;

            friend HttpClient_Android;
        };

       public:
        HttpClient_Android();
        ~HttpClient_Android();
        IHttpRequest* CreateRequest() override;
        void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
        void CancelRequestAsync(std::string const& id) override;
        void CancelAllRequests() override;
        void SetClient(JNIEnv* env, jobject c);
        void EraseRequest(HttpRequest*);
        HttpRequest* GetAndRemoveRequest(std::string id);
        std::string NextIdString();

        static void CreateClientInstance(JNIEnv* env,
                                         jobject java_client);
        static void DeleteClientInstance(JNIEnv* env);
        static void SetCacheFilePath(std::string&& path);
        static const std::string& GetCacheFilePath();
        static std::shared_ptr<HttpClient_Android> GetClientInstance();
        void CallbackForCancel(JNIEnv* env, HttpRequest* t);
        static void SetJavaVM(JavaVM* vm);

       protected:
        std::mutex m_requestsMutex;
        std::vector<HttpRequest*> m_requests;

        jobject m_client = nullptr;
        jclass m_client_class = nullptr;
        jmethodID m_send_id = nullptr;
        jmethodID m_create_id = nullptr;
        jmethodID m_execute_id = nullptr;
        static JavaVM* s_java_vm;
        std::atomic<uint64_t> m_id;
        static std::shared_ptr<HttpClient_Android> s_client;
        static std::string s_cache_file_path;

        bool CheckException(JNIEnv* env, HttpRequest* request);
    };

}
MAT_NS_END
