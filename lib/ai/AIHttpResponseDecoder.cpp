// Copyright (c) Microsoft. All rights reserved.

#include "ai/AIHttpResponseDecoder.hpp"
#include "http/HttpResponseDecoder.hpp"
#include "ILogManager.hpp"
#include <IHttpClient.hpp>
#include "utils/Utils.hpp"
#include <algorithm>
#include <cassert>

#ifdef HAVE_MAT_JSONHPP
#include "json.hpp"
#endif

namespace ARIASDK_NS_BEGIN {

    AIHttpResponseDecoder::AIHttpResponseDecoder(ITelemetrySystem& system)
        :
        m_system(system)
    {
    }

    AIHttpResponseDecoder::~AIHttpResponseDecoder()
    {
    }

    /// <summary>
    /// Dispatches the specified event via parent LogManager to a client callback.
    /// </summary>
    /// <param name="evt"></param>
    /// <returns></returns>
    bool AIHttpResponseDecoder::DispatchEvent(DebugEvent evt)
    {
        return m_system.getLogManager().DispatchEvent(std::move(evt));
    }

    void AIHttpResponseDecoder::handleDecode(EventsUploadContextPtr const& ctx)
    {
#ifndef NDEBUG
        // XXX: [MG] - debug accessing object that's been already freed
        uint64_t ptr = (uint64_t)(ctx->httpResponse);
        assert(ptr != 0x00000000dddddddd);
        assert(ptr != 0xdddddddddddddddd);
#endif

        IHttpResponse const& response = *(ctx->httpResponse);
        IHttpRequest & request = *(ctx->httpRequest);

        HttpRequestResult outcome = Abort;
        auto result = response.GetResult();
        switch (result) {
        case HttpResult_OK:
            if (response.GetStatusCode() == 200)
            {
                outcome = Accepted;
            }
            else if (response.GetStatusCode() >= 500 || response.GetStatusCode() == 408 || response.GetStatusCode() == 429)
            {
                outcome = RetryServer;
            }
            else
            {
                outcome = Rejected;
            }
            break;

        case HttpResult_Aborted:
            ctx->httpResponse = nullptr;
            outcome = Abort;
            break;

        case HttpResult_LocalFailure:
        case HttpResult_NetworkFailure:
            ctx->httpResponse = nullptr;
            outcome = RetryNetwork;
            break;
        }

        if (response.GetBody().size() > 0)
        {
            processBody(response, outcome);
        }

        switch (outcome) {
        case Accepted: {
            LOG_INFO("HTTP request %s finished after %d ms, events were successfully uploaded to the server",
                response.GetId().c_str(), ctx->durationMs);
            {
                DebugEvent evt;
                evt.type = DebugEventType::EVT_HTTP_OK;
                evt.param1 = response.GetStatusCode();
                evt.data = static_cast<void *>(request.GetBody().data());
                evt.size = request.GetBody().size();
                DispatchEvent(evt);
            }
            eventsAccepted(ctx);
            break;
        }

        case Rejected: {
            LOG_ERROR("HTTP request %s failed after %d ms, events were rejected by the server (%u) and will be all dropped",
                response.GetId().c_str(), ctx->durationMs, response.GetStatusCode());
            std::string body(reinterpret_cast<char const*>(response.GetBody().data()), std::min<size_t>(response.GetBody().size(), 100));
            LOG_TRACE("Server response: %s%s", body.c_str(), (response.GetBody().size() > body.size()) ? "..." : "");
            {
                DebugEvent evt;
                evt.type = DebugEventType::EVT_HTTP_ERROR;
                // TODO: [MG] - currently we do not have means of bubbling up
                // HTTP response text, we can only populate the status code.
                // This is to be addressed with ETW trace API that can send
                // a detailed error context to ETW provider.
                evt.param1 = response.GetStatusCode();
                evt.data = static_cast<void *>(request.GetBody().data());
                evt.size = request.GetBody().size();
                DispatchEvent(evt);
                eventsRejected(ctx);
            }
            break;
        }

        case Abort: {
            LOG_WARN("HTTP request %s failed after %d ms, upload was aborted and events will be sent at a different time",
                response.GetId().c_str(), ctx->durationMs);
            {
                DebugEvent evt;
                evt.type = DebugEventType::EVT_HTTP_FAILURE;
                evt.param1 = 0; // response.GetStatusCode();
                DispatchEvent(evt);
            }
            ctx->httpResponse = nullptr;
            // eventsRejected(ctx); // FIXME: [MG] - investigate why ctx gets corrupt after eventsRejected
            requestAborted(ctx);
            break;
        }

        case RetryServer: {
            LOG_WARN("HTTP request %s failed after %d ms, a temporary server error has occurred (%u) and events will be sent at a different time",
                response.GetId().c_str(), ctx->durationMs, response.GetStatusCode());
            std::string body(reinterpret_cast<char const*>(response.GetBody().data()), std::min<size_t>(response.GetBody().size(), 100));
            LOG_TRACE("Server response: %s%s", body.c_str(), (response.GetBody().size() > body.size()) ? "..." : "");
            {
                DebugEvent evt;
                evt.type = DebugEventType::EVT_HTTP_FAILURE;
                evt.param1 = response.GetStatusCode();
                DispatchEvent(evt);
            }
            temporaryServerFailure(ctx);
            break;
        }

        case RetryNetwork: {
            LOG_WARN("HTTP request %s failed after %d ms, a network error has occurred and events will be sent at a different time",
                response.GetId().c_str(), ctx->durationMs);
            {
                DebugEvent evt;
                evt.type = DebugEventType::EVT_HTTP_FAILURE;
                evt.param1 = response.GetStatusCode();
                DispatchEvent(evt);
            }
            temporaryNetworkFailure(ctx);
            break;
        }
        }
    }

    void AIHttpResponseDecoder::processBody(IHttpResponse const& response, HttpRequestResult & result)
    {
#ifdef HAVE_MAT_JSONHPP
        // TODO: [MG] - parse HTTP response without json.hpp library
        nlohmann::json responseBody;
        try
        {
            std::string body(response.GetBody().begin(), response.GetBody().end());
            responseBody = nlohmann::json::parse(body.c_str());
            int received = 0;
            auto rec = responseBody.find("itemsReceived");
            if (responseBody.end() != rec)
            {
                if (rec.value().is_number())
                {
                    received = rec.value().get<int>();
                }
            }

            int accepted = 0;
            auto acc = responseBody.find("itemsAccepted");
            if (responseBody.end() != acc)
            {
                if (acc.value().is_number())
                {
                    accepted = acc.value().get<int>();
                }
            }
            if (received > accepted ) {
                result = Rejected;
            }
        }
        catch (...)
        {
            LOG_ERROR("Http response JSON parsing failed");
        }
#else
        UNREFERENCED_PARAMETER(response);
        UNREFERENCED_PARAMETER(result);
#endif
    }

} ARIASDK_NS_END
