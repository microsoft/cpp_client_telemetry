# C++, C++/CX, C# and Desktop client SDK Release Notes

### Supported Platforms

- Windows Desktop for C/C++ (x86/x64/ARM64)
- Windows 10 Universal (x86/x64/ARM/ARM64)
- Windows Desktop for .NET 4.x (x86/x64)
- Linux x86/x64/ARM/ARM64 (gcc-5+)
- Mac OS X (experimental, source only)
- MinGW (source only)


## Version 3.0.268.1 (09/25/2018)

### New features

1432826 arm64 SKU

### Design Changes

1438210	Support LoadTransmitProfiles for C# apps
1438212	Full- vs Basic- Telemetry: allow to set transmit profile at SDK start

### Bug Fixes

1436626 Spurious wakes-up (OTEL power scorecard regression after upgrade from v1.7 to v3)
1403099	Fix 1DS collector URLs in ILogConfiguration.hpp
1438211	Fix 1DS collector URLs in C# projection code
1403101	LogConfigurationCX.cpp needs to create a string copy

---

## Version 3.0.261.1 (09/18/2018)

### New features

None

### Design Changes

- LogEvent performance improvements
- Offline storage write performance improvements
- Enable .rpm packaging for RHEL and CentOS
- Turn off whole program opt, LTCG and JustMyCode to enable linking with older vs2017

### Bug Fixes

- Fix for ClientTelemetry.dll not loading on Windows 7.1A
- Compiler warnings clean-up
- FlushAndTeardown: skipped uploading some records on shutdown
- FlushAndTeardown: may unnecessarily wait for up to CFG_INT_MAX_TEARDOWN_TIME
- FlushAndTeardown: possible event duplication of some events sent immediately before shutdown

---

## Version 3.0.248.1 (09/05/2018)

### New features

- Fast ram queue

### Design Changes

- Use std::vector instead of sqlite3 DB for ram queue

### Bug Fixes

- Use c: prefix for DeviceInfo.Id field
- FlushAndTeardown respects time set in config CFG_INT_MAX_TEARDOWN_TIME
- FlushAndTeardown respects paused state
- Various memory leak fixes: HTTP stack, event queue
- Invalid storage path handled safely
