
# Xamarin bindings for 1DS C++ SDK

The Xamarin bindings for the 1DS C++ SDK are available for iOS and Android. In both cases, the bindings are for the wrappers in iOS and Android native languages to avoid having to maintain native wrappers to the C++ SDK in 2 different places.

This means that the wrappers and bindings create the following layers:

- **iOS:** Xamarin.iOS -> Objective-C -> C++
- **Android:** Xamarin.Android -> Java -> C++

## 1. Clone the repository

Run `git clone https://github.com/microsoft/cpp_client_telemetry.git` to clone the repo. If your project requires UTC to send telemetry, you need to add `--recurse-submodules` when cloning to tell git to add `lib/modules` repo. You will be prompted to enter your credentials to clone. Use your MSFT GitHub username and GitHub token.

## 2. Pre-requisites

Add an environment variable called ANDROID_SDK_ROOT set to the path of the Android SDK

## 3. Build the Xamarin bindings

Build the C++ library and Xamaring bindings for Android and iOS with the following command

```bash
./build-xamarin.sh [debug|release] [cleanAll|cleanXamarin] [xamarinOnly] [package]
```

- **debug|release**: build configuration to use. Default to release of not specified
- **cleanAll**: deletes output and temporary directories for all platforms
- **cleanXamarin**: deletes output and temporary directories for the Xamarin solution only
- **xamarinOnly**: only builds the Xamarin solution, assuming that iOS and Android SDKs have already been built
- **package**: packages the Xamarin bindings in a NuGet package

For additional on additional options for building for database implementation options for Android, refer to [cpp-start-android.md](docs\cpp-start-android.md#2-build-all)

## 4. Integrate the SDK into your Xamarin project

### Using SDK directly in Xamarin.Android or Xamarin.iOS project

To integrate the SDK directly into the Xamarin projects add a reference to the NuGet package in your Xamarin project.

#### Android

- Add a reference to the **Xamarin.AndroidX.Room.Runtime** NuGet package
- Initialize the Logger from your Application class or main activity if you are building a single activity app:

```csharp
using Microsoft.Applications.Events;
```

```csharp
logger = LogManager.Initialize(Application.Context, "your_Aria_ingestion_token");
```

#### iOS

- Initialize the Logger from your AppDelegate's FinishedLaunching method:

```csharp
using Microsoft.Applications.Events;
```

```csharp
public override bool FinishedLaunching(UIApplication application, NSDictionary launchOptions)
{
    ...
    logger = LogManager.Initialize("your_Aria_ingestion_token");
    ...
}
```

## 5. Logging an event

Include the SDK's namespace

```csharp
using Microsoft.Applications.Events;
```

```csharp
EventProperties props = new EventProperties("MyCustomEvent", DiagnosticLevel.DiagLevelRequired);
    props.SetProperty("MyKey", "Some value");
    props.SetProperty("MyTimestamp", DateTime.Now.ToString("HH:mm:ss.fff"));

logger.LogEvent(props);
```
