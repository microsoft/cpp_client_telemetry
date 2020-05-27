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

#include "LogManager.hpp"

using namespace MAT;

class ETWLogConfiguration : public ILogConfiguration
{
   public:
    ETWLogConfiguration() :
        ILogConfiguration()
    {
        (*this)["name"] = "ETWModule";
        (*this)["version"] = "1.0.0";
        (*this)[CFG_INT_TRACE_LEVEL_MASK] = 0;
        (*this)[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel_Fatal;
        (*this)[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_ETWBackCompat;
        (*this)["schema"]["integrity"] = false;
    } 
};

using ETWLogManager = MAT::LogManagerBase<ETWLogConfiguration>;

ETWLogConfiguration etwLogConfiguration;

DEFINE_LOGMANAGER(ETWLogManager, ETWLogConfiguration);

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

class ETWLoggerFixture : public benchmark::Fixture
{
    ILogger* logger;
    uint64_t m_detailLevel;

   public:

    ETWLoggerFixture(uint64_t detailLevel) :
        logger(nullptr)
    {
        m_detailLevel = detailLevel;
    }

    void SetUp(const ::benchmark::State&)
    {
        const char* token = "deadbeefdeadbeefdeadbeef00000075";
        etwLogConfiguration["schema"][CFG_INT_DETAIL_LEVEL] = m_detailLevel;
        logger = ETWLogManager::Initialize(token, etwLogConfiguration);
    }

    void LogEvent()
    {
        EventProperties event(
            "My.Event.Name",
            {
                { "result", "Success" },
                { "double", 5.6872 }
            });
        logger->LogEvent(event);
    }

    void TearDown(const ::benchmark::State&)
    {
        ETWLogManager::FlushAndTeardown();
    }
};

#define ETWLoggerFixtureX(X)                             \
    class ETWLoggerFixture_##X : public ETWLoggerFixture \
    {                                                    \
       public:                                           \
        ETWLoggerFixture_##X() : ETWLoggerFixture(X){};  \
    };

#define BENCHMARK_LOGEVENT  \
    (benchmark::State & st) \
    {                       \
        for (auto _ : st)   \
        {                   \
            LogEvent();     \
        }                   \
    };

// Benchmark various levels of decoration
ETWLoggerFixtureX(0);
ETWLoggerFixtureX(1);
ETWLoggerFixtureX(2);
ETWLoggerFixtureX(3);

BENCHMARK_F(ETWLoggerFixture_0, LogEvent) BENCHMARK_LOGEVENT
BENCHMARK_F(ETWLoggerFixture_1, LogEvent) BENCHMARK_LOGEVENT
BENCHMARK_F(ETWLoggerFixture_2, LogEvent) BENCHMARK_LOGEVENT
BENCHMARK_F(ETWLoggerFixture_3, LogEvent) BENCHMARK_LOGEVENT

TEST(PerfTests, ETW_Writer)
{
    // Run the benchmark
    ::benchmark::Initialize(0, {});
    ::benchmark::RunSpecifiedBenchmarks();
}

#endif

#endif