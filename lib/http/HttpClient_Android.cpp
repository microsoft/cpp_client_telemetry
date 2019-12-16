#include "HttpClient_Android.hpp"
#include <algorithm>
#include <jni.h>
#include <cstdio>
#include <sstream>
#include <vector>

namespace ARIASDK_NS_BEGIN {

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
	m_body = body; // copy assignment for Great Copy Fun
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
	return m_body.size(); // nope, but what the heck
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
	m_client = nullptr; // well, why not?
}

IHttpRequest* HttpClient_Android::CreateRequest()
{
	HttpRequest * local_request(new HttpRequest(*this));
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

	auto header_buffer = env->NewByteArray(static_cast<jsize>(total_size));
	size_t offset = 0;
	std::vector<jint> index_buffer;
	index_buffer.reserve(2u * headers.size());
	for (auto&& u : headers)
	{
		auto key_len = u.first.length();
		index_buffer.push_back(key_len);
		if (key_len > 0)
		{
			env->SetByteArrayRegion(header_buffer, offset, key_len, reinterpret_cast<jbyte const*>(u.first.data()));
		}
		offset += key_len;
		auto value_len = u.second.length();
		index_buffer.push_back(value_len);
		if (value_len > 0)
		{
			env->SetByteArrayRegion(header_buffer, offset, value_len, reinterpret_cast<jbyte const*>(u.second.data()));
		}
		offset += value_len;
	}
	auto header_lengths = env->NewIntArray(static_cast<jsize>(index_buffer.size()));
	env->SetIntArrayRegion(header_lengths, 0, index_buffer.size(), index_buffer.data());

	auto body = env->NewByteArray(r->m_body.size());
	env->SetByteArrayRegion(body, 0, r->m_body.size(),
		reinterpret_cast<const jbyte*>(r->m_body.data()));
	auto java_id = env->NewStringUTF(request->GetId().c_str());
	auto result = env->CallObjectMethod(m_client, m_create_id,
		env->NewStringUTF(r->m_url.c_str()),
		env->NewStringUTF(r->m_method.c_str()),
		body,
		java_id,
		header_lengths,
		header_buffer);
	HttpRequest* cancel_me = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_requestsMutex);
		for (auto&& u : m_requests)
		{
			if (u->m_id == target_id)
			{
				u->m_callback = callback;
				u->m_java_request = env->NewGlobalRef(result);
				if (!result || u->m_cancel_request)
				{
					cancel_me = u;
					u = m_requests.back();
					m_requests.pop_back();
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
				if (u->m_cancel_request)
				{
					return; // someone already called cancel for this request
				}
				if (u->m_callback)
				{
					cancel_me = u;
				}
				else
				{
					u->m_cancel_request = true;
				}
				break;
			}
		}
	}

	if (cancel_me)
	{
		CallbackForCancel(env, std::move(cancel_me));
	}
}

void HttpClient_Android::CancelAllRequests()
{
	JNIEnv* env;
	if (s_java_vm->AttachCurrentThread(&env, nullptr) != JNI_OK)
	{
		return;
	}

	// We will cancel, notify, and destroy all requests, so swap
	// them into a local vector inside the mutex.

	std::vector<HttpRequest *> local_requests;
	{
		std::lock_guard<std::mutex> lock(m_requestsMutex);
		local_requests.swap(m_requests);
	}
	jclass request_class = nullptr;
	jmethodID cancel_method = nullptr;
	jmethodID get_method = nullptr;
	for (const auto& u : local_requests)
	{
		if (u->m_java_request)
		{
			if (!request_class)
			{
				request_class = env->GetObjectClass(u->m_java_request);
			}
			if (!cancel_method)
			{
				cancel_method = env->GetMethodID(request_class, "cancel", "(Z)Z");
			}
			jboolean interrupt_yes = JNI_TRUE;
			env->CallBooleanMethod(u->m_java_request, cancel_method, interrupt_yes);
			if (!get_method)
			{
				get_method = env->GetMethodID(request_class, "get", "()Z");
			}
			// wait for the request to complete
			env->CallBooleanMethod(u->m_java_request, get_method);
		}
		if (u->m_callback)
		{
			auto failure = new HttpResponse(u->m_id);
			u->m_callback->OnHttpResponse(failure);
		}
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

HttpClient_Android::HttpRequest* HttpClient_Android::GetRequest(std::string id)
{
	std::lock_guard<std::mutex> lock(m_requestsMutex);
	for (auto&& u : m_requests)
	{
		if (u->m_id == id)
		{
			return u;
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

JavaVM* HttpClient_Android::s_java_vm = nullptr;

std::shared_ptr<HttpClient_Android>
HttpClient_Android::GetClientInstance()
{
	return std::shared_ptr<HttpClient_Android>(s_client);
}

std::shared_ptr<HttpClient_Android> HttpClient_Android::s_client;
std::mutex HttpClient_Android::s_client_mutex;

} ARIASDK_NS_END

extern "C"
JNIEXPORT void

JNICALL
Java_com_microsoft_office_ariasdk_httpClient_createClientInstance(JNIEnv* env,
	jobject java_client)
{
	Microsoft::Applications::Events::HttpClient_Android::CreateClientInstance(env, java_client);
}

extern "C"
JNIEXPORT void

JNICALL
Java_com_microsoft_office_ariasdk_httpClient_deleteClientInstance(JNIEnv* env)
{
	Microsoft::Applications::Events::HttpClient_Android::DeleteClientInstance(env);
}

extern "C"
JNIEXPORT void

JNICALL
Java_com_microsoft_office_ariasdk_Request_nativeDispatchCallback(
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
	auto request = client->GetRequest(id_string);
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