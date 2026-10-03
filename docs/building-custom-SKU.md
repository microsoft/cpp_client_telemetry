# Building with Custom build options on Windows

SDK customers may build their own custom SKU tuned to their liking, tailoring SDK for a specific need. Sometimes it is important to optimize the SDK for size, turning non-essential features off. Some customers would like to build SDK with UTC channel only and exclude the rest of unwanted features, such as Offlien Storage. While other customers would like to build SKU without UTC channel. We give our SDK customers the choice of building custom SKU based on their own build recipe, with just the subset of features our specific customer needs.

## SDK build options

Developers may supply a custom SDK header that descibes the set of feature flags:

```cpp
#define CONFIG_CUSTOM_H  "config-custom.h"
```

This option could be defined at top-level script in your build system. For MSBuild projects - use _ForceImportBeforeCppTargets_ to add this preprocessor definition to your build. [Please refer to MSDN documentation to learn more about this option.](
https://docs.microsoft.com/en-us/cpp/ide/working-with-project-properties?view=vs-2017)

Build recipe must contain the following preprocessor definitions:

| #define   | Default value | Description |
|-----------|---------|----------|
| STATS_TOKEN_PROD |    $token    | Default token for internal SDK usage stats 'evt_stats' in PROD |
| STATS_TOKEN_INT | $token | Default token for internal SDK usage stats 'evt_stats' in INT (sandbox) environment |
| HAVE_MAT_EXP | off | Enable built-in A/B config and Experimentation Client for ECS and AFD |
| HAVE_MAT_AI | on | Enable Azure Monitor / Application Insights telemetry channel |
| HAVE_MAT_UTC | on | Enable UTC telemetry channel (available on Windows 10 RS2+ only) |
| HAVE_MAT_JSONHPP | on | Build with [JSON for Modern C++ library](https://github.com/nlohmann/json) |
| HAVE_MAT_ZLIB | on | Use zlib for HTTP requests compression. This option must always be turned on for any high-volume telemetry project |
| HAVE_MAT_LOGGING | on | Enable internal SDK tracing / debug logging |
| HAVE_MAT_WIN_LOG | off | Will log statements to disk on windows if trace enabled and HAVE_MAT_LOGGING defined |
| HAVE_MAT_EVT_TRACEID  | off | Enable event tracking by adding trace-id to http request header on Windows. This is for debugging purpose, and not recommended to be enabled in production. The collector doesn't parse/read this header. As of now, this is meant to be used through the capi, where the http-send handler should remove this header from the event data before sending it to collector. |
| HAVE_MAT_STORAGE | on | Enable SQLite persistent offline storage |
| HAVE_MAT_NETDETECT | on | _Win32 Desktop only_: Use IP Helper connectivity hints on Windows 10 version 2004+; use native Network List Manager COM APIs on older supported Windows versions |
| HAVE_MAT_SHORT_NS | off | Use short "MAT::" namespace instead of "Microsoft::Applications::Events::" to reduce the .DLL size |
| HAVE_CS4 | off | Build with Common Schema 4.0 support. Current default is `off`, i.e. building with Common Schema 3.0 support |
| HAVE_CS4_FULL | off | Enable additional Common Schema 4.0 protocol features needed by server / services SDK |
| COMPACT_SDK | off | Built-in build recipe for smallest possible SDK. Turns most features off. Includes_mat/config-compact.h_ |

### Windows desktop network cost lifecycle

Build with a Windows SDK that declares `NL_NETWORK_CONNECTIVITY_HINT` in `nldef.h`.

The IP Helper APIs are resolved at runtime, preserving the existing Windows 10
and Windows Server 2016 minimum rather than adding newer loader imports. The
modern backend reports aggregate connectivity hints, not just the WinRT Internet
connection profile. Roaming and approaching/exceeded data limits map to the
restrictive `NetworkCost_Roaming` category. Connectivity hints do not expose
WinRT's separate background-data restriction flag.

The fallback activates `INetworkListManager` on a private SDK-owned STA and
preserves the three original event families: network-list connectivity, network
properties, and connection properties. `INetworkCostManager` is queried only
as an optional capability. An unsupported cost interface reports
`NetworkCost_Unknown` while connectivity/property monitoring remains active,
matching the behavior before the WinRT-only detector change. It is not a startup
failure and is not treated as an unmetered connection. No cost-specific event
interface is required.

Base NLM is documented for Windows Vista/Server 2008 onward; cost querying is
documented for Windows 8 clients with no supported Server versions. The fallback
therefore does not require Windows 8 cost support or WinRT on Windows 7 SP1 or
Server 2008 R2. This describes detector API coverage, not a change to the SDK's
overall support policy or compiler/runtime requirements.

This fallback loads `netprofm.dll`; the modern backend does not. Subscription teardown, interface
release, and balanced COM shutdown happen before joining the listener thread.
The host does not need to initialize COM or retain an MTA across SDK DLL reloads.
Callback dispatch is drained on external stop; a reentrant stop does not wait on
itself. Restart and subsequent external stops still drain the previous callback.
Explicit COM disconnection failures are logged, and the non-agile sink's owning
STA still completes `CoUninitialize`, which closes its RPC connections, before
the thread is joined. This does not terminate the host. Native notification
cancellation failure remains fatal because there is no COM apartment rundown
to provide that safety guarantee.

Consumers embedding the SDK in an unloadable library can avoid both network
backends by setting `CFG_BOOL_ENABLE_NET_DETECT` to `false` in the
`ILogConfiguration` passed to SDK initialization. The desktop implementation
does not construct or start a detector in that configuration. This preserves
the existing disabled-detection behavior (unmetered cost), rather than the
unknown cost returned by an enabled detector without cost information.
The setting avoids detector-originated COM/NLM activity; it is not a guarantee
that every SDK component or host dependency is leak-free.

For build-time exclusion, omit `HAVE_MAT_NETDETECT` from a custom SDK recipe.
Defining it as `0` does not disable the feature because it uses presence-based
preprocessor checks.

## Building custom SDK SKU: MSBuild example

Command:

```console
build-all-windows.bat %CD%\Solutions\build.compact.props
```

produces a custom compact SDK build.

The argument passed to `build-all-windows.bat` must be an MSBuild `.props` or `.targets` file that sets the required preprocessor definitions. Do not pass the `config-*.h` header directly to `ForceImportBeforeCppTargets`. `build-all.bat` remains as a compatibility wrapper for existing automation.

How it works:

**build.compact.props** - contains the preprocessor definition that is functionally equivalent to

```cpp
#define CONFIG_CUSTOM_H "config-compact.h"**
```

MSBuild definition here:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemDefinitionGroup>
    <ClCompile>
      <PreprocessorDefinitions>%(PreprocessorDefinitions);CONFIG_CUSTOM_H="config-compact.h"</PreprocessorDefinitions>
    </ClCompile>
  </ItemDefinitionGroup>
</Project>
```

That file is included from **mat/config.h** and propagates the necessary configuration flags to all SDK modules.

## Building custom SDK SKU in Visual Studio

In order to test your custom build configuration in Visual Studio IDE:

- Set `CUSTOM_PROPS_VS` environment variable in cmd.exe

For example:

```console
set "CUSTOM_PROPS_VS=%~dp0\Solutions\build.compact-min.props"
```

- Launch Visual Studio IDE (devenv.exe) from that same shell

```console
tools\start-ide.cmd
```

- Perform selective **Batch build...** of applicable projects. Some projects in solution may not be custom build-friendly. Build only the projects you need.
