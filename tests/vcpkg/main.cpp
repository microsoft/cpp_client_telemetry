// Vcpkg integration test for mstelemetry
// Verifies that find_package(MSTelemetry) works and core APIs are callable

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "LogManager.hpp"

LOGMANAGER_INSTANCE

using namespace MAT;

static int test_count = 0;
static int pass_count = 0;

static void check(bool cond, const char* name)
{
    test_count++;
    if (cond) {
        pass_count++;
        printf("  [PASS] %s\n", name);
    } else {
        printf("  [FAIL] %s\n", name);
    }
}

int main()
{
    printf("=== MSTelemetry vcpkg integration test ===\n");

    // ---- Core API tests ----
    printf("\n-- Core API --\n");

    // 1. Verify headers compile and link
    check(true, "Headers found and compiled successfully");

    // 2. LogManager configuration
    auto& config = LogManager::GetLogConfiguration();
    check(true, "LogManager::GetLogConfiguration() callable");

    // 3. LogManager initialization
    {
        ILogger* logger = LogManager::Initialize("vcpkg-test-token");
        check(logger != nullptr, "LogManager::Initialize() callable");
        LogManager::FlushAndTeardown();
    }

    // 4. EventProperties with multiple types
    {
        EventProperties props("TestEvent");
        props.SetProperty("strProp", "value");
        props.SetProperty("intProp", (int64_t)42);
        props.SetProperty("dblProp", 3.14);
        props.SetProperty("boolProp", true);
        check(props.GetName() == "TestEvent", "Event name matches");
        check(true, "SetProperty for string, int, double, bool");
    }

    // 5. Multiple event types
    {
        std::vector<std::string> names = {"App.Started", "App.PageView", "App.Error"};
        for (const auto& name : names) {
            EventProperties ep(name);
            ep.SetProperty("timestamp", (int64_t)1234567890);
        }
        check(true, "Created multiple event types");
    }

    // 6. PII annotations
    {
        EventProperties props("PiiTest");
        props.SetProperty("userId", "user@example.com", PiiKind_Identity);
        props.SetProperty("ip", "127.0.0.1", PiiKind_IPv4Address);
        check(true, "PII-annotated properties compile and link");
    }

    // 7. Event priority
    {
        EventProperties props("PriorityTest");
        props.SetPriority(EventPriority_High);
        check(true, "SetPriority compiles and links");
    }

    printf("\n=== Results: %d/%d passed ===\n", pass_count, test_count);
    return (pass_count == test_count) ? 0 : 1;
}
