// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Events  {
// *INDENT-ON*


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


}}} // namespace Microsoft::Applications::Events 
