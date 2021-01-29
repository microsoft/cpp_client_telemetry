# Getting started - Android

This tutorial guides you through the process of integrating the 1DS SDK into your Android app.

## 1. Clone the repository

Run `git clone https://github.com/microsoft/cpp_client_telemetry.git` to clone the repo. If your project requires UTC to send telemetry, you need to add `--recurse-submodules` when cloning to tell git to add `lib/modules` repo. You will be prompted to enter your credentials to clone. Use your MSFT GitHub username and GitHub token.

## 2. Build all

You will ideally build the SDK using the same versions of the Android SDK, NDK, and build tooling as your existing app. The repo includes a directory lib/android_build that can be used as an Android Studio project or to build using Gradle. The GitHub CI loop uses Gradle in this directory to build and test the SDK. As with any C++ code that exposes std:: container types in its API, you need to ensure that templates compile compatibly across compilation units, and you need to ensure that all compilation units are linking against the same C++ shared library. The ```build.gradle``` configuration in both modules in ```android_build``` selects the Android llvm shared C++ library. The SDK will not compile and link against the older Android C++ libraries; you will need to use this same library option in your application in order to use the SDK.

The Gradle wrapper in ```android_build``` builds two modules, ```app``` and ```maesdk```. The ```maesdk``` module is the SDK packaged as an AAR, with both the Java and C++ components included. The AAR includes C++ shared libraries for four ABIs (two ARM ABIs for devices and two Intel ABIs for the emulator). Android Gradle (as usual) supports debug and release builds, and the Gradle task ```maesdk:assemble``` should build both flavors of AAR.

On Android, there are two database implementations to choose from. By default (the master branch on Github), the SDK will use the Android-supported androidx.Room database package. This reduces APK size because we don't need to compile and link in a copy of SQLite in native code (SQLite is hundreds of kB per ABI of APK file size). Room does have a slight CPU performance disadvantage since database transactions cross the JNI boundary when native code uses it. If you wish to change from Room to the native SQLite implementation, you should change the two module ```build.gradle``` files (app and maesdk). In those files, you will see an argument to CMake to select Room: ```"-DUSE_ROOM=1"```. Change this to ```"-DUSE_ROOM=0``` to select the native SQLite.

The Room database implementation adds one additional initialization requirement, since it needs a pointer to the JVM and an object reference to the application context. See below (4.5) for the required call to either ```connectContext``` (in Java) or ```ConnectJVM``` (in C++) to set this up.

If you are building on Windows, this helper script [build-android.cmd](../build-android.cmd) is provided to illustrate how to deploy the necessary SDK and NDK dependencies. Once you installed the necessary dependencies, you may use Android Studio IDE for local builds. See [ide.cmd](../lib/android_build/ide.cmd) that shows how to build the project from IDE. The `app` project (`maesdktest`) allows to build and run all SDK tests on either emulator or real Android device. While the tests are running, you can monitor the test results in logcat output.

Default environment variables used by `build-android.cmd` script:

```console

set "ANDROID_NDK_VERSION=21.1.6352462"
set "ANDROID_CMAKE_VERSION=3.10.2.4988404"
set "ANDROID_SDK_ROOT=C:\Android\android-sdk"
set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
set "ANDROID_NDK=%ANDROID_SDK_ROOT%\ndk\%ANDROID_NDK_VERSION%"
set "ANDROID_NDK_HOME=%ANDROID_NDK%"

```

You can specify your own versions of dependencies as needed.

>Note: Only Java JDKs 8-13 will work. Java JDK 14+ will fail to build, due to an issue with the version of Gradle currently in use.

## 3. Integrate the SDK into your C++ project

If you use the lib/android_build Gradle files, they build the SDK into maesdk.aar in the output folders of the maesdk module in lib/android. You can package or consume this AAR in your applications modules, just as you would any other AAR.

For the curious: the app module is an Android application. The GitHub CI loop uses it as a platform to run the unit test suite against the SDK.

## 4. Instrument your code to send a telemetry event

### 1. Include header

Include the main 1DS SDK header file in your main.cpp by adding the following statement to the top of your app's implementation file.

```cpp
#include "LogManager.hpp"
```

### 2. Namespace

Use the namespace, either via using namespace or a namespace alias.

#### 1. Blanket using

Use namespace by adding the following statement after your include statements at the top of your app's implementation file.

```cpp
using namespace Microsoft::Applications::Events;
```

#### 2. Namespace alias

If you prefer to avoid namespace collision, you can create a namespace alias instead:

```cpp
namespace MAE = Microsoft::Applications::Events;
```

#### 3. Verbose

You can simply prefix all the SDK names with ```Microsoft::Application::Events```, as you prefer.

### 3. Instantiate the log manager

Create the default LogManager instance for your project using the following macro in your main C++ file:

```cpp
LOGMANAGER_INSTANCE
```

### 4. Load the native library

Load the shared library from the maesdk AAR (or .so) file. In Java, you call ```System.load_library``` to load the shared object. Loading the dynamic library registers its JNI entry points and permits the Java component of the SDK to call into the C++ portion of the SDK. This is typically done in static initialization in the application:

```java
static {
    System.loadLibrary("maesdk");
}
```

If you fail to do this, you will see errors in logcat at runtime when JNI is unable to link to the native methods in the SDK's Java classes.

### 5. Initialize the Java layer

Some part of the application must create a singleton instance of the Java class
```com.microsoft.applications.events.HttpClient```. The constructor for this class takes one parameter, the application ```Context``` object. In most cases, it will be easiest to do this from Java:

```java
HttpClient client = new HttpClient(getApplicationContext());
```

One could create this instance from C++ code if that code has the appropriate JNIEnv*or JavaVM* pointers (one can obtain JavaVM*from JNIEnv* or vice-versa) and a jobject reference to the application ```Context``` instance. This should occur before initialization of the C++ side of the SDK.

The lifetime of the reference created here is unimportant; the C++ side of the SDK will take a global (static) reference on this singleton and keep it alive until it destructs.

If you are using the (default) Room database implementation, you will need to call either a static method on the Java OfflineRoom class or on the native OfflineStorage_Room class. The Java call is:

```java
OfflineRoom.connectContext(getApplicationContext()); // from an object that has getApplicationContext
```

On the native side, one would call:

```cpp
JNIEnv *env; // the JNI pointer for this thread
jobject context; // a jobject reference to the application context
::Microsoft::Applications::Events::OfflineStorage_Room::ConnectJVM(env, context);
```

### 6. Send some telemetry

Initialize the SDK, create and send a telemetry event, and then flush the event queue and shut down the telemetry
logging system by adding the following statements to your main() function.

```java
// preface the references to the SDK symbols with MAE:: if you use that namespace alias declaration
ILogger* logger = LogManager::Initialize("0123456789abcdef0123456789abcdef-01234567-0123-0123-0123-0123456789ab-0123");
logger->LogEvent("My Telemetry Event");
...
LogManager::FlushAndTeardown();
```

You're done! You can now compile and run your app, and it will send a telemetry event using your ingestion key to your tenant.

Note that it is possible to use more than one log manager. See [examples/cpp/SampleCppLogManagers](https://github.com/microsoft/cpp_client_telemetry/tree/master/examples/cpp/SampleCppLogManagers) for a sample implementation.

Please refer to [EventSender](https://github.com/microsoft/cpp_client_telemetry/tree/master/examples/cpp/EventSender) sample for more details. Other sample apps can be found [here](https://github.com/microsoft/cpp_client_telemetry/tree/master/examples/cpp/). The lib/android_build gradle wrappers will use the Android gradle plugin, and that in turn will use CMake/nmake to build C++ object files.

## 4. Device File Locations

You may find these helpful for debugging. All device files will be found under the path `/data/data/`*app-name*`/` on the device, where *app-name* is the application’s name (such as `com.microsoft.applications.events.maesdktest`).

### 1. Log Files

`.../cache/mat-debug-10782.log`: one log file per session

Local logs are only generated if `HAVE_MAT_LOGGING` is defined in `lib/include/mat/config*.h` file and `LogConfigurationKey.CFG_BOOL_ENABLE_TRACE` is set to `true` in client code. If `ANDROID_SUPPRESS_LOGCAT` is not defined, this will also generate LogCat logs. Note that there are some LogCat logs that are created through `__android_log_print` and not tied to this

### 2. Database Files

`.../cache/`*dbname*`.db`: database file (if using OfflineStorage_SQLite), where *dbname* is the database name.
`.../databases/`*dbname*`.db`: database file (if using OfflineStorage_Room).

One should be able to examine (or modify) the contents of these database files with SQLite on any platform, in theory (if anyone does this, please confirm whether or not it works).
