# Open Source components

These are the Open Source components used by Microsoft 1DS / C++ Client Telemetry SDK.

## [ZLib](https://github.com/madler/zlib)

ZLIB DATA COMPRESSION LIBRARY.

SDK maintains its own snapshot of the mainline ZLib with some Intel architecture performance optimizations [here](../zlib).

## [SQLite](https://www.sqlite.org/index.html)

SQLite is a C-language library that implements a small, fast, self-contained, high-reliability, full-featured, SQL database engine.

SDK maintains its own snapshot of the mainline SQLite, which is used for Windows builds [here](../sqlite). Other platforms use platform-provided SQLite.

## [nlohmann/json](https://github.com/nlohmann/json)

JSON for Modern C++.

SDK maintains its own snapshot of the mainline `nlohmann/json` header-only library [here](../lib/include/mat/json.hpp).

## [libcurl](https://curl.haxx.se/libcurl/)

libcurl - the multiprotocol file transfer library.

SDK uses `libcurl` provided as part of OS package on Linux, Mac, iOS; and as part of OS build tree on Android.

## [Google Test](https://github.com/google/googletest)

Google's C++ test framework. Used only for tests and not included in products.

## [Google Benchmark](https://github.com/google/benchmark)

Google's C++ benchmarking framework. Used only for tests and not included in products.

## [Tony Million Reachability Framework](https://github.com/tonymillion/Reachability)

Reachability is a drop-in replacement for Apple's Reachability class. It is ARC-compatible, and it uses the new GCD methods to notify of network interface changes.
SDK maintains its own snapshot of the mainline `tonymillion/Reachability` [here](../third_party/Reachability). This code is not used nor included in the build of non-Apple OS.

## SHA-1 by Steve Reid

Classic implementation of SHA-1 (Public Domain).
SDK maintains its own snapshot of it [here](../third_party/sha1/sha1.c).
Note that this component is not included or compiled into any of the shipable bits of SDK. It is included for internal developer debug builds only.
For example, SHA-1 may be used to calculate destination ETW Provider GUID based on ETW Provider name on Windows OS in developer trace tooling / instrumentation.

## Other components and systems

Other Open Source components that may be indirectly included by Open Source build tooling, instrumentation and other SDKs (e.g. Apple SDK or Android SDK) used for a corresponding platform:

- CMake
- Gradle
- Android NDK
- OpenSSL

SDK does not compile these components from source. It uses the mainstream prebuilt packages as  provided by corresponding development platform SDKs.

Please refer to original development platform SDKs for the original source code packages:

- [Apple SDK](https://developer.apple.com/)
- [Android SDK and NDK](https://developer.android.com/studio)

Please reach out to [1ds.sdk.cpp DL](mailto:1ds.sdk.cpp@service.microsoft.com) if you identified OSS component used by SDK that has not been listed.
It will be added to this list.
