# 1DS C++ SDK iOS/macOS Podspec Onboarding

This tutorial guides you through the process of integrating the [1DS SDK](https://github.com/microsoft/cpp_client_telemetry) into your existing iOS and macOS app. Here we consume the [Msblox Podspec]() and published [Obj-C Wrappers](https://github.com/microsoft/cpp_client_telemetry/tree/master/wrappers/obj-c). If you want to consume the C++ Library directly, please follow the tutorials at [Getting started - iOS](https://github.com/microsoft/cpp_client_telemetry/blob/master/docs/cpp-start-ios.md) and [Getting Started - Mac OS X](https://github.com/microsoft/cpp_client_telemetry/blob/master/docs/cpp-start-macosx.md)

## Add OneDsCppSdk Cocoapod to the Podfile

The first version is `3.5.67`.

```
source 'https://msblox.visualstudio.com/DefaultCollection/_git/CocoaPods'

pod 'OneDsCppSdk’, '3.5.67'
```

If you are using the pod for the first time, run `pod install`, otherwise, if updating the version, run `pod update`. 

## Update your code to send telemetry event

### Add Obj-C headers to your project's bridging-header file. 
```
#import "ODWLogConfiguration.h"
#import "ODWLogManager.h"
#import "ODWLogger.h"
#import "ODWEventProperties.h"
#import "ODWPrivacyGuard.h"
#import "ODWCommonDataContext.h"
```
Note: You may not need all the headers here based on how you initialize the logger and events.

If your project does not contain a bridging header file, then you may need to create if by yourself. 
To add one manually, add a new file to your Xcode project (File > New > File...) then select “Header File” and click the next button. Name your file “<<YourProjectName>>-Bridging-Header.h”.
Be sure to type your own actual project name instead and take note of the capitalization of the words. If your project name includes a space (Example: “Hello World”) use an underscore between the words.

### Build your project

Make sure your project builds after the previous steps. If you are having issues, then you might not have access to the private repos of 1DS C++ SDK. Make sure you have all the required access.

### Use the Obj-C function signatures in the code. 
- Initialize Logger with Aria Project Key

``` swift
var logger: ODWLogger?
ODWLogManager.setTransmissionProfile(.realTime)
logger = ODWLogManager.initForTenant(ariaProjectKey)
```

- Create Event

``` swift

// Create event
let event  =  ODWEventProperties(name: "TestEvent")

// Log telemetry event
logger?.logEvent(with: event)
// …
ODWLogManager.flushAndTeardown()
```
You're done! You can now compile and run your app, and it will send a telemetry event using your ingestion key to your tenant.
