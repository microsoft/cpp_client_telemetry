@ECHO OFF
REM CALL ..\..\emsdk\emsdk_env.bat
REM %EMSDK_NODE% --experimental-modules --experimental-wasm-modules main.mjs
node --experimental-modules --experimental-wasm-modules main.mjs
REM node main.out.js
