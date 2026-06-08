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
    // Reaching this point already proves the headers compiled and the program
    // linked against MSTelemetry::mat; the checks below assert real runtime
    // behavior so a broken-but-linkable build still fails.
    printf("\n-- Core API --\n");

    // LogManager configuration accepts and reports a config value.
    {
        auto& config = LogManager::GetLogConfiguration();
        config["vcpkgTestKey"] = "vcpkgTestValue";
        check(config.HasConfig("vcpkgTestKey"),
              "LogConfiguration stores and reports a config key");
    }

    // LogManager initialization returns a usable logger.
    {
        ILogger* logger = LogManager::Initialize("vcpkg-test-token");
        check(logger != nullptr, "LogManager::Initialize() returns a logger");
        LogManager::FlushAndTeardown();
    }

    // EventProperties stores the name and typed property values.
    {
        EventProperties props("TestEvent");
        props.SetProperty("strProp", "value");
        props.SetProperty("intProp", (int64_t)42);
        props.SetProperty("dblProp", 3.14);
        props.SetProperty("boolProp", true);

        check(props.GetName() == "TestEvent", "Event name round-trips");

        const auto& stored = props.GetProperties();
        check(stored.count("strProp") == 1 &&
              stored.count("intProp") == 1 &&
              stored.count("dblProp") == 1 &&
              stored.count("boolProp") == 1,
              "All four typed properties stored");
        check(stored.count("intProp") == 1 && stored.at("intProp").as_int64 == 42,
              "Int64 property value round-trips");
    }

    // PII annotation is recorded on the stored property.
    {
        EventProperties props("PiiTest");
        props.SetProperty("userId", "user@example.com", PiiKind_Identity);
        const auto& stored = props.GetProperties();
        check(stored.count("userId") == 1 &&
              stored.at("userId").piiKind == PiiKind_Identity,
              "PII kind recorded on property");
    }

    // Event priority round-trips.
    {
        EventProperties props("PriorityTest");
        props.SetPriority(EventPriority_High);
        check(props.GetPriority() == EventPriority_High, "Priority round-trips");
    }

    printf("\n=== Results: %d/%d passed ===\n", pass_count, test_count);
    return (pass_count == test_count) ? 0 : 1;
}
