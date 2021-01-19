
# Using Privacy Tags

In order to set privacy tags, the C++ SDK exposes the functionality on it's API.

> ***NOTE***: To be able to send an event on UTC mode you need to set CFG_INT_SDK_MODE flag on the LogManager configuration:
>
>```cpp
>config[CFG_INT_SDK_MODE] = SdkModeTypes::SdkModeTypes_UTCCommonSchema;
>```

To set a tag in code you can use the following syntax using the SetProperty method:

```cpp
EventProperties event(eventName);

std::string evtType = "My.Record.BaseType";
event.SetName("MyProduct.TaggedEvent");
event.SetType(evtType);
event.SetProperty("result", "Success");
event.SetProperty("random", rand());
event.SetProperty("secret", 5.6872);
event.SetProperty("seq", (uint64_t)i);
event.SetProperty(COMMONFIELDS_EVENT_PRIVTAGS, PDT_BrowsingHistory);
event.SetLatency(latency);
logger->LogEvent(event);
```

You can also use the syntax to fill a collection:

```cpp
EventProperties event2("MyProduct.TaggedEvent2",
    {
        { "result", "Success" },
        { "random", rand() },
        { "secret", 5.6872 },
        { "seq", (uint64_t)i },
        { COMMONFIELDS_EVENT_PRIVTAGS, PDT_BrowsingHistory }
    });
logger->LogEvent(event2);
```

Here is a list of the privacy flags available on UTC default behavior:

```cpp
PDT_BrowsingHistory                     0x0000000000000002u
PDT_DeviceConnectivityAndConfiguration  0x0000000000000800u
PDT_InkingTypingAndSpeechUtterance      0x0000000000020000u
PDT_ProductAndServicePerformance        0x0000000001000000u
PDT_ProductAndServiceUsage              0x0000000002000000u
PDT_SoftwareSetupAndInventory           0x0000000080000000u
```

The tag set on your event will show it the field `EventInfo.PrivTags`. You can validate that using [Telemetry Real Time Tool (TRTT)](https://osgwiki.com/wiki/Telemetry_Real-Time_Tool_(TRTT))

![UTC Privacy Tags example](/docs/images/14154-utc.png)

# Diagnostic Level

The C++ SDK has an API feature to filter events using the diagnostic level associated with it. The current list of supported Diagnostic level are:

```cpp
DIAG_LEVEL_REQUIRED                                 1
DIAG_LEVEL_OPTIONAL                                 2
DIAG_LEVEL_REQUIREDSERVICEDATA                      110
DIAG_LEVEL_REQUIREDSERVICEDATAFORESSENTIALSERVICES  120
```

The level set on your event will show up in the field `EventInfo.Level`.

There are different ways you can make your diagnostic levels filtering work:

You can set a filter for the default LogManager in your application using the _SetLevel()_ API to allow events to be sent.
An event inherits the Logger level when sent. If you set the **COMMONFIELDS_EVENT_LEVEL** property for your event this will override the default level.
When no level is specified neither at event nor logger, the LogManager level is used for filtering.

Here's an example on how to achieve Diagnostic Level filtering:

```cpp

auto& config = LogManager::GetLogConfiguration();
//Setup your custom config
//...

// Default diagnostic level for this Logger
auto logger0 = LogManager::Initialize(TENANT_TOKEN, config);

// Inherit diagnostic level from parent (LogManager level)
auto logger1 = LogManager::GetLogger();

// Set diagnostic level to OPTIONAL
auto logger2 = LogManager::GetLogger("my_optional_source");
logger3->SetLevel(DIAG_LEVEL_OPTIONAL);

// A set that specifies that nothing passes through level filter
std::set<uint8_t> logNone  = { DIAG_LEVEL_NONE };
// Everything goes through
std::set<uint8_t> logAll   = { };
// Only allow BASIC level filtering
std::set<uint8_t> logRequired = { DIAG_LEVEL_REQUIRED };

auto filters = { logNone, logAll, logRequired };

// Example of how level filtering works
size_t i = 0;
// For each filter defined
for (auto filter : filters)
{
 // Specify diagnostic level filter for the default LogManager
 LogManager::SetLevelFilter(DIAG_LEVEL_DEFAULT, filter);
 // For every logger
 for (auto logger : { logger0, logger1, logger2, logger3 })
 {
  // Create an event without diagnostic level
  EventProperties defLevelEvent("My.DefaultLevelEvent");
  // Behavior is inherited from the current logger
  logger->LogEvent(defLevelEvent);

  // Create an event and set level to REQUIRED
  // This overrides the logger level for filtering
  EventProperties requiredEvent("My.RequiredEvent");
  requiredEvent.SetLevel(DIAG_LEVEL_REQUIRED);
  logger->LogEvent(requiredEvent);

  // Create an event and set level to OPTIONAL
  // This overrides the logger level for filtering
  EventProperties optionalEvent("My.OptionalEvent");
  optionalEvent.SetLevel(DIAG_LEVEL_OPTIONAL);
  logger->LogEvent(optionalEvent);
 }
}

LogManager::FlushAndTeardown();

```
