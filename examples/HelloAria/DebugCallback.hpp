#pragma once

#include "LogManager.hpp"

#include <mutex>
#include <atomic>
#include <cstdint>
#include <thread>

using namespace MAT;

static const constexpr size_t MAX_LATENCY_SAMPLES = 10;

class MyDebugEventListener : public DebugEventListener {
    std::mutex dbg_callback_mtx;

public:
    MyDebugEventListener() : DebugEventListener()
    {
        reset();
    }
    virtual void OnDebugEvent(DebugEvent &evt);
    virtual void reset();
};
