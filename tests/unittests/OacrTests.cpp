//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifdef _WIN32
#include "common/Common.hpp"

using namespace testing;

TEST(OacrTests, BuildMachineOnly_VerifyOacrOutputFolderExists)
{
    char *buildNumber;

    // The BUILD_BUILDID variable should only exist on a build machine
    if (0 == _dupenv_s(&buildNumber, nullptr, "BUILD_BUILDID") && (buildNumber != nullptr))
    {
        LPCSTR outputFolder = "F:\\OACR\\Output";        // Output folder that must exist
        DWORD attributes = GetFileAttributesA(outputFolder);
        bool oacrOutputFolderExists = (attributes != INVALID_FILE_ATTRIBUTES) && ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
        ASSERT_TRUE(oacrOutputFolderExists);
        free(buildNumber);
    }
}

TEST(OacrTests, BuildMachineOnly_VerifyThisBuildHasNoOacrErrors)
{
    char *buildNumber;

    // The BUILD_BUILDID variable should only exist on a build machine
    if (0 == _dupenv_s(&buildNumber, nullptr, "BUILD_BUILDID") && (buildNumber != nullptr))
    {
        char pathToFile[MAX_PATH];
        snprintf(pathToFile, _countof(pathToFile), "F:\\OACR\\Output\\%s.OacrWarnings.xml", buildNumber);
        DWORD attributes = GetFileAttributesA(pathToFile);
        bool oacrWarningsExist = (attributes != INVALID_FILE_ATTRIBUTES);
        ASSERT_FALSE(oacrWarningsExist);
        free(buildNumber);
    }
}
#endif
