#!/bin/sh
export CLANG_PATH=/tools/clang+llvm-7.0.0-x86_64-apple-darwin/bin
export PATH=$CLANG_PATH:$PATH
export CC=$CLANG_PATH/clang-7
export CXX=$CLANG_PATH/clang++
source ./build.sh
