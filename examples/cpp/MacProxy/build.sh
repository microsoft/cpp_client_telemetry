#!/bin/sh
export PATH=/usr/local/bin:$PATH
mkdir -p out
cd out
cmake ..
make
## Uncomment the following line below to strip the binary for Release:
#strip SampleCpp
