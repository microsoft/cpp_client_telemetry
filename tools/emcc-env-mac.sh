#!/bin/sh
# Default location for emscripten on Mac OS X installed with brew
export LLVM=$(brew --prefix)/opt/emscripten/libexec/llvm/bin
export BINARYEN=$(brew --prefix)/opt/binaryen
export CC=emcc

