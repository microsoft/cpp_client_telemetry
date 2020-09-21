# Building 1DS C++ SDK with vcpkg

vcpkg is a Microsoft cross-platform open source C++ package manager. Onboarding instructions for Windows, Linux and Mac OS X [available here](https://docs.microsoft.com/en-us/cpp/build/vcpkg). This document assumes that the customer build system is already configured to use vcpkg. 1DS C++ SDK maintainers provide a build recipe, `mstelemetry` port or CONTROL file for vcpkg. Mainline vcpkg repo is refreshed to point to latest stable open source release of 1DS C++ SDK. Public build of SDK does not include private submodules. However, local port / CONTROL file included in 1DS C++ SDK git repo allows to build SDK with additional Microsoft-proprietary private submodules.

## Installing open source mstelemetry package

The following command can be used to install the public open source release:

```
vcpkg install mstelemetry
```

That's it! The package should be compiled for the current OS.

See instructions below to build the SDK with additional Microsoft-proprietary modules.

## Windows build with submodules

`cmd.exe` command line prompt commands:

```
cit clone --recurse-submodules https://github.com/microsoft/cpp_client_telemetry
cd cpp_client_telemetry
vcpkg install --head --overlay-ports=%CD%\tools\ports mstelemetry
```

## POSIX (Linux and Mac) build with submodules

Shell commands:

```
cit clone --recurse-submodules https://github.com/microsoft/cpp_client_telemetry
cd cpp_client_telemetry
vcpkg install --head --overlay-ports=`pwd`/tools/ports mstelemetry
```

## Using response files to specify dependencies

vcpkg allows to consolidate parameters passed to vcpkg in a response file. All 3rd party dependencies needed for 1DS SDK can be described and installed via response file.

Example for Mac:
```
vcpkg install @tools/ports/mstelemetry/response_file_mac.txt
```

Example for Linux:
```
vcpkg install @tools/ports/mstelemetry/response_file_linux.txt
```

## Using triplets

In order to enable custom build flags - vcpkg triplets and custom environment variables may be used. Please see [triplets instruction here](https://vcpkg.readthedocs.io/en/latest/users/triplets/). Response file for a custom build, e.g. `response_file_linux_PRODUCTNAME.txt` may specify a custom triplet. For example, custom triplet controls if the library is built as static or dynamic. Default triplets may also be overridden with [custom triplets](https://vcpkg.readthedocs.io/en/latest/examples/overlay-triplets-linux-dynamic/#overlay-triplets-example). Custom triplets specific to various products must be maintained by product teams. Product teams may optionally decide to integrate their triplets in the mainline 1DS C++ SDK repo as-needed.
