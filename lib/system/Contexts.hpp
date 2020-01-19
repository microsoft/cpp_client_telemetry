// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "IHttpClient.hpp"
#include "IOfflineStorage.hpp"

#include "pal/PAL.hpp"
#include "utils/Utils.hpp"

#include <atomic>
#include <map>
#include <memory>
#include <vector>

namespace ARIASDK_NS_BEGIN
{
    struct IncomingEventContext : ObjCounter
    {
        ::CsProtocol::Record* source;
        StorageRecord record;
        std::uint64_t policyBitFlags;

        IncomingEventContext() :
            source(nullptr),
            policyBitFlags(0)
        {
            objCount(+1);
        }

        IncomingEventContext(
            std::string const& id,
            std::string const& tenantToken,
            EventLatency latency,
            EventPersistence persistence,
            ::CsProtocol::Record* source) :
            source(source),
            record{id, tenantToken, latency, persistence},
            policyBitFlags(0)
        {
            objCount(+1);
        }

        virtual ~IncomingEventContext()
        {
            objCount(-1);
        }
    };

    typedef IncomingEventContext* IncomingEventContextPtr;

    struct EventsUploadContext : ObjCounter
    {
        /**
         * Release unmanaged pointers associated with EventsUploadContext
         */
        void clear()
        {
#ifndef _WIN32  // FIXME: [MG] - confirm that this behaviour is correct
            if (httpRequest != nullptr)
            {
                delete httpRequest;
                httpRequest = nullptr;
            }
            /* Note that httpResponse is released by httpRequest destructor */
#else
            // httpRequest gets deleted in the SendRequestAsync of WinInt and WinRt
            if (httpResponse != nullptr)
            {
                delete httpResponse;
                httpResponse = nullptr;
            }

#endif
        }

        // Retrieving
        EventLatency requestedMinLatency = EventLatency_Unspecified;
        unsigned requestedMaxCount = 0;

        unsigned maxUploadSize = 0;
        EventLatency latency = EventLatency_Unspecified;
        std::map<std::string, size_t> packageIds;
        std::map<std::string, std::string> recordIdsAndTenantIds;
        std::vector<int64_t> recordTimestamps;
        unsigned maxRetryCountSeen = 0;

        // Encoding
        std::vector<uint8_t> body;
        bool compressed = false;

        // Sending
        IHttpRequest* httpRequest;
        std::string httpRequestId;

        // Receiving
        IHttpResponse* httpResponse;

        int durationMs = -1;
        bool fromMemory;

        // clang-format off
        EventsUploadContext() :
            httpRequest(nullptr),
            httpResponse(nullptr),
            fromMemory(false)
        {
            objCount(+1);
        }
        // clang-format on

        virtual ~EventsUploadContext()
        {
            objCount(-1);
            clear();
        }
    };

    typedef EventsUploadContext* EventsUploadContextPtr;

    struct StorageNotificationContext
    {
        std::string str;
        std::map<std::string, size_t> countonTenant;
    };

}
ARIASDK_NS_END
