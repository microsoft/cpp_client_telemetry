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

# dyld_info /Users/runner/work/cpp_client_telemetry/cpp_client_telemetry/out/lib/libmat.a

cd tests/unittests

xcodebuild	-scheme iOSUnitTests -showdestinations
echo 'End of xcodebuild -showdestinations'
xcodebuild -showsdks -json
echo 'End of xcodebuild -showsdks -json'
xcrun simctl list devices available
echo 'End of xcrun simctl list devices available'
xcodebuild test -scheme iOSUnitTests -destination "platform=iOS Simulator,name=$SIMULATOR" -verbose

cd ../functests
xcodebuild test -scheme iOSFuncTests -destination "platform=iOS Simulator,name=$SIMULATOR" -verbose
