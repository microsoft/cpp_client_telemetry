// Copyright (c) Microsoft. All rights reserved.

#include "HttpResponseDecoder.hpp"
#include "api/CommonLogManagerInternal.hpp"
#include <IHttpClient.hpp>
#include "utils/Utils.hpp"
#include "json.hpp"
#include <algorithm>


namespace ARIASDK_NS_BEGIN {


HttpResponseDecoder::HttpResponseDecoder()
{
}

HttpResponseDecoder::~HttpResponseDecoder()
{
}

void HttpResponseDecoder::handleDecode(EventsUploadContextPtr const& ctx)
{
    IHttpResponse const& response = *ctx->httpResponse;

    enum { Accepted, Rejected, RetryServer, RetryNetwork, Abort } outcome = Abort;
    switch (response.GetResult()) {
        case HttpResult_OK:
            if (response.GetStatusCode() == 200) {
                outcome = Accepted;
            } else if (response.GetStatusCode() >= 500 || response.GetStatusCode() == 408 || response.GetStatusCode() == 429) {
                outcome = RetryServer;
            } else {
                outcome = Rejected;
            }
            break;

        case HttpResult_Aborted:
            outcome = Abort;
            break;

        case HttpResult_LocalFailure:
        case HttpResult_NetworkFailure:
            outcome = RetryNetwork;
            break;
    }

    if (response.GetBody().size() > 0)
    { // parse the response 
        nlohmann::json responseBody;       
        try
        {
            std::string body(response.GetBody().begin(), response.GetBody().end());
            responseBody = nlohmann::json::parse(body.c_str());
            int accepted = 0;
            auto acc = responseBody.find("acc");
            if (responseBody.end() != acc)
            {
                if (acc.value().is_number())
                {
                    accepted = acc.value().get<int>();
                }
            }

            int rejected = 0;
            auto rej = responseBody.find("rej");
            if (responseBody.end() != rej)
            {
                if (rej.value().is_number())
                {
                    rejected = rej.value().get<int>();
                }
            }

            auto efi = responseBody.find("efi");
            if (responseBody.end() != efi)
            {
                for (auto it = responseBody["efi"].begin(); it != responseBody["efi"].end(); ++it)
                {
                    std::string efiKey(it.key());
                    nlohmann::json val = it.value();
                    if (val.is_array())
                    {
                        //std::vector<int> failureVector = val.get<std::vector<int>>();
                        // eventsRejected(ctx);     with only the ids in the vector above
                    }
                    if (val.is_string())
                    {
                        if ("all" == val.get<std::string>())
                        {
                            outcome = Rejected;
                        }
                    }
                }
            }
        }
        catch (...)
        {
            ARIASDK_LOG_ERROR("Http response jason parsing failed");
        }
    }

    switch (outcome) {
        case Accepted: {
            ARIASDK_LOG_INFO("HTTP request %s finished after %d ms, events were successfully uploaded to the server",
                response.GetId().c_str(), ctx->durationMs);
            eventsAccepted(ctx);
            CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_HTTP_OK);
            break;
        }

        case Rejected: {
            ARIASDK_LOG_ERROR("HTTP request %s failed after %d ms, events were rejected by the server (%u) and will be all dropped",
                response.GetId().c_str(), ctx->durationMs, response.GetStatusCode());
            std::string body(reinterpret_cast<char const*>(response.GetBody().data()), std::min<size_t>(response.GetBody().size(), 100));
            ARIASDK_LOG_DETAIL("Server response: %s%s", body.c_str(), (response.GetBody().size() > body.size()) ? "..." : "");
            eventsRejected(ctx);      
			DebugEvent evt;
			evt.type = DebugEventType::EVT_HTTP_ERROR;
			evt.param1 = response.GetStatusCode();
            CommonLogManagerInternal::DispatchEvent(evt);
            break;
        }

        case Abort: {
            ARIASDK_LOG_WARNING("HTTP request %s failed after %d ms, upload was aborted and events will be sent at a different time",
                response.GetId().c_str(), ctx->durationMs);
            requestAborted(ctx);
            CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_HTTP_FAILURE);
            break;
        }

        case RetryServer: {
            ARIASDK_LOG_WARNING("HTTP request %s failed after %d ms, a temporary server error has occurred (%u) and events will be sent at a different time",
                response.GetId().c_str(), ctx->durationMs, response.GetStatusCode());
            std::string body(reinterpret_cast<char const*>(response.GetBody().data()), std::min<size_t>(response.GetBody().size(), 100));
            ARIASDK_LOG_DETAIL("Server response: %s%s", body.c_str(), (response.GetBody().size() > body.size()) ? "..." : "");
            temporaryServerFailure(ctx);
            CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_HTTP_FAILURE);
            break;
        }

        case RetryNetwork: {
            ARIASDK_LOG_WARNING("HTTP request %s failed after %d ms, a network error has occurred and events will be sent at a different time",
                response.GetId().c_str(), ctx->durationMs);
            temporaryNetworkFailure(ctx);
            CommonLogManagerInternal::DispatchEvent(DebugEventType::EVT_HTTP_FAILURE);
            break;
        }
    }
}


} ARIASDK_NS_END
