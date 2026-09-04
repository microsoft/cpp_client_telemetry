//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

// Assume that if we are compiling with MSVC, then we prefer to use Windows HTTP stack,
// e.g. WinInet.dll or Win 10 HTTP client instead
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT)

#include "ctmacros.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "utils/Utils.hpp"
#include "HttpClient_Curl.hpp"
#include "ILogConfiguration.hpp"

// The SDK must never tear down libcurl's process-wide state; see
// EnsureCurlGlobalInit() for why teardown is unknowable from inside an embedded
// library. Poisoning the identifier after the libcurl headers have been
// included turns any future call from this translation unit into a build error
// instead of a rare crash in an unrelated component of the host process.
#if defined(__GNUC__)
#pragma GCC poison curl_global_cleanup
#endif

namespace MAT_NS_BEGIN {

    static bool IsLocalRequestError(CURLcode error) noexcept
    {
        return error == CURLE_UNSUPPORTED_PROTOCOL ||
            error == CURLE_URL_MALFORMAT ||
            error == CURLE_NOT_BUILT_IN;
    }

    static std::string NextReqId() {
        static std::atomic<uint64_t> seq(0);
        return std::string("REQ-") + std::to_string(seq.fetch_add(1));
    }

    // The request carries request data and an id and nothing else. It owns no
    // transport object and holds no cancellation handle. The current Curl
    // implementation copies request data into operation-owned storage, but the
    // public IHttpClient contract still requires the caller to retain a request
    // until its terminal callback begins.
    class CurlHttpRequest : public SimpleHttpRequest
    {
    public:
        CurlHttpRequest() : SimpleHttpRequest(NextReqId()) { }
    };

    /**
     * Per-client shared state.
     *
     * Held by the facade and captured by every completion, so it outlives the
     * HttpClient_Curl object. Completions never capture the client itself.
     *
     * No user callback, libcurl call, or operation-local lock is ever taken
     * while this mutex is held.
     */
    struct CurlClientState
    {
        std::mutex mutex;
        std::condition_variable cv;

        // Owning registry. The operation outlives both the caller's IHttpRequest
        // and the client facade, so cancellation and completion never
        // dereference storage owned by somebody else.
        std::map<std::string, std::shared_ptr<CurlHttpOperation>> operations;

        bool accepting {true};
        size_t cancelAllDepth {0};
        size_t registryGeneration {0};
        size_t callbackGeneration {0};
        size_t callbacksInFlight {0};
        // Incremented before an operation is constructed and decremented by the
        // operation's shared_ptr deleter, i.e. only after ~CurlHttpOperation has
        // joined or detached its worker and run curl_easy_cleanup(). A full
        // drain that observes zero here knows no curl handle is still live.
        size_t liveOperationCount {0};
        std::map<std::thread::id, size_t> callbacksByThread;
        std::map<std::thread::id, size_t> workersByThread;

        std::atomic<bool> sslVerify {true};
        std::string sslCaInfo;      // guarded by mutex

        // Returns true when the caller should start the worker. A false return
        // means the operation must complete as Aborted without touching the
        // network: either admission has stopped, or a cancellation epoch is in
        // progress and must not be starved by late sends.
        bool registerOperation(std::string const& id, std::shared_ptr<CurlHttpOperation> operation)
        {
            bool shouldSend;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (!accepting)
                {
                    return false;
                }
                operations[id] = std::move(operation);
                ++registryGeneration;
                shouldSend = (cancelAllDepth == 0);
            }
            cv.notify_all();
            return shouldSend;
        }

        // Re-evaluated after the deferred creation event has run: the worker may
        // only start if admission is still open and no cancellation epoch is in
        // progress. Mirrors registerOperation's send decision so a creation
        // callback that stopped admission or opened an epoch cannot be raced.
        bool stillAcceptingSend()
        {
            std::lock_guard<std::mutex> lock(mutex);
            return accepting && cancelAllDepth == 0;
        }

        void eraseOperation(std::string const& id)
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                operations.erase(id);
                ++registryGeneration;
            }
            cv.notify_all();
        }

        void stopAccepting()
        {
            std::lock_guard<std::mutex> lock(mutex);
            accepting = false;
        }

        void beginCallback()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                ++callbacksInFlight;
                ++callbacksByThread[std::this_thread::get_id()];
                ++callbackGeneration;
            }
            cv.notify_all();
        }

        void endCallback()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (callbacksInFlight == 0)
                {
                    LOG_ERROR("curl callback accounting underflow");
                }
                else
                {
                    --callbacksInFlight;
                    auto it = callbacksByThread.find(std::this_thread::get_id());
                    if (it == callbacksByThread.end() || it->second == 0)
                    {
                        LOG_ERROR("curl callback thread was not registered");
                    }
                    else if (--it->second == 0)
                    {
                        callbacksByThread.erase(it);
                    }
                }
                ++callbackGeneration;
            }
            cv.notify_all();
        }

        void beginWorker()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                ++workersByThread[std::this_thread::get_id()];
            }
            cv.notify_all();
        }

        void endWorker()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                auto it = workersByThread.find(std::this_thread::get_id());
                if (it == workersByThread.end() || it->second == 0)
                {
                    LOG_ERROR("curl worker thread was not registered");
                }
                else if (--it->second == 0)
                {
                    workersByThread.erase(it);
                }
            }
            cv.notify_all();
        }

        void noteOperationCreated()
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++liveOperationCount;
        }

        void noteOperationDestroyed()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (liveOperationCount == 0)
                {
                    LOG_ERROR("curl operation accounting underflow");
                }
                else
                {
                    --liveOperationCount;
                }
            }
            cv.notify_all();
        }
    };

    // RAII accounting for a user-visible callback. A drain that starts while a
    // callback is running must see it, and must still be able to tell that
    // callback apart from a peer on another thread.
    class CurlCallbackScope
    {
    public:
        explicit CurlCallbackScope(std::shared_ptr<CurlClientState> state)
            : m_state(std::move(state))
        {
            m_state->beginCallback();
        }

        ~CurlCallbackScope()
        {
            m_state->endCallback();
        }

        CurlCallbackScope(CurlCallbackScope const&) = delete;
        CurlCallbackScope& operator=(CurlCallbackScope const&) = delete;

    private:
        std::shared_ptr<CurlClientState> m_state;
    };

    namespace
    {
        // Ties liveOperationCount to the operation's destructor mechanically: the
        // count is released by the deleter, after ~CurlHttpOperation has joined
        // or detached the worker and released the curl handle. No caller can
        // forget to decrement it, and no drain can observe zero while a curl
        // handle is still alive.
        std::shared_ptr<CurlHttpOperation> MakeTrackedOperation(
            std::shared_ptr<CurlClientState> const& state,
            std::string const& method,
            std::string const& url,
            IHttpResponseCallback* callback,
            std::map<std::string, std::string> const& requestHeaders,
            std::vector<uint8_t> const& requestBody,
            size_t httpConnTimeout,
            bool sslVerify,
            std::string const& sslCaInfo)
        {
            state->noteOperationCreated();
            CurlHttpOperation* raw = nullptr;
            try
            {
                raw = new CurlHttpOperation(
                    method, url, callback, requestHeaders, requestBody,
                    false, httpConnTimeout, sslVerify, sslCaInfo,
                    CurlHttpOperation::CallbackHooks {
                        [state]() { state->beginCallback(); },
                        [state]() { state->endCallback(); }
                    },
                    CurlHttpOperation::WorkerHooks {
                        [state]() { state->beginWorker(); },
                        [state]() { state->endWorker(); }
                    },
                    // Tracked operations defer OnCreated/OnCreateFailed until
                    // after registration so a reentrant cancel can find them.
                    true);
            }
            catch (...)
            {
                state->noteOperationDestroyed();
                throw;
            }

            try
            {
                return std::shared_ptr<CurlHttpOperation>(
                    raw, [state](CurlHttpOperation* operation) noexcept {
                        delete operation;
                        state->noteOperationDestroyed();
                    });
            }
            catch (...)
            {
                delete raw;
                state->noteOperationDestroyed();
                throw;
            }
        }
    }

    HttpClient_Curl::HttpClient_Curl() :
        m_state(std::make_shared<CurlClientState>())
    {
        TRACE("Initializing HttpClient_Curl...\n");
        EnsureCurlGlobalInit();
        TRACE("libcurl version = %s\n", curl_version_info(CURLVERSION_NOW)->version);
    }

    HttpClient_Curl::~HttpClient_Curl()
    {
        // Stop admitting work before draining, so the drain below cannot be
        // starved by a concurrent SendRequestAsync.
        m_state->stopAccepting();
        CancelAllRequests();
        // Deliberately no curl_global_cleanup(); see EnsureCurlGlobalInit().
        //
        // Reentrant destruction (a caller deleting this client from inside one
        // of its own callbacks) is safe: CancelAllRequests() recognizes that
        // caller and returns without waiting for it, and the shared state, the
        // running operation and the completion that owns them are all kept alive
        // by the callback's own captures. The client object itself must not be
        // touched after this returns.
        TRACE("Destroyed HttpClient_Curl.\n");
    };

    IHttpRequest* HttpClient_Curl::CreateRequest()
    {
        return new CurlHttpRequest();
    }

    void HttpClient_Curl::SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback)
    {
        // Keep shared state locally: the deferred OnCreated / OnCreateFailed
        // event dispatched below (or the terminal callback) may destroy this
        // facade, so nothing after construction may touch m_state. The request
        // is borrowed under the public IHttpClient contract, while this Curl
        // implementation copies its fields and never touches it after this
        // initial extraction.
        auto state = m_state;
        auto curlRequest = static_cast<CurlHttpRequest*>(request);

        const std::string requestId = curlRequest->GetId();
        const std::string method = curlRequest->m_method;
        const std::string url = curlRequest->m_url;
        const std::vector<uint8_t> body = curlRequest->m_body;
        std::map<std::string, std::string> requestHeaders;
        for (const auto& header : curlRequest->m_headers) {
            requestHeaders[header.first] = header.second;
        }

        bool sslVerify;
        std::string sslCaInfo;
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            sslVerify = state->sslVerify.load(std::memory_order_acquire);
            sslCaInfo = state->sslCaInfo;
        }

        std::shared_ptr<CurlHttpOperation> operation;
        try
        {
            operation = MakeTrackedOperation(
                state, method, url, callback, requestHeaders, body,
                HTTP_CONN_TIMEOUT, sslVerify, sslCaInfo);
        }
        catch (const std::exception&)
        {
            CurlCallbackScope callbackScope(state);
            auto response = std::unique_ptr<SimpleHttpResponse>(
                new SimpleHttpResponse(requestId));
            response->m_result = HttpResult_LocalFailure;
            callback->OnHttpResponse(response.release());
            return;
        }

        auto completion = [state, operation, callback, requestId](CurlHttpOperation& op) {
            // Account for this callback before anything else, so a drain that
            // starts now waits for it (or recognizes itself in it).
            CurlCallbackScope callbackScope(state);

            // Release the registry identity before the user callback runs: the
            // id is then free for reuse and a concurrent CancelRequestAsync()
            // can no longer pick up an operation that is already completing.
            // The 'operation' capture keeps the object alive across the response
            // build and the callback itself.
            state->eraseOperation(requestId);

            auto response = std::unique_ptr<SimpleHttpResponse>(new SimpleHttpResponse(requestId));
            response->m_result = HttpResult_OK;

            response->m_statusCode = op.GetHttpStatusCode();
            if (op.WasAborted()) {
                // Cancellation wins even when libcurl finishes the transfer
                // successfully after the caller has requested an abort.
                response->m_result = HttpResult_Aborted;
            } else if (op.GetSetupError() != CURLE_OK ||
                       IsLocalRequestError(op.GetTransportError())) {
                // There was an error configuring the CURL request.
                response->m_result = HttpResult_LocalFailure;
            } else if (op.GetTransportError() != CURLE_OK) {
                // There was an error in CURL stack while trying to connect.
                response->m_result = HttpResult_NetworkFailure;
            }

            auto responseHeaders = op.GetResponseHeaders();
            response->m_headers.insert(responseHeaders.begin(), responseHeaders.end());
            response->m_body = op.GetResponseBody();

            // 'response' is no longer owned by IHttpClient and gets deleted in EventsUploadContext.clear()
            callback->OnHttpResponse(response.release());
        };

        // Register before dispatching the creation event. A cancellation that
        // arrives from that event (or between here and the first byte on the
        // wire) must not be able to miss the operation.
        const bool shouldSend = state->registerOperation(requestId, operation);

        // Now that the operation is discoverable, replay the OnCreated /
        // OnCreateFailed state event that construction deferred. A reentrant
        // CancelRequestAsync/CancelAllRequests fired from it will find and abort
        // this operation, and it is accounted as a callback via the operation
        // hooks so a concurrent drain observes it.
        bool startWorker = false;
        try
        {
            operation->DispatchDeferredCreationEvent();

            // Re-evaluate the send decision after the creation event. A fast
            // constructor/setup failure never touches the network. Otherwise
            // the worker starts only if registration admitted it, the creation
            // callback did not cancel it, and admission is still open with no
            // cancellation epoch in progress.
            const bool creationFailed = operation->GetSetupError() != CURLE_OK;
            startWorker = shouldSend && !creationFailed &&
                !operation->WasAborted() && state->stillAcceptingSend();
            if (!startWorker && !creationFailed)
            {
                // Canceled, client destroyed, or landed in a cancellation epoch:
                // complete exactly one Aborted terminal, no worker, no socket.
                operation->Abort();
            }
        }
        catch (...)
        {
            // A state observer must not strand the operation without a terminal.
            operation->Abort();
            startWorker = false;
        }

        if (!startWorker)
        {
            // Destroy-before-terminal, no-send path. Exactly one terminal here,
            // on this thread: OnCreateFailed/OnCreated already fired, OnDestroy
            // and the response callback follow in order.
            operation->CompleteWithoutSend(completion);
            return;
        }

        operation->SendAsync(completion);
    }

    void HttpClient_Curl::CancelRequestAsync(std::string const& id)
    {
        // Snapshot the shared operation under the lock, then abort outside it.
        // The entry is never erased here: only the operation's own completion
        // retires its identity, so cancellation can never race a caller into
        // dropping the last owner of a running transfer.
        std::shared_ptr<CurlHttpOperation> operation;
        {
            std::lock_guard<std::mutex> lock(m_state->mutex);
            auto it = m_state->operations.find(id);
            if (it != m_state->operations.end()) {
                LOG_TRACE("HTTP request id=%s being aborted...", id.c_str());
                operation = it->second;
            }
        }

        if (operation != nullptr) {
            operation->Abort();
        }
    }

    void HttpClient_Curl::CancelAllRequests()
    {
        CancelAllRequests(std::chrono::milliseconds::zero());
    }

    void HttpClient_Curl::CancelAllRequests(std::chrono::milliseconds bestEffortTimeout)
    {
        auto state = m_state;

        // The epoch is open for as long as this call runs. Sends that register
        // inside it complete as Aborted without starting work, which is what
        // stops late arrivals from starving the drain; conversely the epoch
        // never rejects them silently, so every send still gets exactly one
        // terminal callback.
        class CancelAllScope
        {
        public:
            explicit CancelAllScope(std::shared_ptr<CurlClientState> state)
                : m_state(std::move(state))
            {
                std::lock_guard<std::mutex> lock(m_state->mutex);
                ++m_state->cancelAllDepth;
            }

            ~CancelAllScope()
            {
                if (m_active)
                {
                    std::lock_guard<std::mutex> lock(m_state->mutex);
                    if (m_state->cancelAllDepth == 0)
                    {
                        LOG_ERROR("curl cancel epoch accounting underflow");
                    }
                    else
                    {
                        --m_state->cancelAllDepth;
                    }
                    m_state->cv.notify_all();
                }
            }

            void finishLocked()
            {
                if (m_state->cancelAllDepth == 0)
                {
                    LOG_ERROR("curl cancel epoch accounting underflow");
                }
                else
                {
                    --m_state->cancelAllDepth;
                }
                m_active = false;
                m_state->cv.notify_all();
            }

        private:
            std::shared_ptr<CurlClientState> m_state;
            bool m_active {true};
        } cancelAllScope(state);

        const bool hasTimeout = bestEffortTimeout > std::chrono::milliseconds::zero();
        const auto deadline = std::chrono::steady_clock::now() + bestEffortTimeout;
        const std::thread::id callerThread = std::this_thread::get_id();

        std::vector<std::shared_ptr<CurlHttpOperation>> initialOperations;
        bool callerIsInsideTrackedCallbackOrWorker = false;
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            for (auto const& item : state->operations)
            {
                initialOperations.push_back(item.second);
            }
            callerIsInsideTrackedCallbackOrWorker =
                state->callbacksByThread.find(callerThread) != state->callbacksByThread.end() ||
                state->workersByThread.find(callerThread) != state->workersByThread.end();
        }

        // A reentrant cancellation must still abort all peers observed at entry.
        // It then ends its epoch and returns rather than waiting for its own
        // callback or worker (or another simultaneously cancelling callback).
        for (auto const& operation : initialOperations)
        {
            operation->Abort();
        }
        initialOperations.clear();

        if (callerIsInsideTrackedCallbackOrWorker)
        {
            std::lock_guard<std::mutex> lock(state->mutex);
            cancelAllScope.finishLocked();
            return;
        }

        auto drained = [&state]() {
            return state->operations.empty() &&
                state->callbacksInFlight == 0 &&
                state->liveOperationCount == 0;
        };

        for (;;)
        {
            size_t registryGeneration = 0;
            size_t callbackGeneration = 0;
            {
                // Scoped so the snapshot's shared_ptr references are gone before
                // the wait below: otherwise this call would hold operations
                // alive and liveOperationCount could never reach zero.
                std::vector<std::shared_ptr<CurlHttpOperation>> operations;
                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    if (drained())
                    {
                        // Completing the epoch under the registry lock makes
                        // this the linearization point: anything registered
                        // later is new work, not work this drain missed.
                        cancelAllScope.finishLocked();
                        return;
                    }

                    registryGeneration = state->registryGeneration;
                    callbackGeneration = state->callbackGeneration;
                    for (auto const& item : state->operations)
                    {
                        operations.push_back(item.second);
                    }
                }

                for (auto const& operation : operations)
                {
                    operation->Abort();
                }
            }

            std::unique_lock<std::mutex> lock(state->mutex);
            if (drained())
            {
                cancelAllScope.finishLocked();
                return;
            }
            if (hasTimeout && std::chrono::steady_clock::now() >= deadline)
            {
                cancelAllScope.finishLocked();
                return;
            }
            auto stateChangedOrDrained = [&]() {
                return state->registryGeneration != registryGeneration ||
                    state->callbackGeneration != callbackGeneration ||
                    drained();
            };
            if (hasTimeout)
            {
                // Soft cap. Returning here may leave the shared state and one
                // operation alive; both are owned by the completion that is
                // still running, and the manager drains its own HttpCallbacks
                // separately.
                if (!state->cv.wait_until(lock, deadline, stateChangedOrDrained))
                {
                    cancelAllScope.finishLocked();
                    return;
                }
            }
            else
            {
                state->cv.wait(lock, stateChangedOrDrained);
            }
        }
    }

    void HttpClient_Curl::ApplySettings(ILogConfiguration& config)
    {
        SetSslVerification(
            config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY],
            (const char *)config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO]);
    }

    void HttpClient_Curl::SetSslVerification(bool sslVerify, const std::string& caInfo)
    {
        std::lock_guard<std::mutex> lock(m_state->mutex);
        if (!sslVerify)
        {
            LOG_WARN("Ignoring sslVerify=false: curl TLS certificate and hostname verification cannot be disabled");
        }
        m_state->sslVerify.store(true, std::memory_order_release);
        m_state->sslCaInfo = caInfo;
    }

} MAT_NS_END

#endif
