//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "IHttpClient.hpp"
#include "IOfflineStorage.hpp"
#include "packager/ISplicer.hpp"
#include "packager/BondSplicer.hpp"
#include "pal/PAL.hpp"
#include "utils/Utils.hpp"

#include <map>
#include <memory>
#include <vector>
#include <atomic>

namespace MAT_NS_BEGIN {


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

#ifdef HAVE_MAT_EVT_TRACEID   
        IncomingEventContext(std::string const& id, std::string const& tenantToken, EventLatency latency, EventPersistence persistence, ::CsProtocol::Record* source)
            : source(source),
            record{ id, tenantToken, latency, persistence, (source != nullptr) ? source->cV : "" },
	    policyBitFlags(0)
        {
        }
#else
        IncomingEventContext(std::string const& id, std::string const& tenantToken, EventLatency latency, EventPersistence persistence, ::CsProtocol::Record* source)
            : source(source),
            record{ id, tenantToken, latency, persistence },
	    policyBitFlags(0)
        {
        }
#endif

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
        void clear() noexcept
        {
            if (httpRequest != nullptr) {
                delete httpRequest;
                httpRequest = nullptr;
            }
            if (httpResponse != nullptr) {
                delete httpResponse;
                httpResponse = nullptr;
            }
        }

        // Retrieving
        EventLatency                         requestedMinLatency = EventLatency_Unspecified;
        unsigned                             requestedMaxCount = 0;

        // Packaging
        std::unique_ptr<ISplicer>            splicer;
        unsigned                             maxUploadSize = 0;
        EventLatency                         latency = EventLatency_Unspecified;
        std::map<std::string, size_t>        packageIds;
#ifdef HAVE_MAT_EVT_TRACEID  
        std::string                          traceId;
#endif
        std::map<std::string, std::string>   recordIdsAndTenantIds;
        std::vector<int64_t>                 recordTimestamps;
        unsigned                             maxRetryCountSeen = 0;

        // Encoding
        std::vector<uint8_t>                 body;
        bool                                 compressed = false;

        // Sending
        IHttpRequest*                        httpRequest = nullptr;
        std::string                          httpRequestId;

        // Receiving
        IHttpResponse*                       httpResponse = nullptr;

        int                                  durationMs = -1;
        bool                                 fromMemory = false;

        EventsUploadContext() noexcept : 
            EventsUploadContext(std::unique_ptr<ISplicer>(new BondSplicer()))
        {
        }

        EventsUploadContext(std::unique_ptr<ISplicer> &&splicer) : 
            splicer(std::move(splicer))
        {
#ifdef CRT_DEBUG_LEAKS
            objCount(1);
#endif
        }

        virtual ~EventsUploadContext() noexcept
        {
#ifdef CRT_DEBUG_LEAKS
            objCount(-1);
#endif
            clear();
        }
    };

    using EventsUploadContextPtr = std::shared_ptr<EventsUploadContext>;

    //---

    struct StorageNotificationContext {
        std::string str;
        std::map<std::string, size_t> countonTenant;
    };


} MAT_NS_END

