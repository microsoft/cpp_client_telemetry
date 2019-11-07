// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IHttpClient.hpp"
#include "IOfflineStorage.hpp"
#include "packager/BondSplicer.hpp"
#include "pal/PAL.hpp"
#include "utils/Utils.hpp"

#include <map>
#include <memory>
#include <vector>
#include <atomic>

namespace ARIASDK_NS_BEGIN {


    class IncomingEventContext {
    public:
        ::CsProtocol::Record*  source;
        StorageRecord          record;
        std::uint64_t          policyBitFlags;

    public:
        IncomingEventContext() :
            source(nullptr),
            policyBitFlags(0)
        {
        }

        IncomingEventContext(std::string const& id, std::string const& tenantToken, EventLatency latency, EventPersistence persistence, ::CsProtocol::Record* source)
            : source(source),
            record{ id, tenantToken, latency, persistence },
	    policyBitFlags(0)
        {
        }

        virtual ~IncomingEventContext()
        {
        }
    };

    typedef IncomingEventContext* IncomingEventContextPtr;

    //---

    class EventsUploadContext {

    private:

#ifdef CRT_DEBUG_LEAKS
        // Track # of outstanding EventUploadContext objects remaining
        long objCount(long delta)
        {
            static std::atomic<long> seq(0);
            seq += delta;
            return seq;
        }
#endif

    public:

        /**
        * Release unmanaged pointers associated with EventsUploadContext
        */
        void clear()
        {
            httpRequest = nullptr;
            httpResponse = nullptr;
        }

        // Retrieving
        EventLatency                         requestedMinLatency = EventLatency_Unspecified;
        unsigned                             requestedMaxCount = 0;

        // Packaging
        BondSplicer                          splicer;
        unsigned                             maxUploadSize = 0;
        EventLatency                         latency = EventLatency_Unspecified;
        std::map<std::string, size_t>        packageIds;
        std::map<std::string, std::string>   recordIdsAndTenantIds;
        std::vector<int64_t>                 recordTimestamps;
        unsigned                             maxRetryCountSeen = 0;

        // Encoding
        std::vector<uint8_t>                 body;
        bool                                 compressed = false;

        // Sending
        std::unique_ptr<IHttpRequest>        httpRequest = nullptr;
        std::string                          httpRequestId;

        // Receiving
        std::unique_ptr<IHttpResponse>       httpResponse = nullptr;

        int                                  durationMs = -1;
        bool                                 fromMemory;

        EventsUploadContext() :
            fromMemory(false)
        {
#ifdef CRT_DEBUG_LEAKS
            objCount(1);
#endif

        }

        virtual ~EventsUploadContext()
        {
#ifdef CRT_DEBUG_LEAKS
            objCount(-1);
#endif
            clear();
        }
    };

    typedef EventsUploadContext* EventsUploadContextPtr;

    //---

    struct StorageNotificationContext {
        std::string str;
        std::map<std::string, size_t> countonTenant;
    };


} ARIASDK_NS_END
