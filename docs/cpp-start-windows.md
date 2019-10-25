
This tutorial guides you through the process of integrating the 1DS C++ SDK into your existing C++ Windows app or service.

## **Clone the repository**

1. Run

```git clone https://github.com/microsoft/cpp_client_telemetry.git```

to clone the repo. If your project requires UTC to send telemetry, you need to add `--recurse-submodules` while cloning to tell git to add `lib/modules` repo that contains various proprietary Microsoft modules, including Universal Telemetry Client support.

2. You will be asked your credentials to clone the repo. Generate a PAT token using GitHub UI, use your username and PAT token to clone the repo. See [Generating PAT token for command line clone](https://help.github.com/en/github/authenticating-to-github/creating-a-personal-access-token-for-the-command-line)

## **Windows prerequisites and dependencies for building from source using Visual Studio 2017**

* Visual Studio 2017.
* C++ Dev Tools for Visual Studio 2017.
* Visual Studio 2019 _should work_ if you use vc141 toolset; newer toolsets _might work_, but not tested.

## **Build the SDK from source using Visual Studio 2017**

* Navigate to the folder where the SDK was cloned and open the cpp_client_telemetry/Solutions folder.
* Open the MSTelemetrySDK.sln file to open the project with Visual Studio.
* You can use batch build all to build all solutions
* Alternatively build from command line using *build-all.bat* conveniently located at build root

If your build fails, then you most likely missing the following optional Visual Studio components:
- ATL support
- ARM64 support
- Spectre mitigation libraries
- specific version of Windows 10 SDK referenced by the project(s)

Make sure you install all of these optional Visual Studio components, as these are required for the SDK to build successfully. If you are using GitHub Actions infrastructure to build your project, then all of these are available by default as part of stock GitHub Actions deployment.

## **Windows prerequisites and dependencies for building from source using LLVM compiler**

* [LLVM compiler](https://releases.llvm.org/download.html)
* [CMake build system](https://cmake.org/download/)
* [LLVM compiler toolchain for Visual Studio](https://marketplace.visualstudio.com/items?itemName=LLVMExtensions.llvm-toolchain)

- Install CMake for Windows into C:\Program Files\CMake\bin\ (default path)
- Install **[LLVM/clang for Windows](http://releases.llvm.org/7.0.0/LLVM-7.0.0-win64.exe)** - version 7 or newer into C:\Program Files\LLVM\bin (default path)

Make sure you can build a simple "Hello World" using cmake before proceeding to the next step.
To build SDK using cmake with clang on Windows, run:

```build-cmake-clang.cmd```

## **Instrument your code to send a telemetry event**

1. Include the main 1DS C++ SDK header file in your main.cpp by adding the following statement to the top of your app's implementation file.

	```
    #include "LogManager.hpp"
	```
    
2. Introduce the 1DS SDK (Beta) namespace by adding the following statement after your include statements at the top of your app's implementation file.

    ```
    using namespace Microsoft::Applications::Events; 
    ```

3. Create the default LogManager instance for your project using the following macro in your main file:

	```
    LOGMANAGER_INSTANCE
    ```

4. Initialize the 1DS SDK, create and send a telemetry event, then flush the event queue and shut down the telemetry
logging system by adding the following statements to your main() function:

    ```
    ILogger* logger = LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
    logger->LogEvent("My Telemetry Event");
    ...
    LogManager::FlushAndTeardown();
    ```
    _**Important!** Replace the place-holder application key value with the actual value of your application key._

*You're done! You can now compile and run your app, and it will send a telemetry event.*

More examples can be found under *examples* folder.

