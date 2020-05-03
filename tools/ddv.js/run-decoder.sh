#!/bin/sh
mkdir -p sandbox
#node --trace-warnings --experimental-wasi-unstable-preview1 --experimental-wasm-bigint main.js
node --trace-warnings --experimental-wasm-bigint main.js
