// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_AI
#include "AIJsonSerializer.hpp"
#include "CorrelationVector.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace ARIASDK_NS_BEGIN {

    static std::string formatTimestamp(time_t ticks)
    {
        time_t epochTicks = 621355968000000000;
        time_t millis = (ticks - epochTicks)/10000;
        auto text = PAL::formatUtcTimestampMsAsISO8601(millis);
        return text;
    }

    json serializeToAppInsightsFormat(IncomingEventContextPtr const& event)
    {
        ::CsProtocol::Record* source = event->source;

        json properties = json::object();
        std::map<std::string, CsProtocol::Value>::const_iterator it;
        for (it = source->data[0].properties.begin(); it != source->data[0].properties.end(); it++)
        {
            properties[it->first] = it->second.stringValue;
        }

        json ans = {
                { "ver", 1 },
                { "name", "Microsoft.ApplicationInsights.Event" },
                { "sampleRate", 100 },
                { "iKey", event->record.tenantToken },
                { "time", formatTimestamp(source->time) },
                { "tags", {
                         { "ai.application.ver", source->extApp[0].ver },
                         { "ai.device.id", "5c3e9dff-4372-4dd0-923b-ef7d1bbd70e9" },
                         { "ai.device.language", "en" },
                         { "ai.device.locale", source->extApp[0].locale }, // en-US vs "en_US"
                         { "ai.device.model", source->extProtocol[0].devModel },
                         { "ai.device.oemName", source->extProtocol[0].devMake },
                         { "ai.device.os", source->extOs[0].name },
                         { "ai.device.osVersion", source->extOs[0].ver },
                         { "ai.device.screenResolution", "2340x1080" },
                         { "ai.device.type", source->extDevice[0].deviceClass },
                         { "ai.session.id", "1741618e-da97-484f-9c9e-95f3cb4f16a3" },
                         { "ai.session.isFirst", "false" },
                         { "ai.session.isNew", "true" },
                         { "ai.user.id", "5c3e9dff-4372-4dd0-923b-ef7d1bbd70e9" },
                         { "ai.internal.sdkVersion", "android:5.1.1" }
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
        json result = serializeToAppInsightsFormat(ctx);
        auto str = result.dump();
        std::vector<uint8_t> vec(str.begin(), str.end());
        ctx->record.blob = vec;
        return true;
    }

} ARIASDK_NS_END
#endif
