#!/bin/sh
export LLVM=$(brew --prefix)/opt/emscripten/libexec/llvm/bin
export BINARYEN=$(brew --prefix)/opt/binaryen
mkdir -p out
emcc -O0 main.cpp -std=c++11 -o out/main-native.js -s WASM=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s EXPORTED_FUNCTIONS="['_main', '_getHello', '_sayHello']"
