// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "utils/Utils.hpp"

#if defined(_DEBUG) && defined(_WIN32) && !defined(_INC_WINDOWS)
extern "C" unsigned long __stdcall IsDebuggerPresent();
#endif

class TestStatusLogger : public testing::EmptyTestEventListener {
  public:
    virtual void OnTestStart(testing::TestInfo const& test) override
    {
        LOG_INFO("--- %s.%s", test.test_case_name(), test.name());
    }

    virtual void OnTestEnd(testing::TestInfo const& test) override
    {
        LOG_INFO("=== %s.%s [%s]", test.test_case_name(), test.name(), test.result()->Passed() ? "OK" : "FAILED");
    }
};
#ifdef _MSC_VER
#pragma warning(suppress:4447) // 'main' signature found without threading model. Consider using 'int main(Platform::Array<Platform::String^>^ args)'.
#define MAIN_CDECL __cdecl
#else
#define MAIN_CDECL
#endif

int MAIN_CDECL main(int argc, char** argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    ::testing::UnitTest::GetInstance()->listeners().Append(new TestStatusLogger());

    ILogConfiguration logConfig;
    RuntimeConfig_Default runtimeConfig(logConfig);
    PAL::initialize(runtimeConfig);
    int result = RUN_ALL_TESTS();
    PAL::shutdown();

#if defined(_DEBUG) && defined(_WIN32)
    if (IsDebuggerPresent()) {
        printf("Press Enter to close...");
        getchar();
    }
#endif

    return result;
}
