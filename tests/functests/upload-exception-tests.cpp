//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "LogManagerProvider.hpp"
#include "common/HttpServer.hpp"
#include "http/HttpClientFactory.hpp"
#include "pal/TaskDispatcher.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

using namespace MAT;

namespace
{
    void Check(bool condition, const char* message)
    {
        if (!condition)
        {
            std::fprintf(stderr, "FAIL: %s\n", message);
            std::_Exit(1);
        }
    }

    class Collector : public testing::HttpServer::Callback
    {
       public:
        Collector()
        {
            const int port = server.addListeningPort(0);
            Check(port > 0, "could not bind collector");
            url = "http://127.0.0.1:" + std::to_string(port) + "/collect";
            server.setKeepalive(false);
            server.addHandler("/collect", *this);
            server.start();
        }

        ~Collector()
        {
            server.stop();
        }

        int onHttpRequest(testing::HttpServer::Request const& request,
                          testing::HttpServer::Response& response) override
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                bodies.push_back(request.content);
            }
            changed.notify_all();
            response.content = "{ \"status\": \"0\" }";
            return 200;
        }

        bool WaitFor(const std::string& event)
        {
            std::unique_lock<std::mutex> lock(mutex);
            return changed.wait_for(lock, std::chrono::seconds(10), [&]()
                                    { return std::any_of(bodies.begin(), bodies.end(), [&](const std::string& body)
                                                         { return body.find(event) != std::string::npos; }); });
        }

        std::string url;

       private:
        testing::HttpServer server;
        std::mutex mutex;
        std::condition_variable changed;
        std::vector<std::string> bodies;
    };

    class ThrowingListener : public DebugEventListener
    {
       public:
        explicit ThrowingListener(bool nonstandard) :
            nonstandard(nonstandard)
        {
        }

        void OnDebugEvent(DebugEvent&) override
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (injected)
                {
                    return;
                }
                injected = true;
            }
            changed.notify_all();
            if (nonstandard)
            {
                throw 7;
            }
            throw std::runtime_error("injected upload preparation failure");
        }

        void WaitForFailure()
        {
            std::unique_lock<std::mutex> lock(mutex);
            Check(changed.wait_for(lock, std::chrono::seconds(10), [&]()
                                   { return injected; }),
                  "SDK did not reach the throwing upload-start listener");
        }

       private:
        const bool nonstandard;
        std::mutex mutex;
        std::condition_variable changed;
        bool injected = false;
    };

    class FaultingHttpClient : public IHttpClient
    {
       public:
        FaultingHttpClient(const std::string& mode, const std::string& url) :
            mode(mode), url(url), client(HttpClientFactory::Create())
        {
            Check(client != nullptr, "native HTTP transport unavailable");
        }

        IHttpRequest* CreateRequest() override
        {
            return client->CreateRequest();
        }

        void SendRequestAsync(IHttpRequest* request, IHttpResponseCallback* callback) override
        {
            if ((mode == "send-exception" || mode == "complete-then-throw") && !injected.exchange(true))
            {
                if (mode == "complete-then-throw")
                {
                    callback->OnHttpResponse(new SimpleHttpResponse(request->GetId()));
                }
                throw std::runtime_error("injected HTTP handoff failure");
            }
            request->SetUrl(url);
            ++forwarded;
            client->SendRequestAsync(request, callback);
        }

        void CancelRequestAsync(const std::string& id) override
        {
            client->CancelRequestAsync(id);
        }

        void CancelAllRequests() override
        {
            client->CancelAllRequests();
        }

        void ApplySettings(ILogConfiguration& config) override
        {
            client->ApplySettings(config);
        }

        std::atomic<bool> injected{false};
        std::atomic<size_t> forwarded{0};

       private:
        const std::string mode;
        const std::string url;
        std::shared_ptr<IHttpClient> client;
    };
}

int main(int argc, char** argv)
{
    Check(argc == 2, "expected fault mode");
    const std::string mode(argv[1]);
    const bool preparation = mode.compare(0, 9, "exception") == 0;
    const bool exitWithoutRetry = mode == "exception-exit" || mode == "exception-exit-nonstandard";
    Check(preparation || mode == "send-exception" || mode == "complete-then-throw", "unknown fault mode");

    Collector collector;
    ThrowingListener listener(mode == "exception-nonstandard" || mode == "exception-exit-nonstandard");
    ILogConfiguration config;
    config[CFG_STR_COLLECTOR_URL] = collector.url;
    config[CFG_STR_CACHE_FILE_PATH] = ":memory:";
    config[CFG_BOOL_ENABLE_NET_DETECT] = false;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 0;
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = false;
    config[CFG_MAP_METASTATS_CONFIG]["enabled"] = false;
    config[CFG_MAP_TPM][CFG_STR_TPM_BACKOFF] = "E,10,100,2,0";
    auto client = std::make_shared<FaultingHttpClient>(mode, collector.url);
    client->ApplySettings(config);
    config.AddModule(CFG_MODULE_HTTP_CLIENT, client);

    status_t status;
    ILogManager* manager = LogManagerProvider::CreateLogManager("UploadExceptionTests", true, config, status);
    Check(manager != nullptr && status == STATUS_SUCCESS, "could not initialize SDK");
    manager->SetTransmitProfile(TransmitProfile_RealTime);
    if (preparation)
    {
        manager->AddEventListener(EVT_SENDING, listener);
    }
    ILogger* logger = manager->GetLogger("00000000000000000000000000000000-00000000-0000-0000-0000-000000000000-0000");
    Check(logger != nullptr, "could not create logger");
    logger->LogEvent(EventProperties("originalReservedEvent"));
    manager->UploadNow();

    if (preparation)
    {
        listener.WaitForFailure();
        // Run behind the failed upload so record cleanup has finished before retry or teardown.
        auto drained = std::make_shared<std::promise<void>>();
        auto done = drained->get_future();
        auto dispatcher = PAL::getDefaultTaskDispatcher();
        PAL::scheduleTask(dispatcher.get(), 0, [drained]()
                          { drained->set_value(); });
        Check(done.wait_for(std::chrono::seconds(10)) == std::future_status::ready, "failed upload task did not finish");
        Check(client->forwarded == 0, "preparation failure reached HTTP handoff");
        if (!exitWithoutRetry)
        {
            manager->UploadNow();
        }
    }
    if (!exitWithoutRetry)
    {
        Check(collector.WaitFor("originalReservedEvent"), "original reserved event was not retried through native transport");
        logger->LogEvent(EventProperties("subsequentEvent"));
        manager->UploadNow();
        Check(collector.WaitFor("subsequentEvent"), "subsequent event did not upload");
        if (!preparation)
        {
            Check(client->injected, "HTTP handoff failure was not exercised");
        }
    }
    manager->FlushAndTeardown();
    Check(LogManagerProvider::Release(config) == STATUS_SUCCESS, "could not release SDK");
    if (exitWithoutRetry)
    {
        Check(client->forwarded == 0, "teardown retried the failed upload");
    }
    std::puts("PASS: native upload exception recovery and teardown");
}
