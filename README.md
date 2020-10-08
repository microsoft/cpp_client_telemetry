# 1DS C/C++ SDK

[![Join](https://img.shields.io/badge/Join%20the%20project-brightgreen)](https://repos.opensource.microsoft.com/microsoft/teams/client-telemetry-sdk/join/)
[![spellcheck](https://github.com/microsoft/cpp_client_telemetry/workflows/spellcheck/badge.svg)](https://github.com/microsoft/cpp_client_telemetry/actions?query=workflow%3Aspellcheck)

**1DS C/C++ SDK** enables cross-platform telemetry collection from various
Microsoft products. It enables data / telemetry upload to Collector++.

**1DS** (One Data Strategy), also known as One Observability, is a cross-org
initiative with five teams across the company coming together to unify
multiple telemetry efforts at Microsoft. This includes the teams that built
Asimov, Aria, Shared Data Platform, Geneva, OMS, Azure Monitor, and Visual
Studio App Center. We aim to create a unified data collection, ingestion,
pipeline, and set of tools for key scenarios for the entire company.

**Collector++** is the externally-facing destination end-point where telemetry
data is uploaded to that subsequently routes the data to Microsoft internal
data pipeline.


# Getting Started

The SDK is released as a [source package](https://github.com/microsoft/cpp_client_telemetry/releases)
every month, following the [milestones](https://github.com/microsoft/cpp_client_telemetry/milestones).
There is no plan to release prebuilt binaries.

## Build

To build the SDK, please refer to [How to build the SDK](CONTRIBUTING.md#How_to_build_the_SDK).
<details>
  <summary>Build Environments (click to expand):</summary>
  
  | Operating System              | Compiler                         |
  | ----------------------------- | -------------------------------- |
  | Mac OS X 10.11.6              | Clang Xcode 8.3                  |
  | Mac OS X 10.12.6              | Clang Xcode 9.0, 9.1             |
  | Mac OS X 10.13.3              | Clang Xcode 9.2, 9.3, 10.0, 10.1 |
  | Raspbian GNU/Linux 8 (jessie) | GCC 4.9.2 (armv7l)               |
  | Ubuntu 14.04.* LTS            | GCC 4.8.*, 4.9.4                 |
  | Ubuntu 14.04.1 LTS            | GCC 5.x.x                        |
  | Ubuntu 16.04 LTS              | GCC 5.x.x (armv7l)               |
  | Windows 10                    | Android Studio/Gradle            |
  | Windows Server 2016           | Visual Studio 2017 (vc141)       |
  | Windows Server 2019           | Visual Studio 2019 (vc142)       |
</details>

<details>
  <summary>Target Platforms (click to expand):</summary>
  
  | Target Platform                | Supported          | Covered by CI      |
  | ------------------------------ | ------------------ | ------------------ |
  | Android                        | partial&dagger;    | :white_check_mark: |
  | iOS 10+ (simulator)            | :white_check_mark: | :white_check_mark: |
  | iOS 10+ (arm64, arm64e)        | :white_check_mark: |                    |
  | Linux (x86, x64, arm, aarch64) | :white_check_mark: |                    |
  | Mac OS X 10.11+                | :white_check_mark: |                    |
  | Mac OS X (latest)              | :white_check_mark: | :white_check_mark: |
  | Ubuntu 14.04.x LTS             | :white_check_mark: | :white_check_mark: |
  | Ubuntu (latest)                | :white_check_mark: | :white_check_mark: |
  | Windows 7.1                    | :white_check_mark: |                    |
  | Windows 8.1                    | :white_check_mark: |                    |
  | Windows 10.x                   | :white_check_mark: |                    |
  | Windows Server 2012            | :white_check_mark: |                    |
  | Windows Server 2016            | :white_check_mark: | :white_check_mark: |
  | Windows Server 2019            | :white_check_mark: |                    |
  
  * **Supported** - these platforms are known to work well with the SDK in
    production.
  * **Covered by CI** - these platforms are tested as part of CI.
  * &dagger; **Android** - supported for Office applications. oteldiscuss@Microsoft.com is a first point of contact.
</details>

## Test

There are two sets of tests available:
* [tests/unittests](tests/unittests) - unit tests that verify operation of
  each individual component.
* [tests/functests](tests/functests) - functional tests that verify
  customer-facing APIs and features, they also cover the flow of data to
  Collector++.

These tests use Google Test / Google Mock framework and get built alongside
the SDK. Launch `functests` and `unittests` binary executables to capture
the test results.

# Getting Support

We recommend [GitHub issues](https://github.com/microsoft/cpp_client_telemetry/issues/new/choose)
as the communication channel for both feature requests and issues.

We are also available on email and community meeting, please refer to [CONTRIBUTING.md](CONTRIBUTING.md)
for more details.

**Note**:
* Please refer to the supported [platforms](#build), anything not in the list
  should be treated as a feature request.
* Only C and C++ API surface is supported - any other language wrappers are
  provided as-is, and not supported.
* Build issues are in general out of the support scope due to the unlimited
  number of build flags combinations.

# Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for more details.

# Versioning

This library follows [Semantic Versioning](http://semver.org/).

# License

By contributing to 1DS C++ SDK repository, you agree that your contributions
will be licensed under [Apache License 2](LICENSE).
