// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include <IHttpClient.hpp>
#include <IOfflineStorage.hpp>
#include "packager/BondSplicer.hpp"
#include "pal/PAL.hpp"
#include <map>
#include <memory>
#include <vector>

namespace ARIASDK_NS_BEGIN {


class IncomingEventContext : public PAL::RefCountedImpl<IncomingEventContext> {
  public:
    ::AriaProtocol::Record * source;
    StorageRecord            record;
    std::uint64_t            policyBitFlags;

  public:
    IncomingEventContext()
    {
    }

    IncomingEventContext(std::string const& id, std::string const& tenantToken, EventPriority priority, ::AriaProtocol::Record* source)
      : source(source),
        record{id, tenantToken, priority}
    {
    }
};

using IncomingEventContextPtr = PAL::RefCountedPtr<IncomingEventContext>;

//---

class EventsUploadContext : public PAL::RefCountedImpl<EventsUploadContext> {
  public:
    // Retrieving
    EventPriority                        requestedMinPriority = EventPriority_Unspecified;
    unsigned                             requestedMaxCount = 0;

    // Packaging
    BondSplicer                          splicer;
    unsigned                             maxUploadSize = 0;
    EventPriority                        priority = EventPriority_Unspecified;
    std::map<std::string, size_t>        packageIds;
    std::vector<std::string>             recordIds;
    std::vector<int64_t>                 recordTimestamps;
    unsigned                             maxRetryCountSeen = 0;

    // Encoding
    std::vector<uint8_t>                 body;
    bool                                 compressed = false;

    // Sending
    std::unique_ptr<IHttpRequest>        httpRequest;
    std::string                          httpRequestId;

    // Receiving
    std::unique_ptr<IHttpResponse const> httpResponse;
    int                                  durationMs = -1;
};

using EventsUploadContextPtr = PAL::RefCountedPtr<EventsUploadContext>;

//---

struct StorageNotificationContext {
    std::string str;
    unsigned    count;
};


} ARIASDK_NS_END
