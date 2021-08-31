//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_LOGCONFIGURATION_HPP
#define MAT_LOGCONFIGURATION_HPP

#include "ILogConfiguration.hpp"

#include "Enums.hpp"
#include "ctmacros.hpp"

#include <string>

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4251 ) // std::string container is safe to expose without DLL export attribute on Windows
#endif
            struct MATSDK_LIBABI LogConfiguration
            {
                /// <summary>[optional] Enable lifecycle session.
                /// default will be false
                /// </summary>
                bool enableLifecycleSession;

                /// <summary>[optional] Enable multiTenant
                /// default will be true
                /// </summary>
                bool multiTenantEnabled;

                /// <summary>[optional] Url of the collector for sending events
                /// default will be used if not specified
                /// </summary>
                std::string eventCollectorUri;

                /// <summary>[not implemented] Url of the collector for sending BLOB data
                /// default will be used if not specified
                /// </summary>
                std::string blobCollectorUri;

                /// <summary>[required] Full path and name of the disk file used to cache events on client side
                /// Specify one to avoid/reduce potential data loss by persisting event to file storage 
                /// for them to be sent in next telemetry session.
                /// </summary>
                std::string cacheFilePath;

                /// <summary>[optional] Size limit of the disk file used to cache events on client side.
                /// Additional events might cause older events in the file cache to be dropped.
                /// This size limit should be larger than the cacheMemorySizeLimitInBytes below
                /// </summary>
                unsigned int cacheFileSizeLimitInBytes;

                /// <summary>[optional] Memory size limit that allows events to be cached in memory.
                /// Additional events will cause older events to be flushed to disk file.
                /// Maximum supported in-ram queue size is 2MB.
                /// </summary>
                unsigned int cacheMemorySizeLimitInBytes;

                /// <summary>[optional] Debug trace module mask controls what modules may emit debug output.<br>
                /// default is 0 - monitor no modules</summary>
                unsigned int  traceLevelMask;

                /// <summary>[optional] Debug trace level mask controls global verbosity level.<br>
                /// default is ACTTraceLevel_Error</summary>
                unsigned int minimumTraceLevel;

                /// <summary>Api to set SDK mode with Non UTC, UTC with common Schema or UTC with legacy Schema.<br>
                /// default is Non UTC</summary>
                unsigned int sdkmode;

                /// <summary>[optional] Maximum amount of time (in seconds) allotted to upload in-ram and offline records on teardown.<br>
                /// If device is in a state where events are not allowed to be transmitted (offline, roaming, etc.), then the value is ignored.<br>
                /// default duration is 0</summary>
                unsigned int maxTeardownUploadTimeInSec;

                /// <summary>[optional] Maximum number of HTTP requests posted to HTTP stack.<br>
                /// This value controls how much RAM is allocated for pending HTTP requests.<br>
                /// Each request may consume up to ~1MB of RAM. On slow network connections<br>
                /// it may happen that there is a large number of requests pending.<br>
                /// default value is 4</summary>
                unsigned int maxPendingHTTPRequests;

                /// <summary>[optional] Maximum number of DB flush back buffers / queues.<br>
                /// Each Flush() operation or overflow of in-ram to offline storage is handled<br>
                /// asynchronously by swapping in-ram queue to a back buffer and scheduling<br>
                /// asynchronous task that saves the buffer to offline storage.<br>
                /// Each queue consumes up to <i>cacheMemorySizeLimitInBytes</i> bytes.<br>
                /// Default value is 3 queues (back buffers)</summary>
                unsigned int maxDBFlushQueues;
            };
#ifdef _MSC_VER
#pragma warning( pop )
#endif
        }
    }
}

namespace MAT_NS_BEGIN {

    MATSDK_LIBABI const ILogConfiguration& GetDefaultConfiguration();

    MATSDK_LIBABI ILogConfiguration FromLogConfiguration(MAT_v1::LogConfiguration &src);

    MATSDK_LIBABI ILogConfiguration FromJSON(const char* json);

} MAT_NS_END

#endif

