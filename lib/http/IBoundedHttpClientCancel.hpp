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

    // A positive timeout is best-effort; zero requires a full drain.
    virtual void CancelAllRequests(std::chrono::milliseconds bestEffortTimeout) = 0;
};

} MAT_NS_END
