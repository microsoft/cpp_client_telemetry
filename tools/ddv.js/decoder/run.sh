#!/bin/sh
mkdir -p sandbox
# Invoke build-emcc.sh script from top-level directory
( cd ../../../; ./build-emcc.sh )

# Uncomment this line and add your own AppInsights key to export events to A.I.
#export APPINSIGHTS_INSTRUMENTATIONKEY=357978ba-37ed-467d-bc81-bedac8068c41

# Start node from current directory
node --trace-warnings --experimental-wasm-bigint --experimental-wasm-threads --experimental-wasm-bulk-memory main.js
