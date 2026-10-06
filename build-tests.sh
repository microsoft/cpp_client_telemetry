#!/bin/sh
set -e

cd "${0%/*}"
SKU=${1:-release}
echo "Building and running $SKU tests..."
CMAKE_OPTS="${CMAKE_OPTS:-} -DMATSDK_BUILD_UNIT_TESTS=ON -DMATSDK_BUILD_FUNC_TESTS=ON" \
  ./build.sh "$SKU"
cd out
ctest --output-on-failure

./tests/functests/FuncTests --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager &
first_pid=$!
./tests/functests/FuncTests --gtest_filter=MultipleLogManagersTests.MultiProcessesLogManager &
second_pid=$!
status=0
wait "$first_pid" || status=$?
wait "$second_pid" || status=$?
exit "$status"
