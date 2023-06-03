#!/bin/bash

export PATH=/usr/local/bin:$PATH
mkdir out
cd out
cmake .. -G Xcode # Generate swift project files for xcode
xcodebuild -quiet
