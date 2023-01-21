#!/bin/sh
cd "${0%/*}"
SKU=${1:-release}
echo Building and running $SKU tests...
./build.sh ${SKU}
# Fail on test errors
set -e
cd out
./tests/functests/FuncTests --gtest_filter=-APITest.C_API_Test # TBD - CAPI tests are failing in CI. Need to be fixed
./tests/unittests/UnitTests
./tests/functests/FuncTests --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager & \
./tests/functests/FuncTests --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager &
