#!/bin/sh
./build.sh
# Fail on test errors
set -e
cd out
./tests/functests/FuncTests
./tests/unittests/UnitTests
