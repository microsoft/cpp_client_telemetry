
This tutorial guides you through the process of integrating the 1DS SDK (Beta) into your existing C++ Linux app or service.

## 1. Linux Prerequisites for building from source

{% include_relative linux-setup-build.md %}

## 2. Clone the repository

1. Run `git clone https://github.com/microsoft/cpp_client_telemetry.git` to clone the repo. If your project requires UTC to send telemetry, you need to add `--recurse-submodules` when cloning to tell git to add `lib/modules` repo.

2. You will be asked your credentials to clone the repo, use your username and GitHub token.	

## 3. Integrate the SDK into your C++ project

SDK package contains headers and library installed at the following locations:

* Headers: /usr/local/include/mat
* Library: /usr/local/lib/${arch}/libmat.a

1DS SDK is built using cmake, but you can explore building it with any other build system of your choice.


## 4. Instrument your code to send a telemetry event

1. Include the main 1DS SDK header file in your main.cpp by adding the following statement to the top of your app's implementation file.

	```
    #include "LogManager.hpp"
	```
    
2. Use namespace by adding the following statement after your include statements at the top of your app's implementation file.

    ```
    using namespace Microsoft::Applications::Events; 
    ```

3. Create the default LogManager instance for your project using the following macro in your main file:

	```
    LOGMANAGER_INSTANCE
    ```

4. Initialize the SDK, create and send a telemetry event, and then flush the event queue and shut down the telemetry
logging system by adding the following statements to your main() function.

    ```
    ILogger* logger = LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
    logger->LogEvent("My Telemetry Event");
    ...
    LogManager::FlushAndTeardown();
    ```

    **Note** For more advanced usage (control of the telemetry system), use *LogManagerProvider**.
    {: .bg-info }

    **Important!** Replace the place-holder application key value with the actual value of your application key.
    {: .bg-info }

*You're done! You can now compile and run your app, and it will send a telemetry event.*
