#!/bin/sh
rm -rf out; mkdir -p out; cd out
cmake .. && make
