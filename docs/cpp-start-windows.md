
This tutorial guides you through the process of integrating the 1DS SDK (Beta) into your existing C++ Windows app or service.

## **Clone the repository**

1. Run `git clone https://github.com/microsoft/cpp_client_telemetry.git` to clone the repo.

	If your project requires UTC to send telemetry, you need to add `--recurse-submodules` when cloning to tell git to add `lib/modules` repo.

2. You will be asked your credentials to clone the repo, use your username and password as entered on Github
	
    If you do not have those credentials, generate them and use the username and password that you enabled.
    
## **Windows prerequisites and dependencies for building from source using Visual Studio 2017**

* Visual Studio 2017
* C++ Dev Tools for Visual Studio 2017
    
## **Build the SDK from source using Visual Studio 2017**

* Navigate to the folder where the SDK was cloned and open the cpp_client_telemetry/Solutions folder

* Open the MSTelemetrySDK.sln file to open the project with Visual Studio 2017.

* Expand the Samples folder, the project SampleCpp should be located there

![SampleCpp](/docs/images/SampleCpp.PNG)

* Go to the SampleCpp project properties and make sure that Visual Studio 2017 is set as Platform Toolset

![SampleCppProperties](/docs/images/SampleCppProperties.PNG)

* When you build the SampleCpp sample app Visual Studio will also build the SDK, as it is listed as a dependency.
    The win32-dll project is the SDK dll, when Visual Studio builds it, it will also build all it's references, including sqlite, zlib and all the headers for the SDK to work.
	
![win32-dll](/docs/images/87016-win32lib.png)

## **Windows prerequisites and dependencies for building from source using LLVM compiler**

* LLVM compiler
* CMake build system

### Install instructions

- Install CMake for Windows into C:\Program Files\CMake\bin\ (default path)
- Install **[LLVM/clang for Windows](http://releases.llvm.org/7.0.0/LLVM-7.0.0-win64.exe)** and install into C:\Program Files\LLVM\bin (default path)

    
## **Build the SDK from source using LLVM compiler**

1. Navigate to the directory where the SDK is located

2. Run the file `build-cmake-clang.sh` to build the SDK, this will build the SDK along with Unit and Functional Tests.

	To disable building the tests go to the CMakeLists.txt file on the root of the SDK directory and change:
    
    ```
    option(BUILD_UNIT_TESTS   "Build unit tests"        YES)
    option(BUILD_FUNC_TESTS   "Build functional tests"  YES)
    ```
    
    to: 
    ```
    option(BUILD_UNIT_TESTS   "Build unit tests"        NO)
    option(BUILD_FUNC_TESTS   "Build functional tests"  NO)
    ```

	_**Note:** In order to build from scratch all dependencies along with the SDK you need to run:_ ` ./build-cmake-clang.sh clean`
    
    _**Note:** If you want to skip building dependencies for the SDK you need to run:_ ` ./build-cmake-clang.sh nodeps`
    
3. The SDK will be installed under `<PATH>`



## **Instrument your code to send a telemetry event**

1. Include the main 1DS SDK (Beta) header file in your main.cpp by adding the following statement to the top of your app's implementation file.

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

4. Initialize the 1DS SDK (Beta) events system, create and send a telemetry event, and then flush the event queue and shut down the telemetry
logging system by adding the following statements to your main() function.

    ```
    ILogger* logger = LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
    logger->LogEvent("My Telemetry Event");
    ...
    LogManager::FlushAndTeardown();
    ```
    _**Important!** Replace the place-holder application key value with the actual value of your application key._

*You're done! You can now compile and run your app, and it will send a telemetry event.*

 _**Note:** If you need a deeper look into how to use the 1DS C++ SDK there is a couple of folders with examples under Aria.SDK.Cpp/examples_


