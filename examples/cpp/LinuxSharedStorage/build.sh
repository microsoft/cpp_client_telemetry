#!/bin/sh
mkdir -p out
cd out
cmake ..
make
# Strip for release
# strip EventSender
