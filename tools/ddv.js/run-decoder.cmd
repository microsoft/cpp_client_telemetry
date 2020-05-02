@ECHO OFF
REM CALL ..\..\emsdk\emsdk_env.bat
REM %EMSDK_NODE% --experimental-modules --experimental-wasm-modules main.mjs
REM node --trace-warnings --experimental-wasi-unstable-preview1 --experimental-wasm-bigint hello.js
node --experimental-wasi-unstable-preview1 --experimental-wasm-bigint main.js
