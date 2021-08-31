//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IBANDWIDTHCONTROLLER_HPP
#define IBANDWIDTHCONTROLLER_HPP

#include "ctmacros.hpp"

namespace MAT_NS_BEGIN
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

} MAT_NS_END
#endif

