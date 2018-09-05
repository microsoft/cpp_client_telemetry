# C++, C++/CX, C# and Desktop client SDK Release Notes

### Supported Platforms

- Windows Desktop for C/C++ (x86/x64)
- Windows 10 Universal (x86/x64/ARM)
- Windows Desktop for .NET 4.x (x86/x64)
- Linux x86/x64/ARM (gcc-5+)
- Mac OS X (experimental, source only)
- MinGW (source only)

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
