//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "pal/PAL.hpp"

#include <string_view>

#ifdef _WIN32
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __attribute__((visibility("default")))
#endif

extern "C" EXPORTED const char* GetSdkVersionForReload()
{
    return std::string_view(PAL::getSdkVersion()).data();
}
