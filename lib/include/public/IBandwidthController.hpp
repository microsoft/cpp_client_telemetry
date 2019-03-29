// Copyright (c) Microsoft. All rights reserved.
#ifndef IBANDWIDTHCONTROLLER_HPP
#define IBANDWIDTHCONTROLLER_HPP

#include "Version.hpp"

namespace ARIASDK_NS_BEGIN
{
    class IBandwidthController {
    public:
        virtual ~IBandwidthController() {}

        /// <summary>
        /// Query the current proposed bandwidth for uploading telemetry events.
        ///
        /// This method is called each time the SDK is preparing to send some
        /// events in an HTTP request.
        ///
        /// The library might not be able to actually honor the return value more
        /// than just not transmitting anything if the proposed bandwidth is 0 and
        /// resuming the upload when it isn't.
        /// </summary>
        /// <returns>Proposed bandwidth in bytes per second</returns>
        virtual unsigned GetProposedBandwidthBps() = 0;
    };
} ARIASDK_NS_END

#endif
