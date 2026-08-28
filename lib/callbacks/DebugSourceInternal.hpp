//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#include "DebugEvents.hpp"

namespace MAT_NS_BEGIN
{
    using DebugEventListenerPendingReleaseCallback =
        void (*)(DebugEventListener*);

    bool IsDebugEventListenerPending(const DebugEventListener* listener) noexcept;
    void SetDebugEventListenerPendingReleaseCallback(
        DebugEventListenerPendingReleaseCallback callback) noexcept;
}
MAT_NS_END
