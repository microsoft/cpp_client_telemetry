//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "ctmacros.hpp"

#include <chrono>

namespace MAT_NS_BEGIN {

class IBoundedHttpClientCancel
{
public:
    virtual ~IBoundedHttpClientCancel() noexcept = default;

    // Positive timeout is a soft, best-effort cap. Implementations stop
    // initiating additional cancellations at the deadline, but one synchronous
    // native handle close already in progress may finish after it. Zero means
    // the caller requires a full drain, matching IHttpClient::CancelAllRequests().
    virtual void CancelAllRequests(std::chrono::milliseconds bestEffortTimeout) = 0;
};

} MAT_NS_END
