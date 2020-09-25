# 1DS Test Server

Test server requires ASP.NET Core 3.x. Uses [Kestrel](https://docs.microsoft.com/en-us/aspnet/core/fundamentals/servers/kestrel?view=aspnetcore-3.1) as default server.

This test server is provided as a reference, for test purposes only, not supported, and not intended to be used by production services.

## Usage

`run.cmd` - on Windows
`run.sh`  - on Linux and Mac

Or launch it directly from Visual Studio solution (tools\tools.sln).

## Directing client SDK to test server

```cpp
    auto& config = LogManager::GetLogConfiguration();
    config[CFG_STR_COLLECTOR_URL] = "http://localhost:5000/OneCollector/";
```

Emit some events. Observe JSON-decoded events in test server console.
