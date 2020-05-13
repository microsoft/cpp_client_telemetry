// Copyright (c) Microsoft. All rights reserved.
#ifdef _WIN32
//
// Currently these perf tests only cover ETW path.
// Prerequisites:
// - Win32
// - Google Benchmark submodule
// - x64 arch
#ifdef HAVE_BENCHMARK
#if __has_include("benchmark/benchmark.h")
/* Benchmarking is enabled */
#include "benchmark/benchmark.h"
#pragma comment(lib, "benchmark.lib")
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

static void ETW_Initialize(uint64_t detailLevel)
{
    const char* token = "deadbeefdeadbeefdeadbeef00000075";
    auto& configuration = LogManager::GetLogConfiguration();
    configuration[CFG_INT_TRACE_LEVEL_MASK] = 0;
    configuration[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Fatal;
    configuration[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_ETWBackCompat;
    configuration["schema"][CFG_INT_DETAIL_LEVEL] = detailLevel;
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

void BM_ETW_Writer_0(benchmark::State& state)
{
    ETW_Initialize(0);
    for (auto _ : state)
        ETW_LogEvent(logger);
    ETW_Teardown();
}

void BM_ETW_Writer_1(benchmark::State& state)
{
    ETW_Initialize(1);
    for (auto _ : state)
        ETW_LogEvent(logger);
    ETW_Teardown();
}

void BM_ETW_Writer_3(benchmark::State& state)
{
    ETW_Initialize(3);
    for (auto _ : state)
        ETW_LogEvent(logger);
    ETW_Teardown();
}

TEST(PerfTests, ETW_Writer)
{
    BENCHMARK(BM_ETW_Writer_0);
    BENCHMARK(BM_ETW_Writer_1);
    BENCHMARK(BM_ETW_Writer_3);
    // Run the benchmark
    ::benchmark::Initialize(0, {});
    ::benchmark::RunSpecifiedBenchmarks();
}

#endif

#endif