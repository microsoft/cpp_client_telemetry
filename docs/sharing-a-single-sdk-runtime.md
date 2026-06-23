# Sharing one SDK runtime across several modules in a process

When more than one module in a single process links this SDK — for example an
application that loads several plug-ins or libraries, each of which uses 1DS —
the easy default (every module statically embeds the SDK) has two costs:

1. **Size.** The SDK (plus its bundled SQLite/zlib) is duplicated once per module.
2. **Duplicated global state.** Each static copy has its *own* default
   `LogManager`, HTTP transport, offline SQLite cache, and upload threads. They do
   not share a pipeline, and multiple writers to the same offline-cache path will
   corrupt it.

This document describes how to ship **one** shared SDK runtime (`mat.dll` /
`libmat.so` / `libmat.dylib`) that every module imports, so there is a single
copy on disk and a single set of process-global state.

There are two ways to consume the shared runtime. **The C API is strongly
recommended** because it removes the fragile C++/CRT ABI coupling between modules.

---

## Option 1 (recommended): consume the stable C API

The SDK ships a flat **C ABI** in [`mat.h`](../lib/include/public/mat.h). Every
`evt_*` entry point (`evt_open`, `evt_log`, `evt_flush`, `evt_upload`,
`evt_pause`, `evt_resume`, `evt_close`, `evt_configure`, …) is a `static inline`
wrapper that marshals its arguments into a POD struct and calls through a single
exported `__cdecl` symbol, `evt_api_call_default`.

Consequences that make this the robust choice:

* **Only one symbol crosses the module boundary, and no C++/STL type does.** The
  request is a plain C struct, so there is *no* requirement that the modules and
  the shared runtime agree on the C++ standard library ABI (`/MD`,
  `_ITERATOR_DEBUG_LEVEL`, MSVC toolset/STL version, libstdc++ vs libc++,
  `_GLIBCXX_USE_CXX11_ABI`, …). A 1DS version bump does not force every module to
  rebuild in lockstep against an identical toolchain.
* **It links without `__declspec(dllimport)`.** A plain C function resolves
  through the shared library's import lib, so the C API works across the boundary
  today.

Each module includes `mat.h`, links the one shared runtime, and uses its own
tenant/source. You still pin the **same SDK version** in every module (so the
request/struct layout matches), but you avoid the C++ ABI lockstep entirely.

## Option 2: consume the C++ API from a shared library

All modules `find_package(MSTelemetry CONFIG REQUIRED)` and link
`MSTelemetry::mat` (resolving to the import lib); none statically embed the SDK.

The C++ public API passes C++ standard-library types (`std::string`, `std::map`,
…) across the module boundary, so **every module and the shared runtime must
share one C++ ABI**. If they do not, you get heap corruption / undefined
behavior. Pin all of the following identically:

| Axis | Requirement |
|------|-------------|
| **CRT linkage (Windows)** | Dynamic CRT (`/MD`, `/MDd` for Debug) everywhere — never `/MT`, and never mix Debug/Release CRT across the boundary. (vcpkg: `VCPKG_CRT_LINKAGE dynamic`.) |
| **STL / iterator debug** | One compiler + STL, one build config. `_ITERATOR_DEBUG_LEVEL` must match (Release `0` vs Debug `2`) — a Release consumer + Debug runtime is a silent layout mismatch. |
| **Toolset** | One MSVC toolset across all binaries (the v14x toolsets share an STL ABI, but don't mix major versions); or one libstdc++/libc++ with the same `_GLIBCXX_USE_CXX11_ABI`. |
| **Language / model** | Same `/std:c++NN`, same `/EHsc` exception model, same architecture, no overridden struct packing. |
| **SDK build options** | Same SDK feature/version selection in every module's manifest — different features mean different headers, hence a different ABI even at the same version. |

Because the C++ ABI must match exactly across separately built and separately
versioned modules, this option is materially more brittle than the C API. Prefer
Option 1 unless you specifically need the C++ surface and control all modules'
toolchains.

---

## Building the shared runtime with vcpkg (per-port linkage)

A common requirement is "share *this* SDK, but keep everything else statically
linked (no DLL forest)". Override the library linkage **per port** in your
triplet so only this SDK goes dynamic:

```cmake
set(VCPKG_CRT_LINKAGE dynamic)
if(PORT STREQUAL "cpp-client-telemetry")
    set(VCPKG_LIBRARY_LINKAGE dynamic)   # mat.dll / libmat.so / libmat.dylib + import lib
else()
    set(VCPKG_LIBRARY_LINKAGE static)
endif()
```

The port honors `VCPKG_LIBRARY_LINKAGE` / `BUILD_SHARED_LIBS` and emits the
shared `mat` plus its import lib and the `MSTelemetry` CMake config package.

### Pin one version across all modules

All modules must compile against identical SDK headers. Pin the same
`cpp-client-telemetry` version and `builtin-baseline` (or a shared version
override) in every module's manifest, and ideally build all artifacts in the same
CI job/container with the same toolchain image. Most ABI drift comes from
separate modules quietly building on different agents.

---

## How the SDK decorates its public symbols

The SDK has no `.def` file; on Windows, exporting/importing is driven entirely by
`MATSDK_LIBABI` in [`ctmacros.hpp`](../lib/include/public/ctmacros.hpp), which the
build ties to the actual linkage:

* **Shared build:** the SDK is compiled with `MATSDK_SHARED_LIB`
  (`__declspec(dllexport)`), and the installed `MSTelemetry::mat` target carries
  an `INTERFACE` definition of `MATSDK_IMPORT_LIB`, so consumers that
  `find_package` + link automatically get `__declspec(dllimport)` — no
  consumer-side configuration required.
* **Static build:** nothing is decorated, so the SDK's public symbols are not
  re-exported by a consumer DLL that absorbs the static lib.

On non-Windows platforms the SDK is built with `-fvisibility=hidden` and the
public API is marked `__attribute__((visibility("default")))`, so only the public
API (including the C API) is exported from the shared object.

---

## Coordinate the single runtime's lifetime

One shared runtime means **one** set of process-global state. Decide ownership:

* **Recommended — single owner.** The top-level module initializes and tears down
  the SDK (`LogManager::Initialize` / `FlushAndTeardown`, or `evt_open` /
  `evt_close`). Other modules obtain loggers (their own tenant/source) but never
  initialize or tear down. This avoids teardown-ordering crashes.
* **Alternative — named instances.** `LogManagerProvider::CreateLogManager(id)`
  gives each module its own instance/tenant/config sharing the one transport; then
  you need a last-one-out teardown refcount and **distinct offline-cache paths**
  (one shared path with multiple writers corrupts it).
* Teardown must happen exactly once, **last**, after every module has stopped
  logging.

---

## Ship exactly one copy on the loader path

Place a single runtime where every module finds it:

* **Windows:** the same directory as the consumers (or side-by-side assembly).
* **Linux:** `RPATH=$ORIGIN` so every module resolves the one copy.
* **macOS:** a stable install name, `@rpath/libmat.dylib`.

Make exactly one package own and ship the SDK runtime; the others declare a
dependency rather than bundling their own. If several packages each ship their
own copy, which one loads is path-order luck — and if their versions differ, you
are back to an ABI mismatch even with "one" DLL.

---

## Validate

* **Dependency present, definitions absent.** `dumpbin /dependents` (Windows),
  `ldd` (Linux), `otool -L` (macOS) on each consumer should show a dependency on
  the one `mat` module; `dumpbin /exports` (or `nm -D`) on a consumer should show
  it imports — not defines — the SDK symbols.
* **One copy at runtime.** Process Explorer / `/proc/<pid>/maps` / `vmmap` should
  map the `mat` module exactly once; there should be one offline-cache file.
* **(C++ option) CRT/STL smoke test.** Have a consumer pass a `std::string` event
  property into the SDK and read it back. A `/MD` vs `/MT` or `_ITERATOR_DEBUG_LEVEL`
  mismatch typically crashes immediately (especially in Debug).
