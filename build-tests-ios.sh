#!/bin/sh
cd ${0%/*}
SKU=${1:-release}
SIMULATOR=${2:-iPhone 8}

set -e
./build-ios.sh ${SKU}

if [ -d "/Applications/Xcode_11.4.1.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/" ] 
then
  echo "Xcode 11.4.1 test build is not supported yet (see https://forums.developer.apple.com/thread/130684 )"
  exit 0
fi

cd tests/unittests
xcodebuild test -scheme iOSUnitTests -destination "platform=iOS Simulator,name=$SIMULATOR"

cd ../functests
xcodebuild test -scheme iOSFuncTests -destination "platform=iOS Simulator,name=$SIMULATOR"
