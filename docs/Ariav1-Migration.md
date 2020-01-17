---
layout: page
title: C++ Migration
---

## Prerequisites

Aria SDK headers and library file added to library path and list of libraries used. Library file name to be added to the list of project libraries:

|SDK Flavor|Library file name|
|----------|-----------------|
|Win32 Desktop (v1)|ClientTelemetry.lib|
|Win32 Desktop (v3)|ClientTelemetry.lib|
|Win 10 Universal - C++ native code (v1)|Microsoft.Applications.Telemetry.Windows.lib|
|Win 10 Universal - C# and other managed (v1)|Microsoft.Applications.Telemetry.Windows.lib|
|Win 10 Universal - C++ native code (v3)|Microsoft.Applications.Telemetry.Windows.native.lib|
|Win 10 Universal - C# and other managed (v3)|Microsoft.Applications.Telemetry.Windows.managed.dll|

## Source code

### Aria C++ SDK v1.7+

```
// include C++ STL libraries 
#include "LogManager.hpp"

using namespace Microsoft::Applications::Telemetry;

#define TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx-xxxx"
static    LogConfiguration configuration;

int main(int argc, char *argv[])
{
    configuration.maxTeardownUploadTimeInSec = 1;
    configuration.cacheFilePath = "STORAGE_FILENAME";
    ILogger *logger = LogManager::Initialize(TOKEN, configuration);
    logger->LogEvent("simple_event");
    LogManager::FlushAndTeardown();
    return 0;
}
```

### 1DS C++ SDK v3.x

```
// include C++ STL libraries
#include "LogManager.hpp"

// CHANGE from v1: new namespace alias in v3 is MAT = Microsoft::Applications::Events
using namespace MAT;

// NEW: v3 allows several .DLL / .SO dynamic library modules have their own Aria singleton instance
LOGMANAGER_INSTANCE

#define TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx-xxxx"
int main(int argc, char *argv[])
{
    // Configuration is no longer a static object because different instances may have each their own configuration.
    // In advanced scenarios it is also possible for several modules to ‘share’ one configuration instance.
    auto& config = LogManager::GetLogConfiguration();
    config["name"] = "HelloAria";
    config["version"] = "1.2.5";
    config["config"]["host"] = "HelloAria";
    config[CFG_STR_CACHE_FILE_PATH]   = " STORAGE_FILENAME";
    config[CFG_INT_MAX_TEARDOWN_TIME] = 1; // time to spend on HTTP upload during SDK teardown
    ILogger *logger = LogManager::Initialize(TOKEN);

    // The rest of API surface is largely the same as v1
    logger->LogEvent("simple_event");
    LogManager::FlushAndTeardown();
    return 0;
}

```
