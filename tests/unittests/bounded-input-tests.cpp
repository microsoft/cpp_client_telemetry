//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#include "bond/All.hpp"
#include "bond/CompactBinaryProtocolReader.hpp"
#include "bond/generated/BondConstTypes.hpp"
#include "common/Common.hpp"
#include "utils/FileUtils.hpp"
#include "utils/Utils.hpp"

#if !defined(_WIN32) && !defined(ANDROID)
#include "pal/posix/sysinfo_sources.hpp"
#endif
#if defined(__linux__) && !defined(ANDROID)
#include <pthread.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace MAT;

class BoundedInputTests : public testing::Test
{
   protected:
    std::string path = GetTempDirectory() + "bounded-input-" + PAL::generateUuidString();

    ~BoundedInputTests() override
    {
        FileDelete(path.c_str());
    }

    void write(const std::string& contents)
    {
        FILE* file = FileOpen(path.c_str(), "wb");
        ASSERT_NE(file, nullptr);
        const size_t written = fwrite(contents.data(), 1, contents.size(), file);
        const int closed = FileClose(file);
        ASSERT_EQ(written, contents.size());
        ASSERT_EQ(closed, 0);
    }
};

TEST_F(BoundedInputTests, BoundsMetadataAtExactByteLimit)
{
    EXPECT_EQ(boundedSystemInfo(static_cast<const char*>(nullptr)), "");
    EXPECT_EQ(boundedSystemInfo("ordinary"), "ordinary");
    const std::string exact(MAX_SYSTEM_INFO_VALUE_SIZE, 'x');
    EXPECT_EQ(boundedSystemInfo(exact.c_str()), exact);
    EXPECT_EQ(boundedSystemInfo((exact + "extra").c_str()), exact);
}

TEST_F(BoundedInputTests, DoesNotSplitUtf8Metadata)
{
    const std::string prefix(MAX_SYSTEM_INFO_VALUE_SIZE - 1, 'x');
    EXPECT_EQ(boundedSystemInfo((prefix + "\xc3\xa9").c_str()), prefix);
    const std::string exact(MAX_SYSTEM_INFO_VALUE_SIZE - 2, 'x');
    EXPECT_EQ(boundedSystemInfo((exact + "\xc3\xa9" + "z").c_str()), exact + "\xc3\xa9");
}

#ifdef _WIN32
TEST_F(BoundedInputTests, BoundsUtf16BeforeConversion)
{
    const std::wstring input(MAX_SYSTEM_INFO_VALUE_SIZE + 1, L'x');
    EXPECT_EQ(boundedSystemInfo(input.c_str()), std::string(MAX_SYSTEM_INFO_VALUE_SIZE, 'x'));
    const std::wstring unicode(MAX_SYSTEM_INFO_VALUE_SIZE, L'\u00e9');
    EXPECT_EQ(boundedSystemInfo(unicode.c_str()).size(), MAX_SYSTEM_INFO_VALUE_SIZE);
}
#endif

TEST_F(BoundedInputTests, RejectsOversizedFilesInsteadOfParsingAPrefix)
{
    const std::string exact(MAX_FILE_CONTENTS_SIZE, 'x');
    write(exact);
    EXPECT_EQ(FileGetContents(path.c_str()), exact);
    write(exact + "x");
    EXPECT_TRUE(FileGetContents(path.c_str()).empty());
    write(std::string(100 * 1024, 'x'));
    EXPECT_TRUE(FileGetContents(path.c_str()).empty());
}

TEST_F(BoundedInputTests, CountsPhysicalCrlfBytesAtTheFileLimit)
{
    std::string exact;
    for (size_t i = 0; i < MAX_FILE_CONTENTS_SIZE / 2; ++i)
    {
        exact += "\r\n";
    }
    write(exact);
    EXPECT_EQ(FileGetContents(path.c_str()), exact);
    write(exact + "x");
    EXPECT_TRUE(FileGetContents(path.c_str()).empty());
}

TEST_F(BoundedInputTests, DoesNotTreatControlZAsEndOfFile)
{
    const std::string prefix = "1234567890\r\noriginal-id\r\n";
    const std::string exact = prefix + '\x1a' +
                              std::string(MAX_FILE_CONTENTS_SIZE - prefix.size() - 1, 'x');
    write(exact);
    EXPECT_EQ(FileGetContents(path.c_str()), exact);
    write(exact + "x");
    EXPECT_TRUE(FileGetContents(path.c_str()).empty());
}

TEST_F(BoundedInputTests, RejectsImpossibleContainerCountsBeforeAllocation)
{
    const std::vector<uint8_t> input = {bond_lite::BT_STRING, 0xff, 0xff, 0xff, 0xff, 0x0f};
    bond_lite::CompactBinaryProtocolReader reader(input);
    uint32_t size;
    uint8_t type;
    EXPECT_FALSE(reader.ReadContainerBegin(size, type));
}

TEST_F(BoundedInputTests, EnforcesContainerElementLimit)
{
    std::vector<uint8_t> input = {bond_lite::BT_BOOL, 0x80, 0x80, 0x04};
    input.resize(input.size() + bond_lite::CompactBinaryProtocolReader::MAX_CONTAINER_ELEMENTS, 0);
    bond_lite::CompactBinaryProtocolReader reader(input);
    uint32_t size;
    uint8_t type;
    EXPECT_TRUE(reader.ReadContainerBegin(size, type));
    input[1] = 0x81;
    input.push_back(0);
    bond_lite::CompactBinaryProtocolReader oversizedReader(input);
    EXPECT_FALSE(oversizedReader.ReadContainerBegin(size, type));
}

#if !defined(_WIN32) && !defined(ANDROID)
TEST_F(BoundedInputTests, ReadsOnlyArgvZeroFromLongNulSeparatedCommandLine)
{
    const std::string executable = "/opt/app with spaces\nin-name";
    write(executable + '\0' + std::string(100 * 1024, 'x'));
    sysinfo_sources sources;
    sources.add("appId", {path.c_str(), sysinfo_selector::first_null});
    EXPECT_EQ(sources.get("appId"), executable);
}

TEST_F(BoundedInputTests, BoundsLongArgvZeroAndRawMetadata)
{
    write(std::string(100 * 1024, 'x') + '\0' + "argument");
    sysinfo_sources sources;
    sources.add("appId", {path.c_str(), sysinfo_selector::first_null});
    sources.add("raw", {path.c_str(), sysinfo_selector::raw});
    EXPECT_EQ(sources.get("appId"), std::string(MAX_SYSTEM_INFO_VALUE_SIZE, 'x'));
    EXPECT_EQ(sources.get("raw").size(), MAX_SYSTEM_INFO_VALUE_SIZE);
}

TEST_F(BoundedInputTests, ReadsOsReleaseKeysWithoutSuffixMatchesOrRegex)
{
    write(
        "VERSION_ID=\"24.04\"\nID_LIKE=wrong\n# ID=wrong\nID=\"ubuntu\"\n"
        "VERSION='24.04 (LTS)'\n");
    sysinfo_sources sources;
    sources.add("osName", {path.c_str(), sysinfo_selector::key_value, "ID"});
    sources.add("osVer", {path.c_str(), sysinfo_selector::key_value, "VERSION_ID"});
    sources.add("osRel", {path.c_str(), sysinfo_selector::key_value, "VERSION"});
    EXPECT_EQ(sources.get("osName"), "ubuntu");
    EXPECT_EQ(sources.get("osVer"), "24.04");
    EXPECT_EQ(sources.get("osRel"), "24.04 (LTS)");
}

TEST_F(BoundedInputTests, BoundsOsReleaseValuesAndReadsUnterminatedFinalLine)
{
    write("ID=linux\nVERSION=" + std::string(100 * 1024, 'x'));
    sysinfo_sources sources;
    sources.add("version", {path.c_str(), sysinfo_selector::key_value, "VERSION"});
    EXPECT_EQ(sources.get("version").size(), MAX_SYSTEM_INFO_VALUE_SIZE);
    write("ID=linux");
    sysinfo_sources finalLine;
    finalLine.add("id", {path.c_str(), sysinfo_selector::key_value, "ID"});
    EXPECT_EQ(finalLine.get("id"), "linux");
}

TEST_F(BoundedInputTests, ReadsOnlyFirstLineAndRetainsCache)
{
    write("Linux build\n" + std::string(100 * 1024, 'x'));
    sysinfo_sources sources;
    sources.add("build", {path.c_str(), sysinfo_selector::first_line});
    EXPECT_EQ(sources.get("build"), "Linux build");
    write("changed");
    EXPECT_EQ(sources.get("build"), "Linux build");
}
#endif

#if defined(__linux__) && !defined(ANDROID)
TEST_F(BoundedInputTests, ActualCommandLineOnSmallStack)
{
    sysinfo_sources sources;
    sources.add("appId", {"/proc/self/cmdline", sysinfo_selector::first_null});
    std::string result;
    struct State
    {
        sysinfo_sources& sources;
        std::string& result;
    } state{sources, result};
    pthread_attr_t attributes;
    ASSERT_EQ(pthread_attr_init(&attributes), 0);
    ASSERT_EQ(pthread_attr_setstacksize(&attributes, 64 * 1024), 0);
    pthread_t thread;
    const int created = pthread_create(&thread, &attributes, [](void* pointer) -> void*
                                       {
        auto& state = *static_cast<State*>(pointer);
        state.result = state.sources.get("appId");
        return nullptr; }, &state);
    pthread_attr_destroy(&attributes);
    ASSERT_EQ(created, 0);
    ASSERT_EQ(pthread_join(thread, nullptr), 0);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.find('\0'), std::string::npos);
    EXPECT_LE(result.size(), MAX_SYSTEM_INFO_VALUE_SIZE);
}

TEST_F(BoundedInputTests, InitializesTelemetryWithA100KbCommandLine)
{
    std::string padding(100 * 1024, 'x');
    const char* filter = "--gtest_filter=BoundedInputTests.ActualCommandLineOnSmallStack";
    const pid_t child = fork();
    ASSERT_NE(child, -1);
    if (child == 0)
    {
        execl("/proc/self/exe", "/proc/self/exe", filter, padding.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }
    int status;
    ASSERT_EQ(waitpid(child, &status, 0), child);
    ASSERT_TRUE(WIFEXITED(status));
    EXPECT_EQ(WEXITSTATUS(status), 0);
}
#endif
