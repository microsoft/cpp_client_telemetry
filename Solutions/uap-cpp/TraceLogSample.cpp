#include "TraceLogSample.hpp"

TRACELOGGING_DEFINE_PROVIDER(
    g_hMyComponentProvider,
    "Microsoft.Applications.Telemetry.Windows",
    // {a6ce7a34-333a-5ae5-941a-018327c608a0}
       (0x5ECB0BAC, 0xB930, 0x47F5, 0xA8, 0xA4, 0xE8, 0x25, 0x35, 0x29, 0xED, 0xB7),
    // (0xa6ce7a34, 0x333a, 0x5ae5, 0x94, 0x1a, 0x01, 0x83, 0x27, 0xc6, 0x08, 0xa0),
    TraceLoggingOptionMicrosoftPartnerTelemetry());

extern "C" void testTraceLogging() {
    // Register the provider
    int status = TraceLoggingRegister(g_hMyComponentProvider);
    if (SUCCEEDED(status)) {
        OutputDebugStringA("Provider successfully registered\n");
        // Log an event
        for (int i = 0; i < 10; i++) {
            testOneEvent();
        }
    }
    else {
        OutputDebugStringA("There was an error registering a provider!\n");
    }

}

extern "C" void testOneEvent() {
    char level = 255;

    OutputDebugStringA("testOneEvent\n");
    bool status = TraceLoggingProviderEnabled(g_hMyComponentProvider, 0, 0);
    OutputDebugStringA((status) ?
        "Provider enabled." :
        "Provider disabled!"
    );

    TraceLoggingWrite(
        g_hMyComponentProvider,
        "SampleEvent1",
//      TraceLoggingKeyword(MICROSOFT_KEYWORD_TELEMETRY),
        TraceLoggingEventTag(MICROSOFT_EVENTTAG_REALTIME_LATENCY),
        TraceLoggingEventTag(MICROSOFT_EVENTTAG_CRITICAL_PERSISTENCE),
        TraceLoggingString("Hello TraceLogging!", "GreetingField")
    );
}

extern "C" void testUnregister() {
    // Unregister
    TraceLoggingUnregister(g_hMyComponentProvider);
}
