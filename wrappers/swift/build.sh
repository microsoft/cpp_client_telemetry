#!/bin/bash

export PATH=/usr/local/bin:$PATH

BUILD_TYPE="${1:-debug}"

if [ "$2" == "clean" ]; then
    # Clean the output folders
    echo "Cleaning output folders..."
    swift package clean
    rm -Rf out
    rm -Rf ./build
fi

# Generate the build files and run build
mkdir out
cd out
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..  -G Ninja # Generate swift project files for xcode
