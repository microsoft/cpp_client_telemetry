//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif 

#ifdef _MSC_VER
#pragma warning (disable : 4389)
#endif

#include "common/Common.hpp"
#include "LogManager.hpp"

#include <atomic>
#include <cassert>

#include "EventDecoderListener.hpp"

using namespace MAT;
using namespace std;

// 1DSCppSdkTest sandbox key
#define TOKEN   "7c8b1796cbc44bd5a03803c01c2b9d61-b6e370dd-28d9-4a52-9556-762543cf7aa7-6991"

void Configure(ILogConfiguration& config)
{
    config[CFG_STR_CACHE_FILE_PATH] = "streamer.db";
    config[CFG_INT_CACHE_FILE_SIZE] = 10 * 1024 * 1024;
    config[CFG_INT_RAM_QUEUE_SIZE] = 2 * 1024 * 1024;
    config[CFG_INT_RAM_QUEUE_BUFFERS] = 10;
    config[CFG_INT_TRACE_LEVEL_MASK] = 0xFFFFFFFF ^ 128;
    config[CFG_INT_TRACE_LEVEL_MIN] = ACTTraceLevel::ACTTraceLevel_Trace;
    config[CFG_INT_MAX_TEARDOWN_TIME] = 20;
    config[CFG_INT_MAX_PENDING_REQ] = 4;
}

void SendEvents(ILogger* pLogger, uint8_t eventCount, std::chrono::milliseconds sleepTime)
{
    for (auto i = 0; i < eventCount; ++i)
    {
        pLogger->LogEvent("Streamer.Basic");

        EventProperties event2("Streamer.Detailed",
            {
                // Key-values
                { "strKey",  "hello" },
                { "int64Key", int64_t { 1 } },
                { "dblKey",   3.14 },
                { "boolKey",  false },
                { "guidKey0", GUID_t("00000000-0000-0000-0000-000000000000") },
                { "guidKey1", GUID_t("00010203-0405-0607-0809-0A0B0C0D0E0F") },
                { "timeKey1",  time_ticks_t((uint64_t)0) },     // time in .NET ticks
                // Key-values with Pii tags
                { "piiKind.None",               EventProperty("field_value",  PiiKind_None) },
                { "piiKind.DistinguishedName",  EventProperty("/CN=Jack Frost,OU=ARIA,DC=REDMOND,DC=COM",  PiiKind_DistinguishedName) },
                { "piiKind.GenericData",        EventProperty("generic_data",  PiiKind_GenericData) },
                { "piiKind.IPv4Address",        EventProperty("127.0.0.1", PiiKind_IPv4Address) },
                { "piiKind.IPv6Address",        EventProperty("2001:0db8:85a3:0000:0000:8a2e:0370:7334", PiiKind_IPv6Address) },
                { "piiKind.MailSubject",        EventProperty("RE: test",  PiiKind_MailSubject) },
                { "piiKind.PhoneNumber",        EventProperty("+1-425-829-5875", PiiKind_PhoneNumber) },
                { "piiKind.QueryString",        EventProperty("a=1&b=2&c=3", PiiKind_QueryString) },
                { "piiKind.SipAddress",         EventProperty("sip:info@microsoft.com", PiiKind_SipAddress) },
                { "piiKind.SmtpAddress",        EventProperty("Jack Frost <jackfrost@fabrikam.com>", PiiKind_SmtpAddress) },
                { "piiKind.Identity",           EventProperty("Jack Frost", PiiKind_Identity) },
                { "piiKind.Uri",                EventProperty("http://www.microsoft.com", PiiKind_Uri) },
                { "piiKind.Fqdn",               EventProperty("www.microsoft.com", PiiKind_Fqdn) }
            });
        pLogger->LogEvent(event2);

        if (sleepTime >= std::chrono::milliseconds(0)) std::this_thread::sleep_for(sleepTime);
    }
}

TEST(BondDecoderTests, BasicTest)
{
    EventDecoderListener eventDecoderListener;
    // Register listeners for HTTP OK and ERROR
    const auto dbgEvents = { EVT_HTTP_OK, EVT_HTTP_ERROR };
    for (const auto& dbgEvt : dbgEvents)
    {
        LogManager::AddEventListener(dbgEvt, eventDecoderListener);
    }

    // Set config settings
    Configure(LogManager::GetLogConfiguration());

    // Obtains default primary logger
    auto pLogger = LogManager::Initialize(TOKEN);

    // Send 10 events with 50ms delay
    SendEvents(pLogger, 10, std::chrono::milliseconds(50));

    // Trigger upload on shutdown
    LogManager::FlushAndTeardown();

    // Unregister listeners
    for (const auto& dbgEvt : dbgEvents)
    {
        LogManager::RemoveEventListener(dbgEvt, eventDecoderListener);
    }
}

#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

