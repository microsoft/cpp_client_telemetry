#!/bin/sh

set -e
build-ios.sh release

cd tests/unittests
xcodebuild test -scheme iOSUnitTests -destination 'platform=iOS Simulator,name=iPhone 7'
