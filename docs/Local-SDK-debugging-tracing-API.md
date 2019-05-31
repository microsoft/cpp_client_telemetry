---
layout: page
title: Local SDK debugging / tracing API
sub_title:

---
# Aria SDK debug logging and tracing API

Aria SDK v1.8+ implements the new configurable debug logging and tracing API. This API allows a customer of Aria SDK to monitor all internal trace messages emitted by SDK. The API also allows to intercept any warning, error or fatal condition encountered by SDK. For example, if invalid input is provided by a customer to ILogger LogEvent API call, then SDK invokes a customer-supplied debug callback reporting file name, line number and severity of event. Having that event notification our customers may catch and diagnose issues happening either in their code (improper usage of SDK) or in Aria SDK code during automation test runs.

## Parameters

Three new parameters added to LogConfiguration surface in LogManager.hpp :

	```cpp
    static constexpr const char* const CFG_STR_DBG_TRACE_PATH       = "debugTracePath";
    static constexpr const char* const CFG_INT_DBG_TRACE_SIZE       = "debugTraceSize";
    static constexpr const char* const CFG_INT_DBG_TRACE_PROVIDER   = "debugTraceProvider";
	```

New configurable parameters:

- **debugTraceProvider** - trace provider type (file, ETW, OutputDebugString, console, etc.)
- **debugTracePath**     - trace provider destination path. Either filename for debug log or ETW provider name/GUID
- **debugTraceSize**     - trace file size for debug log file. This parameter is ignored for non-file providers

Trace provider types can be passed as a const char* value of **CFG_INT_DBG_TRACE_PROVIDER** parameter.

Possible debug trace provider configuration values:

- "0" - /dev/null or NUL. No-op logger
- "1" - File provider. Redirect the output to debug log file on disk. Not recommended for production use.
- "2" - OutputDebugString. Not recommended for production use.
- "3" - ETW (default). The default provider for raw text logs at **Fatal** level is **Microsoft-Applications-Telemetry-Windows** (GUID = {52393f51-e878-5f51-5890-5613b7b09cf6})
- "4" - syslog (*nix systems only)
- "5" - console. Comes in handy for experimentation with Aria SDK.

## Usage examples

### Aria SDK logs sent to ETW provider (default tracing)

Aria SDK logs its internal trace events to **Microsoft-Applications-Telemetry-Windows** (GUID = {52393f51-e878-5f51-5890-5613b7b09cf6}). This provider is off / not registered by default.

Aria SDK logging provider can be enabled using **logman** utility under elevated Command Prompt:

	```
    logman stop Microsoft-Applications-Telemetry-Windows
    logman delete Microsoft-Applications-Telemetry-Windows
    logman create trace Microsoft-Applications-Telemetry-Windows -p {52393f51-e878-5f51-5890-5613b7b09cf6} -o MATW.etl
    logman start Microsoft-Applications-Telemetry-Windows
	```
	
Once the logging is done, provider can be stopped as follows:

    logman stop Microsoft-Applications-Telemetry-Windows

[Microsoft Message Analyzer](https://technet.microsoft.com/en-us/library/jj674821.aspx) utility can be used to view ETW trace events in realtime.

Please follow instructions published in TechNet article and set up Aria (Microsoft.Applications.Telemetry.Windows) Provider ID *{52393f51-e878-5f51-5890-5613b7b09cf6}*


### Configure tracing API to log to file

	```cpp
    LogConfiguration config;
    config.traceLevelMask = 0xFFFFFFFF;                     // Enable reporting across all SDK internal modules
    config.minimumTraceLevel = ACTTraceLevel_Trace;         // Monitor Trace, Info, Warning, Error and Fatal errors
    config.SetProperty(CFG_INT_DBG_TRACE_PROVIDER, "1");    // Output to plain text log file on disk. Default filename is %TEMP%\aria-debug-${pid}.log
    config.SetProperty(CFG_INT_DBG_TRACE_SIZE, "30000000"); // Set log file size upper limit
    LogManager::Initialize(ARIA_TOKEN, config);
	```

Note that SDK Release build attempts to delete the log file on process exit, thus preventing leftover trace files.  SDK Debug build leaves the trace file on disk to simplify debugging / tracing in unit test and BVT test environments.

### Configure tracing API to log to customer-configured ETW provider

Aria SDK uses [EventWriteString](https://msdn.microsoft.com/en-us/library/windows/desktop/aa363750(v=vs.85).aspx) Windows API to send trace log lines to ETW.

Provider can be specified as a parameter under **debugTraceProvider** config key:

- **GUID** - starting with a curly brace; or
- **Provider name** - gets converted to GUID following Name-to-GUID conversion algorithm [described here](https://blogs.msdn.microsoft.com/dcook/2015/09/08/etw-provider-names-and-guids/)

Aria SDK source code contains a script that allows to compute provider GUID hash based on Provider name (etw-make-guid.cmd)

	```cpp
    LogConfiguration config;
    config.traceLevelMask = 0xFFFFFFFF;                     // Enable reporting across all SDK internal modules
    config.minimumTraceLevel = ACTTraceLevel_Trace;         // Monitor Trace, Info, Warning, Error and Fatal errors
    config.SetProperty(CFG_INT_DBG_TRACE_PROVIDER, "3");    // Output to plain text log file on disk
    configuration.SetProperty(CFG_STR_DBG_TRACE_PATH, "{12345678-1234-1234-abcd-abcdefabcdef}");    // Custom provider GUID; or
    // configuration.SetProperty(CFG_STR_DBG_TRACE_PATH, "Microsoft-Applications-Custom-Provider"); // Custom provider name
    LogManager::Initialize(ARIA_TOKEN, config);
	```

### Monitor internal SDK trace events (warning/error/fatal) using DebugEventCallback mechanism

This example shows how to use EVT_TRACE event emitted by SDK and sent to DebugEventListener.

	```
    class MyTraceListener : public DebugEventListener {
        std::atomic<unsigned> evtCount;
    
    public:
        MyTraceListener() :
            DebugEventListener(),
            evtCount(0) {};
    
        virtual void OnDebugEvent(DebugEvent & evt) override
        {
            evtCount++;
            // It is not the best idea to printf, as it is done synchronously within SDK thread.
            // This code logic is for illustration purposes only.
            // Print severity of event, file name and line number where it occured.
            printf("[%d] %s:%d\n", evt.param1, (const char *)evt.data, evt.param2);
        }
    
        virtual unsigned getEvtCount()
        {
            return evtCount.load();
        }
    };

    MyTraceListener traceListener;
    
    // Google Test example
    TEST(APITest, TracingAPI_Callback)
    {
        LogConfiguration config;
        config.traceLevelMask = 0xFFFFFFFF;
        config.minimumTraceLevel = ACTTraceLevel_Trace;
        config.SetProperty(CFG_INT_DBG_TRACE_PROVIDER, "1");      // log to file
        config.SetProperty(CFG_INT_DBG_TRACE_SIZE, "30000000");   // set max log file size to 30MB
        // This event listener is goign to print an error (filename:linenumber) whenever SDK internally experiences any error, warning or fatal event
        LogManager::AddEventListener(EVT_TRACE, traceListener);   // add event listener prior to initialize
        LogManager::Initialize(CUSTOMER_TOKEN, config);
        LogManager::GetLogger()->LogEvent("going-nowhere");       // 'going-nowhere' is invalid event name. SDK triggers an error notification to the customer
        LogManager::FlushAndTeardown();
        LogManager::RemoveEventListener(EVT_TRACE, traceListener);// remove event listener after shutdown
        // Expecting at least one error notification.
        EXPECT_GE(traceListener.getEvtCount(), 1);
    }
	```

Errors and Warnings may be triggered by SDK in situations like network connectivity failure, abnormal network condition, connection reset, out of memory, invalid input passed down to SDK API call, etc. Customers may monitor for trigger points, number of triggers at various severity levels. Then investigate the root cause either using debug log or default ETW provider output.

**DebugEvent** structure fields contain the following information:

- **evt.param1**      - event severity
- **evt.data**        - complete filename of source code module that triggered debug event
- **evt.param2**      - line number

Event message payload text is not supplied to debug callback because of SDK performance considerations.

Event message payload can be captured either using debug log file or using ETW provider hook.

## References

- ETW hook usage examples are available in [Event Tracing for Windows](https://docs.microsoft.com/en-us/windows-hardware/test/weg/instrumenting-your-code-with-etw) documentation.
- Microsoft opensource helper library [KrabsETW](https://github.com/Microsoft/krabsetw) provides a modern C++ wrapper and a .NET wrapper around the low-level ETW trace consumption functions.
- [Microsoft Message Analyzer](https://technet.microsoft.com/en-us/library/jj674821.aspx) utility can be used to view ETW trace events in realtime.
