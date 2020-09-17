#!/bin/sh

if [ "$1" == "release" ] || [ "$1" == "debug" ]; then
    BUILD_CONFIGURATION="$1"
else
    BUILD_CONFIGURATION="release"
fi

if [ "$1" == "cleanXamarin" ] || [ "$2" == "cleanXamarin" ]; then
    CLEAN_XAMARIN=true
fi

if [ "$1" == "cleanAll" ] || [ "$2" == "cleanAll" ]; then
    CLEAN_ALL=true
fi

if [ "$1" == "xamarinOnly" ] || [ "$2" == "xamarinOnly" ] ||  [ "$3" == "xamarinOnly" ] ||  [ "$4" == "xamarinOnly" ]; then
    BUILD_XAMARIN_ONLY=true
fi

if [ "$1" == "package" ] || [ "$2" == "package" ] || [ "$3" == "package" ] || [ "$4" == "package" ]; then
    PACKAGE=true
fi

GREEN="\033[1;32m"
RED="\033[1;31m"
NOCOLOR="\033[0m"

if [ "$BUILD_CONFIGURATION" == "debug" ] && [ "$PACKAGE" == true ]; then
    echo "$RED ====== Cannot package in debug configuration $NOCOLOR"
    exit 1
fi

echo "$GREEN ====== Build configuration = $BUILD_CONFIGURATION $NOCOLOR"

# Clean

if [ "$CLEAN_XAMARIN" == true ] || [ "$CLEAN_ALL" == true ]; then
    echo "$GREEN ====== Cleaning Xamarin $NOCOLOR"

    rm -rf ./wrappers/xamarin/sdk/OneDsCppSdk.iOS.Bindings/obj
    rm -rf ./wrappers/xamarin/sdk/OneDsCppSdk.iOS.Bindings/bin
    rm "./wrappers/xamarin/sdk/OneDsCppSdk.iOS.Bindings/Native References/libmat.a"

    rm -rf ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/obj
    rm -rf ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/bin
    rm -rf ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib
    rm ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/Jars/*.aar

    rm -rf ./wrappers/xamarin/sdk/OneDsCppSdk.Standard/obj
    rm -rf ./wrappers/xamarin/sdk/OneDsCppSdk.Standard/bin
fi

# Fail on error
set -e

if [ "$BUILD_XAMARIN_ONLY" != true ]; then

    # Build for iOS
    echo "$GREEN ====== Building for iOS $NOCOLOR"
    if [ "$CLEAN_ALL" == true ]; then
        DO_CLEAN="clean"
    else
        DO_CLEAN=""
    fi

    for arch in arm64 arm64e x86_64
    do
        ./build-ios.sh $DO_CLEAN $BUILD_CONFIGURATION $arch
        mv ./out/lib/libmat.a ./out/lib/libmat.$arch.a

        DO_CLEAN=""
    done

    pushd ./out/lib/
    lipo -create -output libmat.a libmat.arm64.a libmat.arm64e.a libmat.x86_64.a
    popd

    # Build for Android
    echo "$GREEN ====== Building for Android $NOCOLOR"
    pushd ./lib/android_build
    if [ "$CLEAN_ALL" == true ]; then
        gradle clean
    fi
    gradle build
    popd

fi

echo "$GREEN ====== Copying build artifacts $NOCOLOR"

# Copy artifacts for iOS
rsync -a ./out/lib/libmat.a "./wrappers/xamarin/sdk/OneDsCppSdk.iOS.Bindings/Native References/"

# Copy artifacts for Android
mkdir -p ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/$BUILD_CONFIGURATION/obj/arm64-v8a/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/arm64-v8a/
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/$BUILD_CONFIGURATION/obj/armeabi-v7a/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/armeabi-v7a/
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/$BUILD_CONFIGURATION/obj/x86/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/x86/
rsync -a ./lib/android_build/maesdk/build/intermediates/cmake/$BUILD_CONFIGURATION/obj/x86_64/*.so ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/lib/x86_64/
rsync -a ./lib/android_build/maesdk/build/outputs/aar/maesdk-$BUILD_CONFIGURATION.aar ./wrappers/xamarin/sdk/OneDsCppSdk.Android.Bindings/Jars/

# Build Xamarin Bindings Solution
pushd ./wrappers/xamarin

echo "$GREEN ====== Building Xamarin bindings $NOCOLOR"
msbuild ./Microsoft.Applications.Events.Xamarin.sln /p:Configuration=$BUILD_CONFIGURATION -restore -v:minimal

if [ "$PACKAGE" == true ]; then
    echo "$GREEN ====== Creating NuGet package $NOCOLOR"
    nuget pack ./Microsoft.Applications.Events.nuspec
fi

popd