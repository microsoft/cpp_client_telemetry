//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#include "LogManager.hpp"

#include <mutex>
#include <atomic>
#include <cstdint>
#include <thread>

using namespace MAT;

static const constexpr size_t MAX_LATENCY_SAMPLES = 10;

class EventDecoderListener : public DebugEventListener {
    std::mutex dbg_callback_mtx;

public:
    EventDecoderListener() : DebugEventListener()
    {
        reset();
    }

    virtual void DecodeBuffer(void *data, size_t size);
    virtual void OnDebugEvent(DebugEvent &evt);
    virtual void reset();
};

