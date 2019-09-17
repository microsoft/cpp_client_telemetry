# Supported Platforms
The SDK definines 'platforms' as the union of Operating System, Compiler, and Standard Library.

## Compilers
The SDK requires a code base that at least supports C++11 and our code is C++11 compliant. We guarantee the SDK will compile under any of the following compilation flags:

Linux:
* gcc, clang: -std=c++11
* gcc, clang: -std=c++14

macOS:
* gcc, clang: -std=c++11
* gcc, clang: -std=c++14

Windows:
* msvc: /std:c++11
* msvc: /std:c++14

## Full Support
Platforms in Full Support will be validated as part of the Check-In process when Pull Requests are merged into master.

| Operating System       | Compiler Toolchain   | Standard Library | Notes |
|------------------------|----------------------|------------------|-------|
| Linux                  | GCC 4.8+             | libstdc++        |       |
| Linux                  | GCC 4.8+             | libc++           |       |
| Linux                  | Clang 3.3+           | libc++           |       |
| Linux                  | Clang 3.3+           | libstdc++        |       |
| macOS 10.10+           | Xcode 10.0.2+        | libc++           |       |
| Windows 7+ (Desktop)   | MSVC 2015+           | MSVC             |       |
| Windows 7+ (Desktop)   | Clang 3.3+           | MSVC?            |       |
| Windows 10 (Universal) | MSVC 2015+           | MSVC             |   1   |

1. Requires Windows SDK version 10.0.17134.0 for Microsoft Store distribution.

## Best Effort Support
Platforms in Best Effort Support will not be validated as part of the Check-In process when Pull Requests are merged into master. However, fixes will be accepted into master to address any breaks.

| Operating System       | Compiler Toolchain   | Standard Library | Notes |
|------------------------|----------------------|------------------|-------|
| Android NDK r16+       | GCC 4.8+             | libc++           |   1   |
| Android NDK r16+       | Clang 3.3+           | libc++           |   1   |
| iOS 12.2+              | Xcode 10.0.2+        | libc++           |       |

1. The Android NDK has deprecated support for libstdc++ as of r16, further the SDK depends on functions (std::to_string methods, as well as the inverse methods) that are not supported by the version of libstdc++ shipped in the NDK.

## Unsupported
Everything else. Fixes will not be accepted into master to address any breaks, an Issue must be created and agreed on by the community to add the new Platform to Best Effort or Full Support.
