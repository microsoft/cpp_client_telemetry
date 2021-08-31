//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifdef USE_WINCRYPT

#include <sha1-wincrypt.hpp>

namespace WinCryptHelper {

    /// <summary>
    /// Compute SHA-1 hash of input buffer and save to output
    /// </summary>
    /// <param name="pData">Input buffer</param>
    /// <param name="nData">Input buffer size</param>
    /// <param name="pHashedData">Output buffer</param>
    /// <param name="nHashedData">Output buffer size</param>
    /// <returns></returns>
    bool sha1(const BYTE* pData, DWORD nData, BYTE* pHashedData, DWORD& nHashedData)
    {
        bool bRet = false;
        HCRYPTPROV  hProv = NULL;
        HCRYPTHASH  hHash = NULL;

        if (!CryptAcquireContext(
            &hProv,                   // handle of the CSP
            NULL,                     // key container name
            NULL,                     // CSP name
            PROV_RSA_FULL,            // provider type
            CRYPT_VERIFYCONTEXT))     // no key access is requested
        {
            bRet = false;
            goto CleanUp;
        }

        if (!CryptCreateHash(
            hProv,                    // handle of the CSP
            CALG_SHA1,                // hash algorithm to use
            0,                        // hash key
            0,                        // reserved
            &hHash))                  // 
        {
            bRet = false;
            goto CleanUp;
        }

        if (!CryptHashData(
            hHash,                // handle of the HMAC hash object
            pData,                    // message to hash
            nData,            // number of bytes of data to add
            0))                       // flags
        {
            bRet = false;
            goto CleanUp;
        }

        if (!CryptGetHashParam(
            hHash,                // handle of the HMAC hash object
            HP_HASHVAL,               // query on the hash value
            pHashedData,                     // filled on second call
            &nHashedData,               // length, in bytes,of the hash
            0))
        {
            bRet = false;
            goto CleanUp;
        }

        bRet = true;

    CleanUp:

        if (hHash)
        {
            CryptDestroyHash(hHash);
        }

        if (hProv)
        {
            CryptReleaseContext(hProv, 0);
        }
        return bRet;
    }

}

#endif
