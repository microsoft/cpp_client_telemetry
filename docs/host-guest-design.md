# 1DS C/C++ Host-Guest API detailed design

## Preface

There are scenarios where hybrid applications (C#, C/C++, JavaScript) may need to propagate telemetry
to Common Shared Telemetry library (1DS).

In those scenarios all parts of application need to discover the SDK instance. For example:

- App Core C++ SDK initializes Telemetry Stack as Telemetry `Host`, letting other delay-loaded components
of application to reuse its telemetry stack.
- App Core C++ SDK could use `LogManager::SetContext(...)` and Semantic Context C++ APIs to populate some
common low-level knowledge, accessible only from C++ layer, ex. `ext.user.localId`.
- Extension SDKs (Plugins) could act as Telemetry "Guest". These would discover the existing telemetry
stack and could use `evt_set_logmanager_context` C API to append their context variables to shared
telemetry context.
- App Common C# layer could also act as Telemetry "Guest". It latches itself to the existing instance
of native telemetry stack, appending its own variables available from application store, ex. `ext.app.name`,
`ext.app.id`. Since packaged application also knows from its package what platform it is designed for,
it could populate `ext.os.name`. For example, set it to `Meta Quest 2` / `Meta Quest Pro` intead of
generic `Android` moniker. Presently 1DS C/C++ SDK itself cannot auto-discover those intricacies.
- App layer could also propagate additional compile-time constants, such as `app.build.env`, git tag
of app, etc.

Consolidated Shared Telemetry Context contains a set of fields populated by various elements of
a system stack. From top C# / JS layers to the bottom C++ layer, spanning across extension plugins /
libraries loaded in app context. Those libraries could consume telemetry stack via C API.

Shared context properties could get stamped on all events emitted by a product, irrespective of whether
these events originated from a high-level app written in C#, or a lower-level extension SDK written in C.

## Ultimate user guide to 1DS C/C++ SDK Host-Guest API

`Host`-`Guest` API has been designed for the following scenarios:

### Sharing telemetry stack

Main component - `Host` loads its accessory component (or SDK) - `Guest`. Both components use the
same shared dynamically loadable 1DS C++ SDK binary, e.g. `ClientTelemetry.dll`, `libmaesdk.so`, or
`libcat.so` - whatever is the "distro" used to package 1DS C++ SDK.

`Guest` could dynamically discover and load 1DS C++ via C API. It could latch to currently initialized
instance of its `Host` component `LogManager`. `Guest` could also create its own totally separate sandboxed
instance of a `Guest` `LogManager`. `GetProcAddress` is supported on Windows. `dlsym` supported on Linux
and Android. Lazy-binding (automagic binding / auto-discovery of `Host` telemetry stack) is supported
on Linux, Mac and Android. `P/Invoke` for C# is also fully supported cross-platform for .NET and Mono.
1DS C API provides one unified struct layout, with packed structs approach that works on modern
Intel-x64 and ARM64 OS. One single C# assembly could interoperate with 1DS C++ SDK in a uniform way.

### SDK-in-SDK scenario

Native code SDK could load another extension/accessory SDK. Both parts must share the same telemetry stack.
Extension SDK could be written in C or C++. Main SDK is treated as `Host`, additional SDKs are treated
as `Guests`. `Host` could also facilitate the ingection of Diagnostic Data Viewer plugin, in order to
satisfy our Privacy and Compliance oblogations. Additional `Guest` modules could enrich the main `Host`
shared telemetry context with their properties.

### Telemetry flows and `Telemetry Data Isolation` scenarios

In some cases many different application modules (plugins) get loaded into the main app address space.
For example, `Azure Calling SDK` or `Microsoft Information Protection SDK` running in another product.
These plugin(s) may not necessarily need to share their telemetry flows with the main app. In that case
the modules must operate within their own trust and data collection boundary. Data uploads need to be
controlled separately, with Required Service Data flowing to Azure location of a service resource;
while Optional Customer Data may need to flow to its own regional EUDB-compliant collector.

`Host`-`Guest` API solves this challenge by providing partitioning for different components using the
same telemetry SDK. If necessary, different modules telemetry collection processes run totally isolated
from one another.

## Common Considerations

`HostGuestTests.cpp` module in functional test contains several usage examples.

See detailed explanation of configuration options and examples below.

### Dissecting Host configuration

`Host` configuration example:

```json
{
 "cacheFilePath": "MyOfflineStorage.db",
 "config": {
  "host": "C-API-Host",
  "scope": "*"
 },
 "stats": {
  "interval": 0
 },
 "name": "C-API-Host",
 "version": "1.0.0",
 "primaryToken": "ffffffffffffffffffffffffffffffff-ffffffff-ffff-ffff-ffff-ffffffffffff-0001"
 "maxTeardownUploadTimeInSec": 5,
 "hostMode": true,
 "minimumTraceLevel": 0,
 "sdkmode": 0
}
```

`Host` could specify the two matching parameters:

- `"host": "C-API-Host"`
- `"name": "C-API-Host"`

If host parameter matches the name parameter, then it assumed that the `Host` module acts as the one and
only `Host` in the application. It will be creating its own data collection sandbox. It will not latch to
any other `Host` modules that could be running in the same app. Multiple `Host` modules supported.

In some scenarios a `Host` would prefer to latch (join) an existing telemetry session. This is especially
helpful if multiple Hosts need to share one data collection domain and their startup/load order is not
clearly defined. In that case, a session initialized by first `Host` could be shared with other Hosts.
Data collection domain performs ref-counting of instances latched to it.

Hosts could specify `"host": "*"` to attach to existing data collection session. If first `Host` leaves
(unloads or closes its handle), remaining entities in that session continue operating until the last
`Host` leaves the data collection domain.

Both Guests and Hosts may utilize the `scope` parameter that controls if these would be sharing the
same common telemetry context shared within a sandbox:

- `scope="*"` - SHARED or ALL, means that a component will contribute its context to shared context.
- `scope="-"` - NONE or RESTRICTED, means that a component will not contribute its context, and will
not receive any values from the shared context. This mechanism allows to satisfy data collection
and privacy obligations. Each entity acts within their own data collection and compliance boundary
without sharing any of their telemetry contexts with other modules in the process.

## Dissecting Guest configuration

Guest configuration example:

```json
{
 "cacheFilePath": "MyOfflineStorage.db",
 "config": {
  "host": "*",
  "scope": "*"
 },
 "stats": {
  "interval": 0
 },
 "name": "C-API-Guest",
 "version": "1.0.0",
 "primaryToken": "ffffffffffffffffffffffffffffffff-ffffffff-ffff-ffff-ffff-ffffffffffff-0002",
 "maxTeardownUploadTimeInSec": 5,
 "hostMode": false,
 "minimumTraceLevel": 0,
 "sdkmode": 0
}
```

Guest entity:

- specifies its own data storage file. This is helpful if Guest starts up prior to any other `Host`.
- `"host": "*"` parameter allows the Guest to latch to any host.
- `"scope": "*"` parameter allows the Guest to contribute and share its telemetry context with other modules (`Host` and Guests).
- Hosts and Guests to present themselves with unique name, ex. `"name": "C-API-Guest"` and unique version, ex. `1.0.0`.
- Guest must specify `"hostMode": false`. That is how SDK knows that a Guest is expected to join another `Host`'s sandbox.
- Guest may omit the scope parameter. In this case the Guest cannot capture the main `Host` telemetry contexts.
This is done intentionally as a security feature. Main application developers may ask their plugin developers
to never capture any telemetry contexts populated by the main application. For example, in some cases - main
application `ext.user.localId` or session `TraceId` cannot be shared with extension. There is no explicit
permission model. Since most components are expected to be assembled and tested by product development teams,
the team should audit the usage of Guest scope parameter by the plugins it is loading. There is runtime code
isolation provided by this mechanism. It is based on trust that all loadable modules exercise their due
diligence while setting up their telemetry configuration.

### End-to-end  example

`Host` code:

```cpp
    // Host JSON configuration:
    const char* hostConfig = JSON_CONFIG(
        {
            "cacheFilePath" : "/some/path/MyOfflineStorage.db",
            "config" : {
                "host" : "C-API-Host",
                "scope" : "*"
            },
            "name" : "C-API-Host",
            "version" : "1.0.0",
            "primaryToken" : "ffffffffffffffffffffffffffffffff-ffffffff-ffff-ffff-ffff-ffffffffffff-0001",
            "hostMode" : true
        });

    // Host initializes in Host mode, waiting for Guest()s to register.
    evt_handle_t hostHandle = evt_open(hostConfig);

    // evt_prop[] array that contains common context properties.
    // Contexts between Hosts and Guests could be merged into one shared context.
    evt_prop hostContext[] = TELEMETRY_EVENT(
        _STR("ext.device.localId", "a:4318b22fbc11ca8f"),
        _STR("ext.device.make", "Microsoft"),
        _STR("ext.device.model", "Clippy"),
        _STR("ext.os.name", "MS-DOS"),
        _STR("ext.os.ver", "2100")
    );

    // Host appends common context properties at top-level LogManager.
    // These variables will be shared with Guest(s).
    evt_set_logmanager_context(hostHandle, hostContext);

    evt_prop hostEvent[] = TELEMETRY_EVENT(
        // Part A/B fields
        _STR(COMMONFIELDS_EVENT_NAME, "Event.Host"),
        _INT(COMMONFIELDS_EVENT_PRIORITY, static_cast<int64_t>(EventPriority_Immediate)),
        _INT(COMMONFIELDS_EVENT_LATENCY, static_cast<int64_t>(EventLatency_Max)),
        _INT(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_REQUIRED),
        _STR("strKey", "value1"),
        _INT("intKey", 12345),
        _DBL("dblKey", 3.14),
        _BOOL("boolKey", true),
        _GUID("guidKey", "{01020304-0506-0708-090a-0b0c0d0e0f00}");
    evt_log(hostHandle, hostEvent);
 
```

In above example:

- `Host` performs initialization.
- populates its top-level LogManager semantic context with known values.

For example, the `Host` C++ layer could use native API to access the lower-level platform-specific
Device Id, Device Make, Model. `Host` may emit a telemetry event that would combine the event data
with its context data.

Guest code:

```cpp

    // Guest JSON configuration:
    const char* guestConfig = JSON_CONFIG(
        {
            "config" : {
                "host" : "*",
                "scope" : "*"
            },
            "name" : "C-API-Guest",
            "version" : "1.0.0",
            "primaryToken" : "ffffffffffffffffffffffffffffffff-ffffffff-ffff-ffff-ffff-ffffffffffff-0002",
            "hostMode" : false
        });

    // Guest initializes in Guest mode and latches to previously running Host.
    auto guestHandle = evt_open(guestConfig);

    // evt_prop[] array that contains context properties:
    evt_prop guestContext[] = TELEMETRY_EVENT(
        _STR("ext.app.id", "com.Microsoft.Clippy"),
        _STR("ext.app.ver", "1.0.0"),
        _STR("ext.app.locale", "en-US"),
        _STR("ext.net.cost", "Unmetered"),
        _STR("ext.net.type", "QuantumLeap"));

    // Guest could append some of its common context properties on top of shared context:
    evt_set_logmanager_context(guestHandle, guestContext);

    evt_prop guestEvent[] = TELEMETRY_EVENT(
        _STR("name", "Event.Guest"),
        _INT(COMMONFIELDS_EVENT_PRIORITY, static_cast<int64_t>(EventPriority_Immediate)),
        _INT(COMMONFIELDS_EVENT_LATENCY, static_cast<int64_t>(EventLatency_Max)),
        _INT(COMMONFIELDS_EVENT_LEVEL, DIAG_LEVEL_REQUIRED),
        _STR("strKey", "value2"),
        _INT("intKey", 67890),
        _DBL("dblKey", 3.14),
        _BOOL("boolKey", false),
        _GUID("guidKey", "{01020304-0506-0708-090a-0b0c0d0e0f01}");
    evt_log(guestHandle, guestEvent);
 
```

In above example:

- Guest registers and shares the scope with the `Host`.
- Guest entity could operate on a totally different abstraction layer, e.g. higher-level Unity C# or Android Java app.
It could obtain certain system parameters that are easily accessible only by the higher-level app. Such as, app store
application name and version. It could be a layer that performs User Authentication and Authorization, subsequently
sharing the User Identity as part of common telemetry context shared with lower-level code across the language boundary.

Reference design showing how to use 1DS C API from .NET Core, Mono and Unity applications is provided.

Above examples generate the following event payloads.

`Host` Event payload in Common Schema notation:

```json
{
 "data": {
  "boolKey": true,
  "dblKey": 3.14,
  "guidKey": [[4,3,2,1,6,5,8,7,9,10,11,12,13,14,15,0]],
  "intKey": 12345,
  "strKey": "value1"
 },
 "ext": {
  "device": {
   "localId": "a:4318b22fbc11ca8f",
   "make": "Microsoft",
   "model": "Clippy"
  },
  "os": {
   "name": "MS-DOS",
   "ver": "2100"
  }
 },
 "iKey": "o:7c8b1796cbc44bd5a03803c01c2b9d61",
 "name": "Event.Host",
 "time": 1680074712000,
 "ver": "3.0"
}
```

Guest Event payload in Common Schema notation. Note that Guest event emitted after `Host` initialization
contains the superset of all consolidated common properties:

```json
{
 "data": {
  "boolKey": true,
  "dblKey": 3.14,
  "guidKey": [[4,3,2,1,6,5,8,7,9,10,11,12,13,14,15,0]],
  "intKey": 12345,
  "strKey": "value1"
 },
 "ext": {
  "app": {
   "id": "com.Microsoft.Clippy",
   "locale": "en-US",
   "name": "com.Microsoft.Clippy",
   "ver": "1.0.0"
  },
  "device": {
   "localId": "a:4318b22fbc11ca8f",
   "make": "Microsoft",
   "model": "Clippy"
  },
  "net": {
   "cost": "Unmetered",
   "provider": "",
   "type": "QuantumLeap"
  },
  "os": {
   "name": "MS-DOS",
   "ver": "2100"
  }
 },
 "iKey": "o:7c8b1796cbc44bd5a03803c01c2b9d61",
 "name": "Event.Guest",
 "time": 1680074712000,
 "ver": "3.0"
}
```

`Host`-`Guest` approach allows us to share one common telemetry diagnostic context across the language
boundaries in a hybrid application designed with different programming languages: C/C++, C#, and
JavaScript. Other programming languages may easily leverage Foreign Function Interface and 1DS C API.

`Host`-`Guest` interface plays a central role in aggregation of different module contexts into one
common shared telemetry context of application. C++ example is available in `SampleCppLogManagers`
project.
