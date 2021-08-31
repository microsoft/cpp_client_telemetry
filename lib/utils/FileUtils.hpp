//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef LIB_FILEUTILS_HPP
#define LIB_FILEUTILS_HPP
// Cross-platform C-style file utils that work well with UTF-8 filenames

#include "ctmacros.hpp"
#include "pal/PAL.hpp"

namespace MAT_NS_BEGIN
{
    size_t      FileGetSize(const char* filename);
    int         FileDelete(const char* filename);
    std::FILE*  FileOpen(const char* filename, const char *mode);
    int         FileClose(std::FILE* handle);
    std::string FileGetContents(const char *filename);
    bool        FileWrite(const char* filename, const char* contents);
    bool        FileExists(const char* name);

} MAT_NS_END

#endif

