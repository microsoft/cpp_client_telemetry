// Copyright (c) Microsoft. All rights reserved.
#ifdef _WIN32
//
// Currently these perf tests only cover ETW path and thus Windows-only.
// Prerequisites:
// - Google Benchmark checked out
// - x64
#ifdef HAVE_BENCHMARK
#if __has_include("benchmark/benchmark.h")
/* Benchmarking is enabled */
#include "benchmark/benchmark.h"
#else
/* Benchmarking is not enabled */
#undef HAVE_BENCHMARK
#endif
#endif

#ifdef HAVE_BENCHMARK
#include "mat/config.h"

#include "common/Common.hpp"

#include "CsProtocol_types.hpp"

#include <LogManager.hpp>
#include <atomic>
#include <cassert>

#include "PayloadDecoder.hpp"

#include "mat.h"

#ifdef HAVE_MAT_JSONHPP
#include "json.hpp"
#endif

#include "CorrelationVector.hpp"

static ILogger* logger = nullptr;

static void ETW_Initialize()
{
    const char* token = "deadbeefdeadbeefdeadbeef00000075";
    auto& configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Fatal;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_ETWBackCompat;
    logger = LogManager::Initialize(token, configuration);
}

static void ETW_Teardown()
{
    LogManager::FlushAndTeardown();
}

static void ETW_LogEvent(ILogger* myLogger)
{
    EventProperties event;
    std::string evtType = "My.Record.BaseType";
    event.SetName("MyProduct.TaggedEvent");
    event.SetType(evtType);
    event.SetProperty("result", "Success");
    event.SetProperty("random", rand());
    event.SetProperty("secret", 5.6872);
    event.SetLatency(EventLatency_Normal);
    event.SetLevel(DIAG_LEVEL_REQUIRED);
    myLogger->LogEvent(event);
}

void BM_ETW_Writer(benchmark::State& state)
{
    for (auto _ : state)
        ETW_LogEvent(logger);
}

TEST(PerfTests, ETW_Writer)
{
    ETW_Initialize();
    BENCHMARK(BM_ETW_Writer);
    // Run the benchmark
    ::benchmark::Initialize(0, {});
    ::benchmark::RunSpecifiedBenchmarks();
    ETW_Teardown();
}

#endif

#endif