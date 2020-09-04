#!/bin/sh

# Build for iOS
./build-ios.sh release arm64

# Build for Android
cd ./lib/android_build
gradle build