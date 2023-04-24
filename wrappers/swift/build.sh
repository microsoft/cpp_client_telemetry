#!/bin/bash
export PATH=/usr/local/bin:$PATH
mkdir out
cd out
cmake .. -G Xcode
xcodebuild -project *.xcodeproj -quiet
