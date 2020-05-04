REM Using prebuilt wasm on Windows to avoid installing the tools
node --trace-warnings --experimental-wasm-bigint --experimental-wasm-threads --experimental-wasm-bulk-memory main.js
