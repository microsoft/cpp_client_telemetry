// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_AI
#include "AIJsonSerializer.hpp"
#include "CorrelationVector.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace ARIASDK_NS_BEGIN {

    static const std::string& getSdkSemVer()
    {
        constexpr int semVer[] = {BUILD_VERSION};
        static std::string semVerStr =
            std::string("1DS-C++-") +
            std::to_string(semVer[0]) + "." +
            std::to_string(semVer[1]) + "." +
            std::to_string(semVer[2]) + "-build" +
            std::to_string(semVer[3]);
        return semVerStr;
    };


    static std::string formatTimestamp(time_t ticks)
    {
        time_t epochTicks = 621355968000000000;
        time_t millis = (ticks - epochTicks)/10000;
        auto text = PAL::formatUtcTimestampMsAsISO8601(millis);
        return text;
    }

    json serializeToAppInsightsFormat(IncomingEventContextPtr const& event, std::string const& sessionId, bool isNewSession)
    {
        ::CsProtocol::Record* source = event->source;

        json properties = json::object();
        std::map<std::string, CsProtocol::Value>::const_iterator it;
        for (it = source->data[0].properties.begin(); it != source->data[0].properties.end(); it++)
        {
            switch (it->second.type)
            {
                case CsProtocol::ValueString:
                    properties[it->first] = it->second.stringValue;
                    break;
                case CsProtocol::ValueInt64:
                    properties[it->first] = it->second.longValue;
                    break;
                case CsProtocol::ValueDouble:
                    properties[it->first] = it->second.doubleValue;
                    break;
                case CsProtocol::ValueDateTime:
                    properties[it->first] = it->second.longValue;
                    break;
                case CsProtocol::ValueBool:
                    properties[it->first] = it->second.longValue ? true : false;
                    break;
                default:
                    // TODO: not implemented
                    break;
            }
        }

        json ans = {
                { "ver", 1 },
                { "name", "Microsoft.ApplicationInsights.Event" },
                { "sampleRate", 100 },
                { "iKey", event->record.tenantToken },
                { "time", formatTimestamp(source->time) },
                { "tags", {
                         { "ai.application.ver", source->extApp[0].ver },
                         { "ai.device.id", source->extDevice[0].localId },
                         { "ai.device.locale", source->extApp[0].locale }, // en-US vs "en_US"
                         { "ai.device.model", source->extProtocol[0].devModel },
                         { "ai.device.oemName", source->extProtocol[0].devMake },
                         { "ai.device.os", source->extOs[0].name },
                         { "ai.device.osVersion", source->extOs[0].ver },
                         { "ai.device.type", source->extDevice[0].deviceClass },
                         { "ai.session.id", sessionId },
                         { "ai.session.isFirst", isNewSession },
                         { "ai.session.isNew", "true" },
                         { "ai.user.id", source->extUser[0].localId }
                     }},
                { "data", {
                        { "baseType", "EventData" },
                        { "baseData", {
                               { "ver", 2 },
                               { "name", source->name },
                               { "properties", properties }
                        }}
                }}
        };

        return ans;
    }

    bool AIJsonSerializer::handleSerialize(IncomingEventContextPtr const& ctx)
    {
        bool isNewSession = false;
        if (m_sessionId.empty()) {
            m_sessionId = PAL::generateUuidString();
            isNewSession = true; // TBD is not correct when first events is a stats events
        }

        json result = serializeToAppInsightsFormat(ctx, m_sessionId, isNewSession);
        auto str = result.dump();
        std::vector<uint8_t> vec(str.begin(), str.end());
        ctx->record.blob = vec;
        return true;
    }

} ARIASDK_NS_END
#endif
