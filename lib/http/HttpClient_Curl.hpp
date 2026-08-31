//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef HTTPCLIENTCURL_HPP
#define HTTPCLIENTCURL_HPP

#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string.h>
#include <regex>

#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <map>

#include <algorithm>
#include <numeric>
#include <limits>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <utility>

#include <poll.h>
#include <curl/curl.h>

#include <unistd.h>

#include "IHttpClient.hpp"
#include "IBoundedHttpClientCancel.hpp"
#include "pal/PAL.hpp"

#ifdef HAVE_ONEDS_BOUNDCHECK_METHODS
#include "utils/annex_k.hpp"
#endif

#define HTTP_CONN_TIMEOUT       5L
#define HTTP_STATUS_REGEXP		"HTTP\\/\\d\\.\\d (\\d+)\\ .*"
#define HTTP_HEADER_REGEXP      "(.*)\\: (.*)\\n*"

#undef TRACE
#define TRACE(...)	// printf

namespace MAT_NS_BEGIN {

/**
 * Perform libcurl's process-wide initialization exactly once.
 *
 * curl_global_init() is not thread-safe on the libcurl versions this SDK
 * supports, and it must run before any other libcurl entry point. Every code
 * path that can be the process's first libcurl user -- the HttpClient_Curl
 * facade and a directly constructed CurlHttpOperation -- funnels through this
 * function. The C++11 function-local static guarantees the initializer runs
 * exactly once per process and that concurrent first callers block until it
 * has completed, so overlapping client construction cannot race.
 *
 * There is deliberately no matching curl_global_cleanup() anywhere in the SDK.
 * libcurl's global state is process-wide and shared with every other static
 * libcurl user in the host process: the application itself, other SDKs, and
 * plugins that may be loaded after this library. This SDK cannot observe those
 * users, so it cannot know when the last one is finished, which makes teardown
 * unknowable from here. Releasing the global state when a telemetry client is
 * destroyed would pull it out from under an unrelated component (and, worse,
 * out from under this SDK's own in-flight transfers). Leaving it initialized
 * for the life of the process is the only correct choice for an embedded
 * library; the host may still call curl_global_cleanup() itself at exit.
 */
inline void EnsureCurlGlobalInit() noexcept
{
    static const CURLcode initResult = curl_global_init(CURL_GLOBAL_ALL);
    (void)initResult;
}

// Private per-client shared state. Defined in HttpClient_Curl.cpp: it owns the
// operation registry, the drain bookkeeping and the SSL settings, and it
// outlives the facade because every completion captures it by shared_ptr.
struct CurlClientState;

/**
 * Curl-based HTTP client
 */
class HttpClient_Curl : public IHttpClient, public IBoundedHttpClientCancel {
public:
    HttpClient_Curl();
    virtual ~HttpClient_Curl();

    virtual IHttpRequest* CreateRequest() override;
    virtual void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override;
    virtual void CancelRequestAsync(std::string const& id) override;

    // Full drain: returns once every tracked operation has delivered its
    // terminal callback and has been destroyed, unless the caller is itself
    // running inside one of this client's callbacks (see the implementation).
    virtual void CancelAllRequests() override;
    // Soft-bounded drain: stops initiating further cancellations at the
    // deadline and may return while an operation and the shared state are
    // still alive.
    virtual void CancelAllRequests(std::chrono::milliseconds bestEffortTimeout) override;

    virtual void ApplySettings(ILogConfiguration& config) override;
    // sslVerify is retained for source compatibility, but false is ignored:
    // production transports always verify the peer certificate and hostname.
    void SetSslVerification(bool sslVerify, const std::string& caInfo = "");

private:
    std::shared_ptr<CurlClientState> m_state;
};

class CurlHttpOperation {
public:
    struct CallbackHooks
    {
        std::function<void()> begin;
        std::function<void()> end;
    };

    struct WorkerHooks
    {
        std::function<void()> begin;
        std::function<void()> end;
    };

private:
    class CallbackScope
    {
    public:
        explicit CallbackScope(CallbackHooks const& hooks)
            : m_hooks(hooks)
        {
            if (m_hooks.begin != nullptr)
            {
                m_hooks.begin();
                m_started = true;
            }
        }

        ~CallbackScope() noexcept
        {
            if (m_started && m_hooks.end != nullptr)
            {
                try
                {
                    m_hooks.end();
                }
                catch (...)
                {
                }
            }
        }

        CallbackScope(CallbackScope const&) = delete;
        CallbackScope& operator=(CallbackScope const&) = delete;

    private:
        CallbackHooks const& m_hooks;
        bool m_started {false};
    };

    class WorkerScope
    {
    public:
        explicit WorkerScope(WorkerHooks const& hooks)
            : m_hooks(hooks)
        {
            if (m_hooks.begin != nullptr)
            {
                m_hooks.begin();
                m_started = true;
            }
        }

        ~WorkerScope() noexcept
        {
            if (m_started && m_hooks.end != nullptr)
            {
                try
                {
                    m_hooks.end();
                }
                catch (...)
                {
                }
            }
        }

        WorkerScope(WorkerScope const&) = delete;
        WorkerScope& operator=(WorkerScope const&) = delete;

    private:
        WorkerHooks const& m_hooks;
        bool m_started {false};
    };

public:
    void DispatchEvent(HttpStateEvent type)
    {
        if (m_callback != nullptr)
        {
            CallbackScope callbackScope(m_callbackHooks);
            m_callback->OnHttpStateEvent(type, static_cast<void*>(curl), 0);
        }
    }

    // Replays the creation state event (OnCreated / OnCreateFailed) that
    // construction deferred (see the deferCreationEvent constructor parameter).
    // A no-op for a directly constructed operation, which dispatches its
    // creation event during construction. Dispatching here -- after the caller
    // has registered the operation -- is what lets a reentrant
    // CancelRequestAsync/CancelAllRequests fired from the creation callback find
    // and abort this operation before any network work starts. The dispatch is
    // accounted through the operation's callback hooks, exactly like every other
    // state event, so a concurrent drain sees it.
    void DispatchDeferredCreationEvent()
    {
        if (m_hasPendingCreationEvent)
        {
            m_hasPendingCreationEvent = false;
            DispatchEvent(m_pendingCreationEvent);
        }
    }

    std::atomic<bool> isAborted { false };      // Set to 'true' when async callback is aborted
    /**
     * Create local CURL instance for url and body
     *
     * @param url
     * @param body
     * @param httpConnTimeout   HTTP connection timeout in seconds
     * @param httpReadTimeout   HTTP read timeout in seconds
     */
    // Selects HTTP/2 only when the libcurl we are actually linked against was
    // built with HTTP/2 support. Setting CURLOPT_HTTP_VERSION to
    // CURL_HTTP_VERSION_2_0 against a libcurl without HTTP/2 does not silently
    // downgrade -- it fails the transfer with CURLE_UNSUPPORTED_PROTOCOL -- so
    // the version has to be probed at runtime rather than assumed.
    static long GetPreferredHttpVersion() noexcept
    {
        const curl_version_info_data* versionInfo = curl_version_info(CURLVERSION_NOW);
        if (versionInfo != nullptr && (versionInfo->features & CURL_VERSION_HTTP2) != 0)
        {
            return CURL_HTTP_VERSION_2_0;
        }
        return CURL_HTTP_VERSION_1_1;
    }

    CurlHttpOperation(
            std::string method,
            std::string url,
            IHttpResponseCallback* callback,
            // requestHeaders and requestBody are copied into operation-owned storage
            // so the worker does not depend on the caller retaining the request.
            const std::map<std::string, std::string>& requestHeaders,
            const std::vector<uint8_t>& requestBody,
            // Default connectivity and response size options
            bool rawResponse                                         = false,
            size_t httpConnTimeout                                   = HTTP_CONN_TIMEOUT,
            // SSL certificate verification options
            bool sslVerify                                           = true,
            const std::string& sslCaInfo                             = "",
            CallbackHooks callbackHooks                              = CallbackHooks(),
            WorkerHooks workerHooks                                  = WorkerHooks(),
            // When true (client-created, tracked operations), the OnCreated /
            // OnCreateFailed state event is not dispatched during construction.
            // It is recorded and replayed later by DispatchDeferredCreationEvent()
            // once the operation has been registered, so a reentrant
            // CancelRequestAsync/CancelAllRequests fired from that event can find
            // the operation. A directly constructed operation keeps the historical
            // immediate-dispatch behavior.
            bool deferCreationEvent                                  = false) :

            // Optional connection params
            rawResponse(rawResponse),
            httpConnTimeout(httpConnTimeout),

            m_callback(callback),
            m_method(method),
            m_url(url),
            m_sslCaInfo(sslCaInfo),
            m_callbackHooks(std::move(callbackHooks)),
            m_workerHooks(std::move(workerHooks)),
            m_deferCreationEvent(deferCreationEvent),

            // Local vars
            m_requestBody(requestBody)
    {
        // sslVerify is retained for source compatibility. Disabling TLS
        // authentication is never permitted by the production transport.
        (void)sslVerify;

        TRACE("--------------------------------------------------------------------------------------------------\n");
        response.memory = nullptr;
        response.size = 0;

        // A directly constructed operation may be the process's first libcurl
        // user, so it shares the client's init-once rather than assuming an
        // HttpClient_Curl was built first.
        EnsureCurlGlobalInit();

        /* get a curl handle */
        curl = curl_easy_init();
        if(!curl)
        {
            TRACE("libcurl failed to init!\n");
            m_transportError = CURLE_FAILED_INIT;
            m_setupError = CURLE_FAILED_INIT;
            EmitCreationEvent(OnCreateFailed);
            return;
        }

        if (!SetOption(CURLOPT_VERBOSE, 0L) ||
            !SetOption(CURLOPT_URL, m_url.c_str()) ||
            !SetOption(CURLOPT_SSL_VERIFYPEER, 1L) ||
            !SetOption(CURLOPT_SSL_VERIFYHOST, 2L) ||
            (!m_sslCaInfo.empty() && !SetOption(CURLOPT_CAINFO, m_sslCaInfo.c_str())) ||
            // The worker is one thread of a host process this SDK does not own:
            // never let libcurl install process-wide signal handlers or use
            // SIGALRM-based timeouts.
            !SetOption(CURLOPT_NOSIGNAL, 1L) ||
            // The progress callback is the only cancellation channel that is
            // safe to trigger from another thread: it runs on the worker,
            // inside libcurl, and aborts the transfer in an orderly way.
            !SetOption(CURLOPT_NOPROGRESS, 0L) ||
            !SetAbortProgressOption() ||
            // HTTP/2 when the linked libcurl supports it, otherwise HTTP/1.1
            !SetOption(CURLOPT_HTTP_VERSION, GetPreferredHttpVersion()))
        {
            EmitCreationEvent(OnCreateFailed);
            return;
        }

        // Do not override libcurl's shipped connect timeout. With NOSIGNAL,
        // a synchronous resolver may still block before libcurl can invoke the
        // progress callback; cancellation is therefore observed once libcurl
        // returns to its transfer loop, not while that resolver call is active.

        // Headers are copied into m_headersChunk during construction and the
        // curl_slist is kept alive until destruction, so the original map does
        // not need operation-lifetime storage.
        for (const auto& kv : requestHeaders)
        {
            std::string header = kv.first + ": " + kv.second;
            curl_slist* appendedHeaders = curl_slist_append(m_headersChunk, header.c_str());
            if (appendedHeaders == nullptr)
            {
                m_transportError = CURLE_OUT_OF_MEMORY;
                m_setupError = CURLE_OUT_OF_MEMORY;
                EmitCreationEvent(OnCreateFailed);
                return;
            }
            m_headersChunk = appendedHeaders;
        }

        if (m_headersChunk != nullptr && !SetOption(CURLOPT_HTTPHEADER, m_headersChunk))
        {
            EmitCreationEvent(OnCreateFailed);
            return;
        }
        TRACE("method=%s, url=%s\n", this->m_method.c_str(), this->m_url.c_str());

        EmitCreationEvent(OnCreated);
    }

    /**
     * Destroy CURL instance
     */
    virtual ~CurlHttpOperation()
    {
        if (m_worker.joinable())
        {
            if (m_worker.get_id() == std::this_thread::get_id())
            {
                // The completion callback can release the owning request on this
                // worker. Detach rather than joining the current thread; Send() has
                // finished and the worker does not touch this operation afterward.
                m_worker.detach();
            }
            else
            {
                m_worker.join();
            }
        }

        DispatchDestroyEvent();
        m_transportError = CURLE_OK;
        if (curl != nullptr)
        {
            curl_easy_cleanup(curl);
        }
        if (m_headersChunk != nullptr)
        {
            curl_slist_free_all(m_headersChunk);
        }
        ReleaseResponse();
    }

    /**
     * Send request synchronously
     */
    void Send()
    {
        TRACE("method=%s\n", this->m_method.c_str());

        ReleaseResponse();
        // Request buffer
        const void *request  = m_requestBody.empty() ? nullptr : m_requestBody.data();
        const size_t reqSize = m_requestBody.size();
        long httpStatusCode = 0;
        CURLcode infoResult = CURLE_OK;

        if(!curl)
        {
            m_transportError = CURLE_FAILED_INIT;
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }
        if (m_setupError != CURLE_OK)
        {
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }
        if (isAborted)
        {
            // Cancelled before the worker reached the network. Do not open a
            // connection; the terminal result is Aborted either way.
            m_transportError = CURLE_ABORTED_BY_CALLBACK;
            goto cleanup;
        }

        // TODO: should we control what local source port we use?
        // curl_easy_setopt(curl, CURLOPT_LOCALPORT, dcf_port);

        // Perform initial connect, handling the timeout if needed
        if (!SetOption(CURLOPT_CONNECT_ONLY, 1L))
        {
            DispatchEvent(OnConnectFailed);
            goto cleanup;
        }
        DispatchEvent(OnConnecting);
        m_transportError = curl_easy_perform(curl);
        if(CURLE_OK != m_transportError)
        {
            DispatchEvent(OnConnectFailed);     // couldn't connect - stage 1
            TRACE("Error #1: %s\n", curl_easy_strerror(m_transportError));
            goto cleanup;
        }

        /* Extract the socket from the curl handle - we'll need it for waiting.
         * Note that this API takes a pointer to a 'long' while we use
         * curl_socket_t for sockets otherwise.
         */

#if LIBCURL_VERSION_NUM >= 0x072D00 // Version 7.45.00
        m_transportError = curl_easy_getinfo(curl, CURLINFO_ACTIVESOCKET, &sockextr);
#else
        {
            long lastSocket = -1;
            m_transportError = curl_easy_getinfo(curl, CURLINFO_LASTSOCKET, &lastSocket);
            if (m_transportError == CURLE_OK)
            {
                sockextr = static_cast<curl_socket_t>(lastSocket);
            }
        }
#endif

        if(CURLE_OK != m_transportError)
        {
            DispatchEvent(OnConnectFailed);     // couldn't connect - stage 2
            TRACE("Error #2: %s\n", curl_easy_strerror(m_transportError));
            goto cleanup;
        }
        if (sockextr == CURL_SOCKET_BAD)
        {
            m_transportError = CURLE_FAILED_INIT;
            DispatchEvent(OnConnectFailed);     // couldn't connect - no socket
            TRACE("Error #2: curl returned an invalid socket\n");
            goto cleanup;
        }

        /* wait for the socket to become ready for sending */
        sockfd = sockextr;
        if (WaitOnSocket(sockfd, 0, static_cast<long>(httpConnTimeout) * 1000L) <= 0 || isAborted)
        {
            TRACE("Error #3: timeout, aborted=%u\n", isAborted.load() );
            m_transportError = CURLE_OPERATION_TIMEDOUT;
            DispatchEvent(OnConnectFailed);     // couldn't connect - stage 3
            goto cleanup;
        }

        // once connection is there - switch back to easy perform for HTTP post
        if (!SetOption(CURLOPT_CONNECT_ONLY, 0L))
        {
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }

        // send all data to our callback function
        if (rawResponse)
        {
            if (!SetOption(CURLOPT_HEADER, 1L) ||
                !SetOption(CURLOPT_WRITEFUNCTION, &WriteMemoryCallback) ||
                !SetOption(CURLOPT_WRITEDATA, static_cast<void*>(&response)))
            {
                DispatchEvent(OnSendFailed);
                goto cleanup;
            }
        } else {
            if (!SetOption(CURLOPT_HEADERFUNCTION, &WriteVectorCallback) ||
                !SetOption(CURLOPT_HEADERDATA, static_cast<void*>(&respHeaders)) ||
                !SetOption(CURLOPT_WRITEFUNCTION, &WriteVectorCallback) ||
                !SetOption(CURLOPT_WRITEDATA, static_cast<void*>(&respBody)))
            {
                DispatchEvent(OnSendFailed);
                goto cleanup;
            }
        }

        // TODO: only two methods supported for now - POST and GET
        if (m_method.compare("POST") == 0)
        {
            // POST
            if (!SetOption(CURLOPT_POST, 1L) ||
                !SetOption(CURLOPT_POSTFIELDS, static_cast<const char*>(request)) ||
                !SetOption(CURLOPT_POSTFIELDSIZE_LARGE, static_cast<curl_off_t>(reqSize)))
            {
                DispatchEvent(OnSendFailed);
                goto cleanup;
            }
        } else
        if (m_method.compare("GET") == 0)
        {
            // GET
        } else
        {
            TRACE("Error #4: unsupported method %s\n", m_method.c_str());
            m_transportError = CURLE_UNSUPPORTED_PROTOCOL;
            goto cleanup;
        }

        if (!SetOption(CURLOPT_LOW_SPEED_TIME, 30L) ||
            !SetOption(CURLOPT_LOW_SPEED_LIMIT, 4096L))
        {
            DispatchEvent(OnSendFailed);
            goto cleanup;
        }
        DispatchEvent(OnSending);
        m_transportError = curl_easy_perform(curl);
        if(CURLE_OK != m_transportError)
        {
            DispatchEvent(OnSendFailed);
            TRACE("Error: %s\n", curl_easy_strerror(m_transportError));
            goto cleanup;
        }

        /* Code snippet to parse raw HTTP response. This might come in handy
         * if we ever consider to handle the raw upload instead of curl_easy_perform
       ...
       std::string resp((const char *)response);
       std::regex http_status_regex(HTTP_STATUS_REGEXP);
       std::smatch match;
       if(std::regex_search(resp, match, http_status_regex))
         http_code = std::stol(match[1]);
       ...
         */

        /* libcurl is nice enough to parse the response code itself: */
        infoResult = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);
        if (infoResult != CURLE_OK)
        {
            m_transportError = infoResult;
            DispatchEvent(OnSendFailed);
            TRACE("Error getting HTTP response code: %s\n", curl_easy_strerror(m_transportError));
            goto cleanup;
        }
        m_httpStatusCode = httpStatusCode;
        // We got some response from server. Dump the contents.
        TRACE("HTTP response code %ld\n", httpStatusCode);
        DispatchEvent(OnResponse);

cleanup:
        return;
    }

    void SendAsync(std::function<void(CurlHttpOperation &)> callback = nullptr) {
        // A newly created std::thread may run before it is assigned to m_worker.
        // Hold this gate until the assignment completes so a fast failure cannot
        // destroy the operation from its callback while SendAsync still uses it.
        {
            std::lock_guard<std::mutex> startGuard(m_workerStartMtx);
            if (m_sendAttempted)
            {
                throw std::logic_error("CurlHttpOperation is single-use");
            }
            m_sendAttempted = true;

            try
            {
                m_worker = std::thread([this, callback]() {
                    {
                        std::lock_guard<std::mutex> startGuard(m_workerStartMtx);
                    }
                    {
                        WorkerScope workerScope(m_workerHooks);
                        try
                        {
                            Send();
                        }
                        catch (...)
                        {
                            // std::async stored worker exceptions in its unobserved
                            // future. A raw thread must contain them.
                            m_transportError = CURLE_FAILED_INIT;
                            m_setupError = CURLE_FAILED_INIT;
                        }
                        Complete(callback);
                    }
                });
                return;
            }
            catch (...)
            {
                // Callable allocation/copy or std::thread creation failed.
            }
        }

        m_transportError = CURLE_FAILED_INIT;
        m_setupError = CURLE_FAILED_INIT;
        CompleteWithoutSend(callback);
    }

    void CompleteWithoutSend(const std::function<void(CurlHttpOperation &)>& callback) noexcept
    {
        Complete(callback);
    }

    CURLcode GetTransportError() const
    {
        return m_transportError;
    }

    long GetHttpStatusCode() const
    {
        return m_httpStatusCode;
    }

    /**
     * Get whether or not response was programmatically aborted.
     *
     * Once the outcome has been frozen (at the start of Complete, before the
     * OnDestroy state event runs; see FreezeOutcome) this returns the latched
     * classification rather than the live flag. That is what stops an Abort()
     * triggered from an OnDestroy observer -- which is legitimately allowed to
     * cancel *peers* -- from retroactively turning this operation's already
     * finished, successful transfer into an Aborted one. A cancellation that
     * won before the freeze is captured by the latch and still reported as
     * Aborted.
     */
    bool WasAborted()
    {
        if (m_outcomeFrozen.load(std::memory_order_acquire))
        {
            return m_frozenAborted.load(std::memory_order_relaxed);
        }
        return isAborted.load();
    }

    CURLcode GetSetupError() const
    {
        return m_setupError;
    }

    /**
     * Return a copy of response headers
     *
     * @return
     */
    std::map<std::string, std::string> GetResponseHeaders()
    {
        std::map<std::string, std::string> result;
        if (respHeaders.empty())
        {
            return result;
        }

        std::stringstream ss;
        std::string headers(reinterpret_cast<const char*>(respHeaders.data()), respHeaders.size());
        ss.str(headers);

        std::string header;
        while (std::getline(ss, header, '\n')) {
            std::smatch match;
            std::regex http_headers_regex(HTTP_HEADER_REGEXP);
            if (std::regex_search(header, match, http_headers_regex))
                result[match[1]] = match[2];    // Key: value
        }
        return result;
    }

    /**
     * Return a copy of response body
     *
     * @return
     */
    std::vector<uint8_t> GetResponseBody()
    {
        return respBody;
    }

    /**
     * Return a raw copy of response headers+body
     *
     * @return
     */
    std::vector<uint8_t> GetRawResponse()
    {
        std::vector<uint8_t> result;
        if ((response.memory != nullptr) && (response.size != 0))
        {
            const auto* begin = reinterpret_cast<const uint8_t*>(response.memory);
            result.insert(result.end(), begin, begin + response.size);
        }
        return result;
    }

    /**
     * Release memory allocated for response
     */
    void ReleaseResponse()
    {
        if (response.memory != nullptr) {
            free(response.memory);
            response.memory = nullptr;
            response.size = 0;
        }
        respHeaders.clear();
        respBody.clear();
    }

    /**
     * Request cancellation of a request that is connecting or transferring.
     *
     * This raises a flag and nothing else. It deliberately does not close the
     * socket: the descriptor is owned by the worker thread and by libcurl, and
     * closing it from another thread races with libcurl's own close. After that
     * race the descriptor number can be handed straight back out by the kernel,
     * so a late close tears down an unrelated connection somewhere else in the
     * host process. The worker observes the flag from libcurl's progress
     * callback and from its poll loop and unwinds the transfer on the thread
     * that owns it. The terminal result stays Aborted because WasAborted()
     * wins over whatever CURLcode the unwind produces.
     */
    void Abort()
    {
        isAborted.store(true, std::memory_order_release);
    }

    CURL *GetHandle()
    {
        return curl;
    }

protected:
    const bool   rawResponse;       // Do not split response headers from response body
    const size_t httpConnTimeout;   // Timeout for connect.  Default: 5s

    CURL *curl;                     // Local curl instance
    CURLcode m_transportError = CURLE_OK;
    CURLcode m_setupError = CURLE_OK;
    long m_httpStatusCode = 0;

    IHttpResponseCallback* m_callback = nullptr;

    // Request values
    std::string m_method;
    std::string m_url;
    std::string m_sslCaInfo;
    CallbackHooks m_callbackHooks;
    WorkerHooks m_workerHooks;
    // Deferred creation-event bookkeeping (see the deferCreationEvent ctor arg
    // and DispatchDeferredCreationEvent). m_deferCreationEvent is fixed at
    // construction; the pending fields are only touched on the caller thread
    // before the worker exists, so they need no synchronization.
    bool m_deferCreationEvent;
    bool m_hasPendingCreationEvent {false};
    HttpStateEvent m_pendingCreationEvent {OnCreated};
    // Own the payload so operation lifetime is independent of CurlHttpRequest.
    std::vector<uint8_t> m_requestBody;
    struct curl_slist *m_headersChunk = nullptr;

    // Processed response headers and body
    std::vector<uint8_t>        respHeaders;
    std::vector<uint8_t>        respBody;

    // Socket parameters
    // Owned exclusively by the worker thread; CURL_SOCKET_BAD is the "no
    // socket" sentinel (0 is a valid descriptor number).
    curl_socket_t sockfd = CURL_SOCKET_BAD;

    curl_socket_t sockextr = CURL_SOCKET_BAD;

    curl_off_t nread = 0;
    size_t sendlen   = 0;        // # bytes sent by client
    size_t acklen    = 0;        // # bytes ack by server

    std::mutex m_workerStartMtx;
    bool m_sendAttempted = false;
    std::thread m_worker;
    std::atomic<bool> m_destroyEventDispatched { false };

    // Latched cancellation classification. Frozen once, at the very start of
    // completion, before the OnDestroy state event can run. Only the
    // cancellation outcome is latched -- transport/setup/status fields stay
    // live -- because those are already final by completion, while isAborted is
    // the one input an OnDestroy observer can still legally flip (when it
    // cancels peers) after this transfer has already succeeded.
    std::atomic<bool> m_outcomeFrozen { false };
    std::atomic<bool> m_frozenAborted { false };

    // Snapshot the abort classification exactly once. After this returns,
    // WasAborted() reports the latched value regardless of any later Abort().
    void FreezeOutcome() noexcept
    {
        if (!m_outcomeFrozen.load(std::memory_order_acquire))
        {
            m_frozenAborted.store(isAborted.load(std::memory_order_acquire), std::memory_order_relaxed);
            m_outcomeFrozen.store(true, std::memory_order_release);
        }
    }

    // Dispatch the creation event immediately, or record it for later replay
    // when the operation was constructed in deferred mode.
    void EmitCreationEvent(HttpStateEvent type)
    {
        if (m_deferCreationEvent)
        {
            m_pendingCreationEvent = type;
            m_hasPendingCreationEvent = true;
            return;
        }
        DispatchEvent(type);
    }

    void DispatchDestroyEvent() noexcept
    {
        if (!m_destroyEventDispatched.exchange(true, std::memory_order_acq_rel))
        {
            try
            {
                DispatchEvent(OnDestroy);
            }
            catch (...)
            {
                // State observers must not terminate the worker or destructor.
            }
        }
    }

    void Complete(const std::function<void(CurlHttpOperation &)>& callback) noexcept
    {
        // Latch the cancellation outcome before the OnDestroy event fires. The
        // operation is still in the registry here, so an OnDestroy observer may
        // reenter CancelAllRequests/CancelRequestAsync and Abort() this object;
        // freezing first guarantees response mapping sees the outcome as it was
        // when the transfer actually finished, not as a late cancel rewrote it.
        FreezeOutcome();
        // Preserve the documented state event while m_callback is still valid.
        // The completion callback can release the last owner, so this must remain
        // the worker's final access to the operation.
        DispatchDestroyEvent();
        try
        {
            if (callback != nullptr)
            {
                callback(*this);
            }
        }
        catch (...)
        {
            // Match the old unobserved-future behavior at the thread boundary.
        }
    }

    template <typename T>
    bool SetOption(CURLoption option, T value)
    {
        if (curl == nullptr)
        {
            m_transportError = CURLE_FAILED_INIT;
            m_setupError = CURLE_FAILED_INIT;
            return false;
        }

        const CURLcode optionResult = curl_easy_setopt(curl, option, value);
        if (optionResult == CURLE_OK)
        {
            return true;
        }

        LOG_WARN("curl_easy_setopt(%d) failed: %s", static_cast<int>(option), curl_easy_strerror(optionResult));
        m_transportError = optionResult;
        m_setupError = optionResult;
        return false;
    }

    /**
     * Helper routine to wait for data on socket.
     *
     * Polls in short slices instead of one long sleep so a cancellation flagged
     * on another thread is observed within a bounded delay, without anybody
     * closing the descriptor the worker owns.
     *
     * @param socket
     * @param for_recv
     * @param timeout_ms
     * @return >0 when the socket is ready, 0 on timeout or cancellation, <0 on error
     */
    int WaitOnSocket(curl_socket_t socket, int for_recv, long timeout_ms)
    {
        // Cap timeout to max int value to avoid overflow in poll()
        long remaining = std::min(std::max(timeout_ms, 0L), static_cast<long>(std::numeric_limits<int>::max()));
        constexpr long sliceMs = 100;
        for (;;)
        {
            if (isAborted.load(std::memory_order_acquire))
            {
                return 0;
            }

            const long slice = std::min(remaining, sliceMs);
            struct pollfd pfd;
            pfd.fd = socket;
            pfd.events = for_recv ? POLLIN : POLLOUT;
            pfd.revents = 0;
            const int pollResult = poll(&pfd, 1, static_cast<int>(slice));
            if (pollResult != 0)
            {
                // Ready, or a poll() error. Both are terminal, exactly as the
                // single-shot poll() this replaced.
                return pollResult;
            }
            if (remaining <= slice)
            {
                return 0;   // timed out
            }
            remaining -= slice;
        }
    }

    /**
     * Install the libcurl progress callback used to abort a transfer.
     *
     * XFERINFO supersedes PROGRESSFUNCTION in libcurl 7.32.0; keep the old
     * option for builds pinned to an older libcurl.
     */
    bool SetAbortProgressOption()
    {
#if LIBCURL_VERSION_NUM >= 0x072000 // Version 7.32.0
        return SetOption(CURLOPT_XFERINFOFUNCTION, &XferInfoAbortCallback) &&
               SetOption(CURLOPT_XFERINFODATA, static_cast<void*>(this));
#else
        return SetOption(CURLOPT_PROGRESSFUNCTION, &ProgressAbortCallback) &&
               SetOption(CURLOPT_PROGRESSDATA, static_cast<void*>(this));
#endif
    }

#if LIBCURL_VERSION_NUM >= 0x072000 // Version 7.32.0
    static int XferInfoAbortCallback(void* clientp, curl_off_t, curl_off_t, curl_off_t, curl_off_t) noexcept
    {
        const auto* operation = static_cast<const CurlHttpOperation*>(clientp);
        // Returning non-zero makes libcurl fail the transfer with
        // CURLE_ABORTED_BY_CALLBACK, on the worker thread, with the socket and
        // the easy handle still owned by their owner.
        return (operation != nullptr && operation->isAborted.load(std::memory_order_acquire)) ? 1 : 0;
    }
#else
    static int ProgressAbortCallback(void* clientp, double, double, double, double) noexcept
    {
        const auto* operation = static_cast<const CurlHttpOperation*>(clientp);
        return (operation != nullptr && operation->isAborted.load(std::memory_order_acquire)) ? 1 : 0;
    }
#endif

    // SECURITY: upper bound on the collector response the client will buffer. The
    // OneCollector protocol responses (status, kill-switch tokens, retry-after, small
    // config) are tiny, so this generous cap never rejects a legitimate response but
    // stops a hostile or MITM'd collector from driving unbounded memory growth by
    // returning an oversized body (a memory-amplification DoS of the embedding process).
    // Exceeding it aborts the transfer, so the upload is treated as failed and retried.
    static constexpr size_t kMaxResponseBytes = 16 * 1024 * 1024; // 16 MB

    // Raw response buffer
    struct MemoryStruct {
      char *memory;
      size_t size;
    } response;

    /**
     * Old-school memory allocator
     *
     * @param contents
     * @param size
     * @param nmemb
     * @param userp
     * @return
     */
    static size_t WriteMemoryCallback(char* contents, size_t size, size_t nmemb, void* userp)
    {
        // Guard the size * nmemb product against size_t overflow before using it.
        if (nmemb != 0 && size > static_cast<size_t>(-1) / nmemb) {
            return 0;
        }
        size_t realsize = size * nmemb;
        auto* mem = static_cast<MemoryStruct*>(userp);

        // SECURITY: bound the buffered response (see kMaxResponseBytes). Compare
        // overflow-safely (mem->size is always <= kMaxResponseBytes here). Returning a
        // short count aborts the transfer with CURLE_WRITE_ERROR.
        if (realsize > kMaxResponseBytes - mem->size) {
            TRACE("Response exceeds max buffered size (%zu bytes); aborting transfer\n", kMaxResponseBytes);
            return 0;
        }

        auto* memory = static_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));
        if(memory == nullptr) {
          /* out of memory! */
          TRACE("not enough memory (realloc returned NULL)\n");
          return 0;
        }
        mem->memory = memory;
#ifdef HAVE_ONEDS_BOUNDCHECK_METHODS
        BoundCheckFunctions::oneds_memcpy_s(&(mem->memory[mem->size]), realsize, contents, realsize);
#else
        memcpy(&(mem->memory[mem->size]), contents, realsize);
#endif
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
    }

    /**
     * C++ STL std::string allocator
     *
     * @param ptr
     * @param size
     * @param nmemb
     * @param data
     * @return
     */
    static size_t WriteVectorCallback(char* ptr, size_t size, size_t nmemb, void* userp)
    {
        // Guard the size * nmemb product against size_t overflow before using it.
        if (nmemb != 0 && size > static_cast<size_t>(-1) / nmemb) {
            return 0;
        }
        size_t realsize = size * nmemb;
        auto* data = static_cast<std::vector<uint8_t>*>(userp);
        if (data != nullptr) {
            // SECURITY: bound the buffered response (see kMaxResponseBytes). Compare
            // overflow-safely (data->size() is always <= kMaxResponseBytes here).
            // Returning a short count aborts the transfer with CURLE_WRITE_ERROR.
            if (realsize > kMaxResponseBytes - data->size()) {
                TRACE("Response exceeds max buffered size (%zu bytes); aborting transfer\n", kMaxResponseBytes);
                return 0;
            }
            const auto* begin = reinterpret_cast<const uint8_t*>(ptr);
            const auto* end   = begin + realsize;
            data->insert( data->end(), begin, end);
        }
        return realsize;
    }

};

} MAT_NS_END

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

#endif // HTTPCLIENTCURL_HPP
