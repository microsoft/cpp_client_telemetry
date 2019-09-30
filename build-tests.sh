#!/bin/sh
./build.sh
cd out
./tests/functests/FuncTests
./tests/unittests/UnitTests
