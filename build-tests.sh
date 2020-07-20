#!/bin/sh
cd "${0%/*}"
SKU=${1:-release}
echo Building and running $SKU tests...
./build.sh ${SKU}
# Fail on test errors
set -e
cd out
./tests/functests/FuncTests
./tests/unittests/UnitTests
 ./FuncTests --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager & ./FuncTests --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager
