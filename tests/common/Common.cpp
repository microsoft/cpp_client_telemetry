//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Common.hpp"
#include "zlib.h"
#include "utils/Utils.hpp"
#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <processthreadsapi.h>
#elif defined(linux)
#include <malloc.h>
#include <unistd.h>
#endif

namespace testing {

    MATSDK_LOG_INST_COMPONENT_NS("Testing", "Unit testing helpers");

    CsProtocol::Value toCsProtocolValue(const std::string& val)
    {
        CsProtocol::Value temp;
        temp.stringValue = val;
        return temp;
    }

    CsProtocol::Value toCsProtocolValue(bool val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueBool;
        temp.longValue = val;
        return temp;
    }


    CsProtocol::Value toCsProtocolValue(double val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueDouble;
        temp.doubleValue = val;
        return temp;
    }


    CsProtocol::Value toCsProtocolValue(int64_t val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueInt64;
        temp.longValue = val;
        return temp;
    }

    CsProtocol::Value toCsProtocolValue(uint64_t val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueArrayUInt64;
        temp.longValue = val;
        return temp;
    }


    CsProtocol::Value toCsProtocolValue(MAT::EventLatency val)
    {
        CsProtocol::Value temp;
        temp.type = CsProtocol::ValueArrayInt32;
        temp.longValue = (int)val;
        return temp;
    }

    /// <summary>
    /// Compress buffer from source to dest.
    /// </summary>
    /// <param name="source"></param>
    /// <param name="sourceLen"></param>
    /// <param name="dest"></param>
    /// <param name="destLen"></param>
    /// <param name="prependSize"></param>
    /// <returns></returns>
    bool Compress(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool prependSize)
    {
        if ((!source) || (!sourceLen))
            return false;

        *dest = NULL;
        destLen = 0;

        // Compressing variables
        uLong compSize = compressBound((uLong)sourceLen);

        // Allocate memory for the new compressed buffer
        size_t reserved = ((unsigned)prependSize * sizeof(uint32_t));
        char* compBody = new char[std::max(compSize, ((uLong)sourceLen)) + reserved];
        if (compBody != NULL)
        {
            if (prependSize)
            {
                // Remember source uncompressed size if requested
                uint32_t *s = (uint32_t*)(compBody);
                (*s) = (uint32_t)sourceLen; // truncate this to 32-bit, we do not support 3+ TB blobs
            }
            // Deflate
            int res = compress2((Bytef *)(compBody + reserved), &compSize, (Bytef *)source, (uLong)sourceLen, Z_BEST_SPEED);
            if (res != Z_OK)
            {
                LOG_ERROR("Compression failed, error=%u", res);
                delete[] compBody;
                compBody = NULL;
                return false;
            }
            else
            {
                *dest = compBody;
                destLen = compSize + reserved;
                return true;
            }
        }
        // OOM
        return false;
    }

    /// <summary>
    /// Expand buffer from source to dest.
    /// </summary>
    /// <param name="source"></param>
    /// <param name="sourceLen"></param>
    /// <param name="dest"></param>
    /// <param name="destLen"></param>
    /// <param name="sizeAtZeroIndex"></param>
    /// <returns></returns>
    bool Expand(const char* source, size_t sourceLen, char** dest, size_t& destLen, bool sizeAtZeroIndex)
    {
        if (!(source) || !(sourceLen))
            return false;

        *dest = NULL;

        unsigned reserved = (unsigned)sizeAtZeroIndex * sizeof(uint32_t);
        // Get uncompressed size at zero offset.
        if (sizeAtZeroIndex)
        {
            uint32_t s32 = *((uint32_t*)(source));
            uint64_t s64 = (sourceLen >= sizeof(uint64_t)) ? *((uint64_t*)(source)) : 0;
            // If we are reading 64-bit generated legacy DB, step 32-bit forward to
            // skip zero-padding in most-significant DWORD on Intel architecture
            if ((s64 - s32) == 0)
                reserved += sizeof(uint32_t);
            destLen = s32;
        }

        // Allocate memory for the new uncompressed buffer
        if (destLen > 0)
        {
            try {
                char* decompBody = new char[destLen];
                if (source != NULL)
                {
                    // Inflate
                    uLongf len = (uLongf)destLen;
                    int res = uncompress((Bytef *)decompBody, &len, (const Bytef *)(source + reserved), (uLong)(sourceLen - reserved));
                    if ((res != Z_OK) || (len != destLen))
                    {
                        LOG_ERROR("Decompression failed, error=%d, len=%z, destLen=%z", res, len, (unsigned int)destLen);
                        delete[] decompBody;
                        return false;
                    }
                    *dest = decompBody;
                    destLen = len;
                    return true;
                }
            }
            catch (std::bad_alloc&) {
                LOG_ERROR("Decompression failed (out of memory): destLen=%u", destLen);
                dest = NULL;
                destLen = 0;
            }
        }

        // OOM
        return false;
    }

	EventProperties CreateSampleEvent(const char *name, EventPriority prio)
	{
#ifdef _WIN32
    	/* Test for Win32 GUID type, specific to Windows only */
    	GUID win_guid;
    	win_guid.Data1 = 0;
    	win_guid.Data2 = 1;
    	win_guid.Data3 = 2;

    	for (uint8_t i = 0; i < 8; i++)
    	{	
        	win_guid.Data4[i] = i;
    	}
#endif

    	// GUID constructor from byte[16]
    	const uint8_t guid_b[16] = {
        	0x03, 0x02, 0x01, 0x00,
        	0x05, 0x04,
        	0x07, 0x06,
        	0x08, 0x09,
        	0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

    	GUID_t guid_c(
        	0x00010203,
        	0x0405,
        	0x0607,
        	{ 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F }
    	);

    	const GUID_t guid_d;

    	// Prepare current time in UTC (seconds precision)
    	std::time_t t = std::time(nullptr);
    	std::gmtime(&t);

    	/* С++11 constructor for Visual Studio 2015: this is the most JSON-lookalike syntax that makes use of C++11 initializer lists. */
    	EventProperties props(name,
        	{
    	#ifdef _MSC_VER
        	    { "_MSC_VER", _MSC_VER },
    	#endif
            	{ "piiKind.None",               EventProperty("jackfrost",  PiiKind_None) },
            	{ "piiKind.DistinguishedName",  EventProperty("/CN=Jack Frost,OU=PIE,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
            	{ "piiKind.GenericData",        EventProperty("jackfrost",  PiiKind_GenericData) },
            	{ "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
            	{ "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
            	{ "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
            	{ "piiKind.PhoneNumber",        EventProperty("+1-613-866-6960", PiiKind_PhoneNumber) },
            	{ "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
            	{ "piiKind.SipAddress",         EventProperty("sip:jackfrost@microsoft.com", PiiKind_SipAddress) },
            	{ "piiKind.SmtpAddress",        EventProperty("Jack Frost <jackfrost@microsoft.com>", PiiKind_SmtpAddress) },
            	{ "piiKind.Identity",           EventProperty("Jack Frost", PiiKind_Identity) },
            	{ "piiKind.Uri",                EventProperty("http://www.microsoft.com", PiiKind_Uri) },
            	{ "piiKind.Fqdn",               EventProperty("www.microsoft.com", PiiKind_Fqdn) },

            	{ "strKey",   "hello" },
            	{ "strKey2",  "hello2" },
            	{ "int64Key", (int64_t)1L },
           	 	{ "dblKey",   3.14 },
            	{ "boolKey",  false },

            	{ "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
            	{ "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
            	{ "guidKey2", GUID_t(guid_b) },
            	{ "guidKey3", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
            	{ "guidKey4", GUID_t(guid_c) },

            	{ "timeKey1",  time_ticks_t((uint64_t)0) },     // ticks   precision
            	{ "timeKey2",  time_ticks_t(&t) }               // seconds precision
        	});
#ifdef _WIN32
    	props.SetProperty("win_guid", GUID_t(win_guid));
#endif
    	props.SetPriority(prio);
    	props.SetLevel(DIAG_LEVEL_REQUIRED);

    	return props;
	}

	std::string GetUniqueDBFileName()
	{
		std::string fname = std::to_string(MAT::GetCurrentProcessId());
		fname.insert(0, "file_");
		fname.append(".db");
        return fname;
	}

	void LogMemUsage(const char* label) 
	{
#ifdef DEBUG_PERF
#ifdef _WIN32
        DWORD processID = ::GetCurrentProcessId();
        HANDLE hProcess;
        PROCESS_MEMORY_COUNTERS pmc;
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                PROCESS_VM_READ,
                FALSE, processID);
        if (NULL == hProcess)
            return;
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            printf("Heap Usage- %s ...  %ld\n", label, pmc.WorkingSetSize);
        }
        CloseHandle(hProcess);
#elif defined(linux)
        struct mallinfo mem = mallinfo();
        printf("Heap Usage- %s ...  %ld\n", label, mem.uordblks + mem.hblkhd);
#else
        UNREFERENCED_PARAMETER(label);
#endif
#else
        UNREFERENCED_PARAMETER(label);
#endif
    }

    void LogCpuUsage(const char* label)
    {
#ifdef DEBUG_PERF
    	static int64_t lastTime = GetUptimeMs();
    	int64_t currTime = GetUptimeMs();
    	printf("Time taken- %s: ... %lld\n", label, (currTime - lastTime));
    	lastTime = currTime;
#else
    	UNREFERENCED_PARAMETER(label);
#endif        

    }

} // namespace testing

