#!/bin/sh

if [ "$1" == "skip_build" ]; then
SKIP_BUILD=true
else
SKIP_BUILD=false
fi

if [ ! SKIP_BUILD ]; then

# Build for iOS
./build-ios.sh release arm64

# Build for Android
pushd ./lib/android_build
gradle build
popd

fi

# Copy artifacts for iOS
rsync -a ./out/lib/libmat.a "./wrappers/xamarin/sdk/OneDsCppSdk.iOS.Bindings/Native References/"

# Copy artifacts for Android
mkdir -p ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/release/obj/arm64-v8a/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/arm64-v8a/
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/release/obj/armeabi-v7a/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/armeabi-v7a/
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/release/obj/x86/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/x86/
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/release/obj/x86_64/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/x86_64/
rsync -a ./lib/android_build/maesdk/build/outputs/aar/maesdk-release.aar ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/Jars/

# Build Xamarin Bindings Solution
