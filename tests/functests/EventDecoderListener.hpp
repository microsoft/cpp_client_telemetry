//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#include "LogManager.hpp"

#include <mutex>
#include <atomic>
#include <cstdint>
#include <chrono>
#include <mutex>
#include <thread>

using namespace MAT;

static const constexpr size_t MAX_LATENCY_SAMPLES = 10;

class EventDecoderListener : public DebugEventListener
{
    std::mutex dbg_callback_mtx;
    ILogManager& parent;

   public:
    EventDecoderListener(ILogManager& parent) :
        DebugEventListener(),
        parent(parent)
    {
        reset();
    }

    virtual void DecodeBuffer(void* data, size_t size);
    virtual void OnDebugEvent(DebugEvent& evt);
    virtual void reset();
};

