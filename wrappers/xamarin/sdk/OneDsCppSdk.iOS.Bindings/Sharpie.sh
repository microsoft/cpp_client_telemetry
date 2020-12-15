#!/bin/bash

echo "Before running this script, make sure you:"
echo -e "\t - build for iOS, ./build-ios.sh clean release arm64"
echo -e "\t - add #include <Foundation/Foundation.h> to wrappers/obj-c/OneDsCppSdk.h"
echo

read -p "Press enter to continue"

sharpie bind --output=./ -sdk iphoneos14.2 -namespace=Microsoft.Applications.Events -scope=../../../obj-c ../../../obj-c/OneDsCppSdk.h -c -xobjective-c
