# Getting Started - Windows

This tutorial guides you through the process of integrating the 1DS C++ SDK into your existing C++ Windows app or service.

## **Clone and provide authentication credentials, if required**

### 1. Cloning the repository

```git clone https://github.com/microsoft/cpp_client_telemetry.git```

to clone the repo. If your project requires UTC to send telemetry, you need to add `--recurse-submodules` while cloning to tell git to add `lib/modules` repo that contains various proprietary Microsoft modules, including Universal Telemetry Client support.

### 2. You will be asked for your credentials to clone the repo. Generate a PAT token using GitHub UI, use your username and PAT token to clone the repo. See [Generating PAT token for command line clone](https://help.github.com/en/github/authenticating-to-github/creating-a-personal-access-token-for-the-command-line)

## **Windows prerequisites and dependencies for building from source**

* Visual Studio 2017 or 2019
* C++ Dev Tools

## **Build the SDK from source using Visual Studio**

* Open the *cpp_client_telemetry/Solutions/MSTelemetrySDK.sln* solution in Visual Studio.
* Alternatively you can use *build-all.bat* located in workspace root folder to build from command line

If your build fails, then you most likely missing the following optional Visual Studio components:

* ATL support
* ARM64 support
* Spectre mitigation libraries

Please review the script [here](../tools/setup-buildtools.cmd) that installs all the necessary dependencies.

Make sure you install all of these above optional Visual Studio components, as these are required for the SDK to build all SDK SKUs successfully for all platform types. If you are using GitHub Actions infrastructure to build your project, then all of these optional components are available by default as part of stock GitHub Actions deployment.

Specific version of Windows 10 SDK is referenced by the project(s). You will need to install that exact version of Windows 10 SDK, but you may also manually upgrade all projects to current latest version of Windows 10 SDK for your local build. SDK may periodically update its Windows 10 SDK dependency from time to time. Your responsibility is to decide what Windows 10 SDK you need to use for shipping 1DS C++ SDK build from source.

See:

* [How to add optional components in Visual Studio 2017](https://docs.microsoft.com/en-us/visualstudio/install/modify-visual-studio?view=vs-2017)
* [Download Windows 10 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)

## **Windows prerequisites and dependencies for building from source using LLVM compiler**

* Visual Studio
* [LLVM compiler](https://releases.llvm.org/download.html)
* [CMake build system](https://cmake.org/download/)
* [LLVM compiler toolchain for Visual Studio](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.llvm-toolchain)

Installing dependencies:

1. Start elevated `cmd.exe` shell (Run As Administrator...)
2. `SET INSTALL_LLVM=1` or invoke `tools\install-llvm.cmd` to download and install LLVM. You may skip this step if you already have it installed.
3. Launch `tools\setup-buildtools.cmd` script to install all other build dependencies.

Make sure you can build a simple "Hello World" using CMake before proceeding to the next step.

To build SDK using cmake with clang on Windows, run:

```build-cmake-clang-vs2017.cmd```

  or

```build-cmake-clang-vs2019.cmd```

depending on what Visual Studio version you are using.

## **Instrument your code to send a telemetry event**

### 1. Include the main 1DS C++ SDK header file in your main.cpp by adding the following statement to the top of your app's implementation file

```cpp
#include "LogManager.hpp"
```

### 2. Introduce the 1DS SDK (Beta) namespace by adding the following statement after your include statements at the top of your app's implementation file

```cpp
using namespace Microsoft::Applications::Events;
```

### 3. Create the default LogManager instance for your project using the following macro in your main file

```cpp
LOGMANAGER_INSTANCE
```

### 4. Initialize the 1DS SDK, create and send a telemetry event, then flush the event queue and shut down the telemetry

logging system by adding the following statements to your main() function:

```cpp
ILogger* logger = LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
logger->LogEvent("My Telemetry Event");
//...
LogManager::FlushAndTeardown();
```

_**Important!** Replace the place-holder application key value with the actual value of your application key._

*You're done! You can now compile and run your app, and it will send a telemetry event.*

More examples can be found under *examples* folder.
