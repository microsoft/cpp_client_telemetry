@echo off
pushd %~dp0
if not exist sandbox mkdir sandbox
popd

REM Invoke build-emcc.sh script from top-level directory
cd ../../../
call build-emcc.cmd

REM Uncomment this line and add your own AppInsights key to export events to A.I.
REM set APPINSIGHTS_INSTRUMENTATIONKEY=357978ba-37ed-467d-bc81-bedac8068c41
REM Start node from current directory
pushd %~dp0
node --trace-warnings --experimental-wasm-bigint --experimental-wasm-threads --experimental-wasm-bulk-memory main.js
popd
