#pragma once

#include "LogManager.hpp"
#include "IHttpClient.hpp"

#include <mutex>
#include <atomic>
#include <cstdint>
#include <thread>

using namespace MAT;

class HttpEventListener : public DebugEventListener
{
    std::mutex dbg_callback_mtx;
    void printDebugEvent(const char *label, const DebugEvent& evt);
public:
    virtual void OnHttpStateEvent(DebugEvent& evt);
    virtual void OnDebugEvent(DebugEvent &evt);
};
