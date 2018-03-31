#ifndef IBANDWIDTHCONTROLLER_HPP
#define IBANDWIDTHCONTROLLER_HPP
// Copyright (c) Microsoft. All rights reserved.

#include "Version.hpp"

namespace ARIASDK_NS_BEGIN
{

    ///@cond INTERNAL_DOCS

    class IBandwidthController {
    public:
        virtual ~IBandwidthController() {}

        /// <summary>
        /// Queries the currently proposed bandwidth for uploading telemetry events.
        /// This method is called each time the SDK prepares to send 
        /// events in an HTTP request.
        /// <br><b>Note:</b> This method might not honor the return value. 
        /// It simply stops transmitting when the proposed bandwidth becomes zero, 
        /// and resumes again when the bandwidth becomes greater than zero.
        /// </summary>
        /// <returns>An unsigned integer that contains the proposed bandwidth in bytes per second.</returns>
        virtual unsigned GetProposedBandwidthBps() = 0;
    };

    /// @endcond

} ARIASDK_NS_END
#endif
