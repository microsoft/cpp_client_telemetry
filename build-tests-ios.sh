#!/bin/sh
// Add current user
current_user=`id`
echo "CURRENT USER:" $current_user
cd ${0%/*}
SKU=${1:-release}
SIMULATOR=${2:-iPhone 8}

set -e

./build-ios.sh ${SKU}

# dyld_info /Users/runner/work/cpp_client_telemetry/cpp_client_telemetry/out/lib/libmat.a

cd tests/unittests

xcrun simctl list devices available
echo 'End of xcrun simctl list devices available'

xcodebuild test -scheme iOSUnitTests -destination "platform=iOS Simulator,name=$SIMULATOR"

cd ../functests
xcodebuild test -scheme iOSFuncTests -destination "platform=iOS Simulator,name=$SIMULATOR"
