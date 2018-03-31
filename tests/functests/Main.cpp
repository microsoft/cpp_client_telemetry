// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"

#ifdef ARIASDK_PAL_SKYPE
#include <auf/auf_log2_utils.hpp>
#endif

#if defined(_DEBUG) && defined(_WIN32) && !defined(_INC_WINDOWS)
extern "C" unsigned long __stdcall IsDebuggerPresent();
#endif

class TestStatusLogger : public testing::EmptyTestEventListener {
  public:
    virtual void OnTestStart(testing::TestInfo const& test) override
    {
        using namespace testing;
        LOG_INFO("--- %s.%s", test.test_case_name(), test.name());
    }

    virtual void OnTestEnd(testing::TestInfo const& test) override
    {
        using namespace testing;
        LOG_INFO("=== %s.%s [%s]", test.test_case_name(), test.name(), test.result()->Passed() ? "OK" : "FAILED");
    }
};

#pragma warning(suppress:4447) // 'main' signature found without threading model. Consider using 'int main(Platform::Array<Platform::String^>^ args)'.
int main(int argc, char** argv)
{
    ::testing::InitGoogleMock(&argc, argv);

#ifdef ARIASDK_PAL_SKYPE
    auf::g_enablePreloadAtStartup = false;
    auf::LogFactory::instance().addAppender(
        auf::ILogAppenderPtr(new auf::OfstreamLogAppender("debug.log"), false),
        auf::AF_KeepOverStop);
#endif
    ::testing::UnitTest::GetInstance()->listeners().Append(new TestStatusLogger());

    int result = RUN_ALL_TESTS();

#if defined(_DEBUG) && defined(_WIN32)
    if (IsDebuggerPresent()) {
        printf("Press Enter to close...");
        getchar();
    }
#endif

    return result;
}
