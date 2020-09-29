### First, build for iOS, ./build-ios.sh clean release arm64
### add #include <Foundation/Foundation.h> to OneDsCppSdk.h
sharpie bind --output=/Users/bemartin/Projects/GitHub/1DsCppSdk/iOSBindingsLibrary/ -sdk iphoneos13.6 -namespace=Com.Microsoft.Applications.Events -scope=./wrappers/obj-c ./wrappers/obj-c/OneDsCppSdk.h -c -xobjective-c