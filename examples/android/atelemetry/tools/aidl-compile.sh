#!/bin/bash
#
# This tool requires aidl-cpp
#
mkdir -p gen/include
aidl-cpp com/microsoft/telemetry/ITelemetryAgent.aidl gen/include gen/include/Stubs.h
find gen/ -iname *.h -o -iname *.cpp | xargs clang-format -i
