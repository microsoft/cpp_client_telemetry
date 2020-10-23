# Getting Started - Mac OS X

This tutorial guides you through the process of integrating the 1DS SDK (Beta) into your existing C++ Mac OS X app or service.

## **Mac OS X prerequisites and dependencies for building from source**

Prerequisites:

* Mac OS X 10.10+
* XCode 8+

Dependencies:

* zlib
* sqlite3
* libcurl
* gtest (optional)
* gmock (optional)

_**Note:** Preferably use Homebrew to install missing packages._

## **Clone the repository**

### 1. Run `git clone https://github.com/microsoft/cpp_client_telemetry.git` to clone the repo

If your project requires UTC to send telemetry, you need to add `--recurse-submodules` when cloning to tell git to add `lib/modules` repo.

### 2. You will be asked your credentials to clone the repo, use your username and password as entered on Github

If you do not have those credentials, generate them and use the username and password that you enabled.

## **Build the SDK from source**

### 1. Navigate to the directory where the SDK is located

### 2. Run the file build.sh to build the SDK, this will build the SDK along with Unit and Functional Tests

To disable building the tests go to the **CMakeLists.txt** file in the root of the SDK directory and change

```console
option(BUILD_UNIT_TESTS   "Build unit tests"        YES)
option(BUILD_FUNC_TESTS   "Build functional tests"  YES)
```

to

```console
option(BUILD_UNIT_TESTS   "Build unit tests"        NO)
option(BUILD_FUNC_TESTS   "Build functional tests"  NO)
```

_**Note:** In order to build from scratch all dependencies along with the SDK you need to run: `./build.sh clean`_

### 3. The SDK will be installed under `usr/local/lib/libmat.a`

## **Instrument your code to send a telemetry event**

### 1. Include the main 1DS SDK (Beta) header file in your main.cpp by adding the following statement to the top of your app's implementation file

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

### 4. Initialize the 1DS SDK (Beta) events system, create and send a telemetry event, and then flush the event queue and shut down the telemetry

logging system by adding the following statements to your main() function.

```cpp
ILogger* logger = LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
logger->LogEvent("My Telemetry Event");
//...
LogManager::FlushAndTeardown();
```

_**Important!** Replace the place-holder application key value with the actual value of your application key._

*You're done! You can now compile and run your app, and it will send a telemetry event.*

_**Note:** If you need a deeper look into how to use the 1DS C++ SDK there is a couple of folders with examples under `examples`_
