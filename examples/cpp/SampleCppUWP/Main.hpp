#pragma once

#include <algorithm>
#include <functional>
#include <varargs.h>

// Implementation of Debug Output window
extern std::function<void(const char*)> PrintLine;

// Simple stress-test app that shows how to use Debug Callbacks feature
extern void PerformanceTest();

// Debug logger that would convert varargs to PrintLine(buffer)
extern void DebugPrintf(const char *fmt, ...);

extern void TelemetryInitialize();

extern void TelemetryTeardown();
