#!/bin/sh
// Add current user
current_user=`id`
echo "CURRENT USER:" $current_user
cd ${0%/*}
SKU=${1:-release}
SIMULATOR=${2:-iPhone 8}

set -e

echo "Building build-ios.sh"
./build-ios.sh ${SKU}
echo "End of build-ios.sh"

lipo -info /Users/runner/work/cpp_client_telemetry/cpp_client_telemetry/out/lib/libmat.a

cd tests/unittests
xcodebuild test -scheme iOSUnitTests -destination "platform=iOS Simulator,name=$SIMULATOR"

cd ../functests
xcodebuild test -scheme iOSFuncTests -destination "platform=iOS Simulator,name=$SIMULATOR"
