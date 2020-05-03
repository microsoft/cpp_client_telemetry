#!/usr/bin/env node

'use strict';
const fs = require('fs');

const native = require('./out/main-native.js');

let native_HEAPU8  = null;
let native_HEAPU16 = null;
let native_HEAPU32 = null;

let js_HEAP_size = 16384;
let js_HEAP =
{
  "ptr": null,  // native_HEAPU8 ptr
  "size": 0,    // block size
};

const EM_malloc = (size) =>
{
  return { "ptr": native._malloc(size), "size": size };
};

const sendBuffer_mock = () =>
{
  // Send some values
  let arr = new Uint8Array(native_HEAPU8.buffer, js_HEAP.ptr, js_HEAP.size)
  for(var i=0; i<127; i++)
  {
    arr[i] = i;
  }
  native._sendBuffer(js_HEAP.ptr, 127);
};

// Called when C++ main starts up
global.onEmInit = () => {
  console.log("EM C++ code starting...");
  // initialize references to Emscripten HEAP objects
  native_HEAPU8  = native.HEAPU8;
  native_HEAPU16 = native.HEAPU16;
  native_HEAPU32 = native.HEAPU32;
  // Show the memory available
  var memory = native.getMemory();
  console.log("Memory:", memory);
  // Allocate buffer of 1024 bytes to use
  // for data transfer between JS and C++
  js_HEAP = EM_malloc(js_HEAP_size);
  // send some bytes to C++
  sendBuffer_mock();
};

// Called when C++ shuts down
global.onEmDone = () => {
  console.log("EM C++ code stopping...");
};

// Pass strings from C++ to Javascript console
global.console_log = (offset, length) => {
  const bytes = new Uint8Array(native_HEAPU8.buffer, offset, length);
  let string = new TextDecoder('utf8').decode(bytes);
  console.log(string);
};

// Consider https://nodejs.org/api/globals.html#globals_queuemicrotask_callback
// for buffer passing from node.js scope to WebAssembly for decoding

console.log("WebAssembly starting...");
