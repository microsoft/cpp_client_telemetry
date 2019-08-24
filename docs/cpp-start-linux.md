
This tutorial guides you through the process of integrating the 1DS SDK (Beta) into your existing C++ Linux app or service.

## 1. Linux Prerequisites for building from source

1DS SDK (Beta) is available as a prebuilt packages for several popular Linux distributions. Go to section #2 if consuming a prebuilt binary package .

**Note:** Contact the 1DS SDK dev team if you would like us to build our SDK for your own Linux distribution / platform / architecture from source.
{: .bg-info }

{% include_relative linux-setup-build.md %}

{% include contents/tutorial-create-api-key.md %}

## 2. Clone the repository

1. Run `git clone https://github.com/microsoft/cpp_client_telemetry.git` to clone the repo.

	If your project requires UTC to send telemetry, you need to add `--recurse-submodules` when cloning to tell git to add `lib/modules` repo.

2. You will be asked your credentials to clone the repo, use your username and password as entered on Github
	
    If you do not have those credentials, generate them and use the username and password that you enabled.

## 3. Get the 1DS SDK (Beta) for C++

Click the following links to download a prebuilt package:

| Distribution | Download link |
|---|---|
| Ubuntu 16.04 LTS | [i386](https://ariamediahost.blob.core.windows.net/sdk/onesdk/msevents-sdk-1.0.349-ubuntu-16.04-i386.deb) |
| Ubuntu 16.04 LTS | [x86_64](https://ariamediahost.blob.core.windows.net/sdk/onesdk/msevents-sdk-1.0.349-ubuntu-16.04-x86_64.deb) |
| Raspberry Pi 3 (Raspbian stretch) | [armhf](https://ariamediahost.blob.core.windows.net/sdk/onesdk/msevents-sdk-1.0.349-debian-8-armhf.deb) |
| More platforms | Coming soon |

Please refer to Linux package installation guides to manually install the downloaded package:
* [How to install .deb package](https://askubuntu.com/questions/40779/how-do-i-install-a-deb-file-via-the-command-line)
* [How to install .rpm package](https://access.redhat.com/solutions/1189)

## 4. Integrate the SDK into your C++ project

SDK package contains headers and library installed at the following locations:

* Headers: /usr/local/include/msevents
* Library: /usr/local/lib/${arch}/libmsevents.a

1DS SDK (Beta) prefers cmake over other build systems, but you can integrate with any other build system of your choice.

### CMakeList.txt example

```
cmake_minimum_required(VERSION 3.1.0)
project(sample1)
set(MSEVENTS_SDK_LIB	/usr/local/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
set(MSEVENTS_SDK_INCLUDE	/usr/local/include/msevents)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0 -ggdb -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb -std=c++11")
find_package (Threads)
# 1DS SDK include dir
include_directories( . ${MSEVENTS_SDK_INCLUDE} )
# Link main.cpp to executable
add_executable(sample1 main.cpp)
source_group(" " REGULAR_EXPRESSION "")
# Prefer linking to recent sqlite3 lib
if(EXISTS "/usr/local/lib/libsqlite3.a")
set (SQLITE3_LIB "/usr/local/lib/libsqlite3.a")
else()
set (SQLITE3_LIB "sqlite3")
endif()
target_link_libraries(sample1 ${MSEVENTS_SDK_LIB}/libmsevents.a curl z ${CMAKE_THREAD_LIBS_INIT} ${SQLITE3_LIB} dl)
```

## 5. Instrument your code to send a telemetry event

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

    **Note** For more advanced usage (control of the telemetry system), use *LogManagerProvider**.
    {: .bg-info }

    **Important!** Replace the place-holder application key value with the actual value of your application key.
    {: .bg-info }

*You're done! You can now compile and run your app, and it will send a telemetry event.*
