# Maintainer onboarding

This guide is for engineers who maintain or make substantial changes to the
1DS C/C++ SDK repository. It complements the platform-specific documentation;
it does not replace those documents or the build definitions used by CI.

## Start here

Before changing the SDK:

1. Read the root [README](../README.md) for supported platforms and the public
   support boundary.
2. Read [Coding style](Coding%20style.md) and the documentation for the area
   being changed.
3. Read [Versioning](versioning.md) before making compatibility or release
   decisions.
4. Read [Embedding with CMake](embedding-with-cmake.md) and
   [Building with vcpkg](building-with-vcpkg.md) when changing CMake targets,
   dependencies, exported packages, or feature selection.
5. Read [Sharing a single SDK runtime](sharing-a-single-sdk-runtime.md) before
   changing binary boundaries, symbol visibility, CRT linkage, or the stable C
   API used by SDK-in-SDK consumers.
6. Inspect the relevant workflows under [`.github/workflows`](../.github/workflows).
   They are executable documentation for supported build combinations.

Do not assume that a successful build of one configuration proves that another
configuration works. Platform PALs, HTTP transports, storage implementations,
static/shared linkage, optional modules, and generated configuration headers can
select materially different code.

## Access and repository layout

The public repository contains the core SDK. Some Microsoft-only modules are
maintained separately under `lib/modules`. Clone recursively only when work
requires those modules and your GitHub account has access:

```powershell
git clone --recurse-submodules https://github.com/microsoft/cpp_client_telemetry.git
```

Do not make core behavior accidentally depend on a private module. Builds that
do not fetch `lib/modules` must continue to compile and link.

Important locations:

| Path | Purpose |
| --- | --- |
| `lib/include/public` | Supported public C and C++ headers |
| `lib/api`, `lib/system` | Public API implementation and telemetry pipeline |
| `lib/http` | Platform HTTP transports |
| `lib/offline` | Persistent and in-memory storage |
| `lib/pal` | Platform abstraction layers |
| `lib/modules` | Optional Microsoft-only modules |
| `tests/unittests` | Component-level tests |
| `tests/functests` | Public behavior and pipeline tests |
| `tests/vcpkg` | Installed-package consumer tests |
| `examples` | Sample integrations |
| `.github/workflows` | CI and release automation |

## Development workflow

1. Start from current `main` and create a focused topic branch.
2. Search for existing helpers and platform-specific implementations before
   adding new code.
3. Preserve API and configuration compatibility unless the change explicitly
   requires a documented break.
4. Update tests and directly related documentation with the implementation.
5. Build and test the code paths selected by the change, not merely the easiest
   local configuration.
6. Review the final diff for generated files, accidental dependency changes,
   warning suppressions, disabled tests, credentials, tenant tokens, and local
   paths.
7. Open a PR with the behavior change, compatibility implications, and exact
   validation performed.

Treat a warning suppression, skipped test, conditional source exclusion, or
`continue-on-error` as an unverified safety claim. Check why it exists and
whether the changed code makes that claim invalid. Fix the underlying defect
instead of broadening a suppression.

## Building locally

### CMake presets

List the maintained presets:

```powershell
cmake --list-presets
```

The compatibility wrappers use those presets:

```powershell
# Windows; run from PowerShell with Visual Studio C++ tools installed
.\build-cmake.ps1 -Configuration Debug
.\build-cmake.ps1 -Configuration Release

# Build a shared SDK
.\build-cmake.ps1 -Configuration Release -Shared
```

On Linux, WSL, or macOS:

```bash
./build.sh debug
./build.sh release
```

For a clean source-package consumer check, use the installed-package and
embedding guidance in [Embedding with CMake](embedding-with-cmake.md). CMake and
vcpkg builds exercise packaging behavior that an in-tree Visual Studio solution
build does not.

### Windows solution and test build

The Windows CI path uses `build-tests.cmd`:

```powershell
# Platform, configuration, optional .props/.targets file, transport
.\build-tests.cmd x64 Release "" WinHTTP
.\build-tests.cmd x64 Debug "" WinHTTP

# Exercise the opt-in legacy transport when transport code or selection changes
.\build-tests.cmd x64 Release "" WinInet
```

The default desktop transport is WinHTTP. It supports services and
non-interactive processes; WinInet is retained for consumers that explicitly
need its user-profile proxy or cookie behavior.

### WSL and Linux

From a WSL distribution with the compiler, CMake, and other documented
prerequisites installed:

```bash
./build-tests.sh release
./tests/vcpkg/test-vcpkg-linux.sh
```

`build-tests.sh` currently excludes `APITest.C_API_Test`. Do not describe that
run as complete C API coverage, and re-evaluate the exclusion when changing the
C API or its implementation.

### vcpkg consumer tests

The scripts under `tests/vcpkg` configure, build, link, and run a separate
consumer against the package:

```powershell
.\tests\vcpkg\test-vcpkg-windows.ps1 -VcpkgRoot C:\path\to\vcpkg
.\tests\vcpkg\test-vcpkg-windows.ps1 -VcpkgRoot C:\path\to\vcpkg -WinInet
```

```bash
./tests/vcpkg/test-vcpkg-linux.sh
./tests/vcpkg/test-vcpkg-macos.sh
./tests/vcpkg/test-vcpkg-ios.sh --simulator
./tests/vcpkg/test-vcpkg-android.sh
```

See [the vcpkg test README](../tests/vcpkg/README.md) for prerequisites,
triplets, supported Android ABIs, and device deployment.

### Android

Android work requires Java 17+, the Android SDK, CMake, and a supported NDK.
Set the SDK and NDK environment variables to the installed locations:

```powershell
$env:ANDROID_SDK_ROOT = "C:\Android\android-sdk"
$env:ANDROID_HOME = $env:ANDROID_SDK_ROOT
$env:ANDROID_NDK_HOME = "$env:ANDROID_SDK_ROOT\ndk\27.0.12077973"
$env:ANDROID_NDK = $env:ANDROID_NDK_HOME
```

Use the Gradle project under `lib/android_build` for the AAR and Android test
application. Install an emulator image through Android Studio or the SDK manager
when no physical device is available. Run the test application on the emulator
or a connected device and inspect its logcat output. Changes to JNI, Room,
storage, networking, packaging, or ABI selection should not be validated only
with a host build.

Read [Getting started with Android](cpp-start-android.md) before changing the
Android HTTP bridge or choosing between Room and native SQLite.

### macOS and iOS

Use a macOS machine with Xcode for Apple-native code:

```bash
./tests/vcpkg/test-vcpkg-macos.sh
./tests/vcpkg/test-vcpkg-ios.sh --simulator
```

For changes to Objective-C/Swift wrappers, Apple networking, packaging, or
platform lifecycle behavior, also build the xcframework and run a representative
sample in an iOS Simulator. Use a physical iOS device when the behavior depends
on device identity, networking, background execution, or platform APIs that a
simulator cannot represent.

See [Getting started with iOS](cpp-start-ios.md),
[Getting started with macOS](cpp-start-macosx.md), and
[`tools/apple/README.md`](../tools/apple/README.md).

## Choosing validation depth

Scale validation to the affected behavior and the cost of a missed regression.
The examples below are guidance, not permission to ignore a platform selected by
the changed code.

| Change risk | Expected validation |
| --- | --- |
| Documentation or isolated, non-behavioral change | Manual diff review and any repository documentation checks; rely on CI for unaffected build matrices |
| Focused implementation change | Targeted unit/functional tests plus a Debug or Release build on the primary affected platform |
| Shared C++, CMake, dependency, storage, or transport change | Windows and WSL/Linux builds; relevant static/shared, package-consumer, and transport combinations |
| Android or Apple platform change | Host checks plus emulator/simulator execution; use a physical device when platform behavior warrants it |
| Cross-platform or release-critical change | Windows, WSL/Linux, Android, macOS, iOS Simulator, package-consumer tests, and representative physical-device checks where feasible |
| High-risk integration or ingestion change | Embed the local SDK in a real consuming application, exercise the changed scenario end to end, and verify expected events in the authorized telemetry destination |

Manual review and CI can be sufficient for a small, low-risk change. A larger
change should use more independent environments. Record what was and was not
tested in the PR so reviewers can assess residual risk.

## End-to-end telemetry validation

When package-level tests are insufficient:

1. Build the SDK locally in the same linkage and configuration used by a real
   consumer.
2. Point the consumer at that local build without committing machine-specific
   paths.
3. Exercise a scenario that emits an identifiable, privacy-safe event.
4. Confirm the application remains stable through initialization, upload,
   flush, shutdown, and any background/service lifecycle involved.
5. Use an approved, authenticated query path to verify that the event reached
   the intended database and table. Bound the query by test time and safe
   correlation fields.
6. Record event time, ingestion time, SDK version, platform, and result. Do not
   commit access tokens, tenant tokens, customer data, raw device identifiers,
   or query exports.
7. Remove temporary services, applications, local overrides, credentials, and
   test artifacts.

An empty offline queue alone does not prove ingestion: an event can be accepted,
rejected, or removed before the downstream table is queried. Verify the
destination when end-to-end delivery is the behavior under test.

## Pull-request checklist

- Public and private-module source configurations still compile as applicable.
- Changed public headers compile for consumers and preserve intended ABI/API
  behavior.
- Relevant unit, functional, package-consumer, and platform tests pass.
- No relevant tests are silently excluded.
- Static/shared and Debug/Release differences were considered.
- WinHTTP/WinInet, Room/SQLite, and platform PAL differences were considered
  where relevant.
- Documentation and examples match the implementation.
- The diff contains no credentials, tenant tokens, customer data, generated
  build trees, or machine-specific paths.
- CI failures and skipped jobs are understood before merge.

## Cutting a release

Releases use four components: `X.Y.Z.W`. The current generator policy is
documented in [`Solutions/version.txt`](../Solutions/version.txt); do not
hand-edit a version based on an outdated example.

### Prepare the release

1. Start from current `main` after required changes have merged and required CI
   is green.
2. Generate `lib/include/public/Version.hpp`:

   ```powershell
   .\tools\gen-version.cmd
   ```

   ```bash
   ./tools/gen-version.sh
   ```

3. Verify the generated version, repository status, and diff. The release
   preparation PR should contain only intentional release changes.
4. Open and merge the release preparation PR through the normal review and
   approver process.

### Publish the release

1. Update local `main` to the merged release commit.
2. Create the four-component tag `vX.Y.Z.W` on that commit and push it to the
   canonical repository.
3. Publish a non-draft, non-prerelease GitHub Release for the tag with release
   notes.
4. Verify that the GitHub source archives resolve and contain the generated
   version.

Do not move or force-update a published release tag. Correct a bad release with
a new build/version component and release notes describing the correction.

### Verify downstream publication

Publishing the GitHub Release triggers:

- `.github/workflows/spm-release.yml`, which builds and validates
  `MATTelemetry.xcframework`, uploads it to the release, and publishes the
  parallel three-component Swift Package Manager tag.
- `.github/workflows/vcpkg-release-bump.yml`, which can prepare a
  `microsoft/vcpkg` port update when its fork and credential settings are
  configured.

Check both workflows; do not assume that the release trigger completed all
distribution work.

The vcpkg workflow requires repository configuration and a token capable of
pushing to a fork and opening the upstream PR. If those credentials are
intentionally unavailable, manually:

1. Branch from current `microsoft/vcpkg` `master`.
2. Update `ports/cpp-client-telemetry/portfile.cmake` to the new tag and source
   archive SHA512.
3. Update and format the port manifest.
4. Run `vcpkg x-add-version cpp-client-telemetry --overwrite-version`.
5. Build the production port from the published archive, not from a local SDK
   source override.
6. Commit the port and version-database changes and open a
   `microsoft/vcpkg` PR.

Coordinate updates to other maintained distribution channels, including
CocoaPods, when the release is expected to be available there. Validate each
package through a clean external consumer rather than only checking that an
artifact exists.

### Post-release checklist

- GitHub Release and four-component tag point to the intended commit.
- `Version.hpp` in the source archive reports the release version.
- Release notes describe compatibility or migration requirements.
- SPM artifact, checksum, external consumer build, and three-component tag
  succeeded.
- The vcpkg port PR is open or the existing port already matches the release.
- Other maintained package feeds have been updated or explicitly tracked.
- A representative consumer can build against the released package.
- Release-critical telemetry changes have an authorized end-to-end ingestion
  result.

If a release workflow fails after publication, use its manual dispatch for the
same tag when the workflow supports an idempotent rerun. Do not create duplicate
tags or silently replace published artifacts without understanding the
workflow's recovery behavior.
