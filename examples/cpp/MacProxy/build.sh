#!/bin/sh
export PATH=/usr/local/bin:$PATH
mkdir -p out
cd out
cmake ..
make
# Strip for release
# strip SampleCpp
