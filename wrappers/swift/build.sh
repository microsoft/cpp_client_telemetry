#!/bin/bash

export PATH=/usr/local/bin:$PATH

# Build flavor
BUILD_TYPE="debug"

if [ "$1" == "release" ]; then
    BUILD_TYPE="$1"
fi

if [ "$1" == "clean" ] || [ "$2" == "clean" ]; then
    # Clean the output folders
    echo "Cleaning output folders..."
    swift package clean
    rm -Rf out
    rm -Rf .build
fi

# Generate the build files and run build
mkdir -p out/StaticLibs
cd out
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..  -G Ninja # Generate swift project files for ninja
