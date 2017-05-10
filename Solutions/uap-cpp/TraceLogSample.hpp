#pragma once

#include <windows.h>
#include <TraceLoggingProvider.h>

TRACELOGGING_DECLARE_PROVIDER(g_hMyComponentProvider);

#define ARIA_PROVIDER_GUID (0x5ECB0BAC, 0xB930, 0x47F5, 0xA8, 0xA4, 0xE8, 0x25, 0x35, 0x29, 0xED, 0xB7)
                         //(0xa6ce7a34, 0x333a, 0x5ae5, 0x94, 0x1a, 0x01, 0x83, 0x27, 0xc6, 0x08, 0xa0)

#define TraceLoggingOptionMicrosoftPartnerTelemetry()       TraceLoggingOptionGroup ARIA_PROVIDER_GUID

#define MICROSOFT_KEYWORD_CRITICAL_DATA 0x0000800000000000 // Bit 47
#define MICROSOFT_KEYWORD_MEASURES      0x0000400000000000 // Bit 46
#define MICROSOFT_KEYWORD_TELEMETRY     0x0000200000000000 // Bit 45
#define MICROSOFT_KEYWORD_RESERVED_44   0x0000100000000000 // Bit 44 (reserved for future assignment)

#define MICROSOFT_EVENTTAG_CORE_DATA            0x00080000
#define MICROSOFT_EVENTTAG_INJECT_XTOKEN        0x00100000

#define MICROSOFT_EVENTTAG_REALTIME_LATENCY     0x00200000
#define MICROSOFT_EVENTTAG_NORMAL_LATENCY       0x00400000

#define MICROSOFT_EVENTTAG_CRITICAL_PERSISTENCE 0x00800000
#define MICROSOFT_EVENTTAG_NORMAL_PERSISTENCE   0x01000000

#define MICROSOFT_EVENTTAG_DROP_PII             0x02000000
#define MICROSOFT_EVENTTAG_HASH_PII             0x04000000
#define MICROSOFT_EVENTTAG_MARK_PII             0x08000000

#define MICROSOFT_FIELDTAG_DROP_PII             0x04000000
#define MICROSOFT_FIELDTAG_HASH_PII             0x08000000

extern "C" {

    void testTraceLogging();
    void testOneEvent();
    void testUnregister();

};