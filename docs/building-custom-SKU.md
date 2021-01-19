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
| HAVE_MAT_STORAGE | on | Enable SQLite persistent offline storage |
| HAVE_MAT_NETDETECT | on | _Win32 Desktop only_: Use NLM COM object for network cost detection on Windows 8+ |
| HAVE_MAT_SHORT_NS | off | Use short "MAT::" namespace instead of "Microsoft::Applications::Events::" to reduce the .DLL size |
| HAVE_CS4 | off | Build with Common Schema 4.0 support. Current default is `off`, i.e. building with Common Schema 3.0 support |
| HAVE_CS4_FULL | off | Enable additional Common Schema 4.0 protocol features needed by server / services SDK |
| COMPACT_SDK | off | Built-in build recipe for smallest possible SDK. Turns most features off. Includes_mat/config-compact.h_ |

## Building custom SDK SKU: MSBuild example

Command:

```console
build-all.bat %CD%\Solutions\build.compact.props
```

produces a custom compact SDK build.

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
