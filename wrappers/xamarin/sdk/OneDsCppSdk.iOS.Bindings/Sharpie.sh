### First, build for iOS, ./build-ios.sh clean release arm64
### add #include <Foundation/Foundation.h> to OneDsCppSdk.h
sharpie bind --output=./ -sdk iphoneos14.0 -namespace=Microsoft.Applications.Events -scope=../../../obj-c ../../../obj-c/OneDsCppSdk.h -c -xobjective-c