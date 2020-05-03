#!/bin/sh
brew install emscripten
export LLVM_ROOT=$(brew --prefix)/opt/emscripten/libexec/llvm/bin
export BINARYEN_ROOT=$(brew --prefix)/opt/binaryen
