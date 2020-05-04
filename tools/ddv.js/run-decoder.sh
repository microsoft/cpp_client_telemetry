#!/bin/sh
mkdir -p sandbox
# Invoke build-emcc.sh script from top-level directory
( cd ../../; ./build-emcc.sh )
# Start node from current directory
node --trace-warnings --experimental-wasm-bigint --experimental-wasm-threads --experimental-wasm-bulk-memory main.js
