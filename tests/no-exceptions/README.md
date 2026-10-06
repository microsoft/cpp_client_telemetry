# No-exceptions runtime regression

This dependency-free smoke executable links the full SDK and compiles both the
SDK and driver with C++ exceptions disabled. It checks malformed configuration
and response JSON, configuration and timer numeric limits, partial profile
loading, correlation-vector overflow, unrepresentable expansion sizes, and
deterministic allocation failure for a representable expansion.
It requires neither Google Test nor the private optional modules.

For GCC, Clang, Apple Clang, MSVC, or clang-cl:

```sh
cmake -S tests/no-exceptions -B out-no-exceptions \
  -DCMAKE_BUILD_TYPE=Debug -DMATSDK_DISABLE_EXCEPTIONS=ON
cmake --build out-no-exceptions --target no-exceptions-smoke --parallel 2
ctest --test-dir out-no-exceptions --output-on-failure
```

For clang-cl, select the `ClangCL` toolset.
Use a fresh build directory when changing compilers or exception modes.
Expansion requests larger than `PTRDIFF_MAX` fail before calling an allocator;
representable allocations still use `std::nothrow` and report failure normally.
The internal expansion overload accepts a per-call allocator. Regression tests
return null for a 32-byte request and verify the allocator was called, a
previously non-null output was cleared, and the output length became zero.
Production calls retain the normal `std::nothrow` allocator; there is no global
failure-injection state.

`FromJSON` preserves throwing syntax errors in exception-enabled builds. When
exceptions are disabled it logs malformed syntax and returns an empty
configuration. Null input and a non-object root log an error and return empty.

`TransmitProfiles::load(string)` retains its historical partial-load contract:
a schema error stops parsing, replaces existing custom profiles with the valid
prefix, and returns true if that prefix is nonempty. Default profiles remain
available. Entries after the first invalid profile are not loaded. The vector
overload validates every candidate before replacing existing custom profiles.
