#!/bin/sh
cd ${0%/*}
SKU=${1:-release}
SIMULATOR=${2:-iPhone 8}

set -e
./build-ios.sh ${SKU}

cd tests/unittests
xcodebuild test -scheme iOSUnitTests -destination "platform=iOS Simulator,name=$SIMULATOR"
