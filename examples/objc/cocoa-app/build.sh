#!/bin/sh
#rm -rf out
mkdir -p out
cd out
cmake -G Xcode ..
cmake --build .
xcodebuild -list -project foo.xcodeproj
cd debug/foo.app/Contents/MacOS/
#./foo
open -a foo
#open foo.xcodeproj/
