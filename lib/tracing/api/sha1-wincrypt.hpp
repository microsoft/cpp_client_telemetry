//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef SHA1_HPP
#define SHA1_HPP

#ifdef _WIN32
#include <Windows.h>
#include <Wincrypt.h>

namespace WinCryptHelper {

    /// <summary>
    /// Compute SHA-1 hash of input buffer and save to output
    /// </summary>
    /// <param name="pData">Input buffer</param>
    /// <param name="nData">Input buffer size</param>
    /// <param name="pHashedData">Output buffer</param>
    /// <param name="nHashedData">Output buffer size</param>
    /// <returns></returns>
    bool sha1(const BYTE* pData, DWORD nData, BYTE* pHashedData, DWORD& nHashedData);

}

#endif

#endif
