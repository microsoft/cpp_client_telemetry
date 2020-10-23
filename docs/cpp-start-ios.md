# Getting started - iOS

This tutorial guides you through the process of integrating the 1DS SDK into your existing C++ iOS app.

## 1. Clone the repository

Run `git clone https://github.com/microsoft/cpp_client_telemetry.git` to clone the repo. If your project requires any of the private modules (ECS, DefaultDataViewer, etc.), you need to add `--recurse-submodules` when cloning to tell git to add `lib/modules` repo. You will be prompted to enter your credentials to clone. Use your MSFT GitHub username and GitHub token.

## 2. Build all

### Simulator

Run `build-ios.sh [clean] [release|debug]` script in the root folder of the source tree. This will fetch the necessary build tools and build the SDK. You will be prompted to enter your root password in order to install the necessary build tools and the resulting SDK package.

### Device

Run `build-ios.sh [clean] [release|debug] [arm64|arm64e]`.

## 3. Integrate the SDK into your C++ project

SDK package contains headers and library installed at the following locations:

* Headers: /usr/local/include/mat
* Library: /usr/local/lib/${arch}/libmat.a

1DS SDK is built using cmake, but you can explore building it with any other build system of your choice.

## 4. Instrument your code to send a telemetry event

### 1. Include the main 1DS SDK header file in your main.cpp by adding the following statement to the top of your app's implementation file

```cpp
#include "LogManager.hpp"
```

### 2. Use namespace by adding the following statement after your include statements at the top of your app's implementation file

```cpp
using namespace Microsoft::Applications::Events;
```

### 3. Create the default LogManager instance for your project using the following macro in your main file

```cpp
LOGMANAGER_INSTANCE
```

### 4. Initialize the SDK, create and send a telemetry event, and then flush the event queue and shut down the telemetry

logging system by adding the following statements to your main() function.

```cpp
ILogger* logger = LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
logger->LogEvent("My Telemetry Event");
//...
LogManager::FlushAndTeardown();
```

You're done! You can now compile and run your app, and it will send a telemetry event using your ingestion key to your tenant.

Please refer to [EventSender](https://github.com/microsoft/cpp_client_telemetry/tree/master/examples/cpp/EventSender) sample for more details. Other sample apps can be found [here](https://github.com/microsoft/cpp_client_telemetry/tree/master/examples/cpp/). All of our SDK samples require CMake build system, but you may consume the SDK using any other alternate build system of your choice (GNU Make, gn, etc.).
