#!/bin/sh
export PATH=/usr/local/bin:$PATH
mkdir -p build
cd build
cmake ..
make
