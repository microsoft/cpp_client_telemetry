#!/bin/bash

# Build first
./build.sh

# Start node.js test server
export PATH=./out:$PATH

ariasysinfo
