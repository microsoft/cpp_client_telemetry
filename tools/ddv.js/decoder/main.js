#!/usr/bin/env node
// (c) 2020 Microsoft Corporation. All rights reserved.
//
// Standalone implementation of Common Schema/Bond decoder.
//
// Author: Max Golovanov <maxgolov@microsoft.com>
//
'use strict';
const fs = require('fs');
const http = require('http');
const url = require('url');

// TOTALLY OPTIONAL, BUT LOOKS NICE!
const colorize = require('json-colorizer');

// Pointers to Emscripten heap are populated when 'native' Module is run
let native_HEAPU8 = null;
let native_HEAPU16 = null;
let native_HEAPU32 = null;

// Called when C++ main starts up
global.onEmInit = () => {
  console.log("EM C++ code starting...");

  // initialize references to Emscripten HEAP objects
  native_HEAPU8 = native.HEAPU8;
  native_HEAPU16 = native.HEAPU16;
  native_HEAPU32 = native.HEAPU32;

  // Show the memory available
  console.log("Memory used:", native.getMemory());

  onEmReady();
};

// TODO: binplace decoder.js and decoder.wasm to same dir as main.js
const native = require('./build/decoder.js');

// Allocate block on EM HEAP
const malloc = (size) => { return { "ptr": native._malloc(size), "size": size }; };

// Free block on EM HEAP
const free = (block) => { native._free(block.ptr); };

// strdup jsString on EM HEAP, return block ptr and size
const strdup = (jsString) => {
  let maxlen = native.lengthBytesUTF8(jsString) + 1;
  let block = malloc(maxlen);
  native.stringToUTF8(jsString, block.ptr, maxlen);
  return block;
}

// Accept encoded request Content-Type, Content-Encoding and Body.
// Assume that we do not require other non-standard headers right now.
// Pass to decoder and wait for onRequestDecoded (potentially async)
// notification. This code may benefit from refactoring it into Promise
function decodeRequest(contentType, contentEncoding, body) {
  let emContentType = strdup(contentType);
  let emContentEncoding = strdup(contentEncoding);
  let block = malloc(body.length);
  // Unfortunately we require a memcpy here because there's no way
  // to pass Javascript variable to Emscripten heap...
  // Unless we optimize this to always store request body on heap.
  new Uint8Array(native_HEAPU8.buffer, block.ptr, block.size).set(body);
  native._decodeRequest(emContentType.ptr, emContentEncoding.ptr, block.ptr, block.size);
  // Free 'native' HEAP
  free(block);
  free(emContentType);
  free(emContentEncoding);
};

global.decodeRequest = decodeRequest;

let onRequestDecodedCallback = null;

//
// Request decoder callback receives UTF-8 string that contains JSON array (package) of JSON objects (events)
//
const registerOnRequestDecoded = (handler) =>
{
  onRequestDecodedCallback = handler;
};

// Invoked when decoded result is ready
global.onRequestDecoded = (buffer, size) => {
  console.log("Request decoded: ", buffer, size);
  if (onRequestDecodedCallback != null)
  {
    onRequestDecodedCallback(native.UTF8ToString(buffer, size));
  }
};

//
// Called when C++ shuts down
//
global.onEmDone = () => {
  console.log("EM C++ code stopping...");
};

//
// Pass strings from C++ to Javascript console
//
global.console_log = (offset, length) => {
  // TODO: can also use UTF8ToString
  const bytes = new Uint8Array(native_HEAPU8.buffer, offset, length);
  let string = new TextDecoder('utf8').decode(bytes);
  console.log(string);
};

// Consider https://nodejs.org/api/globals.html#globals_queuemicrotask_callback
// for buffer passing from node.js scope to WebAssembly for decoding
console.log("WebAssembly starting...");

//
// Periodic heartbeat in Javascript
//
var seq = 0;
const heartbeat = () => {
  // Uncomment to debug JS hearbeats:
  // console.log("JS heartbeat:", seq);
  seq++;
  setTimeout(heartbeat, 1000);
};

//
// Invoked async after onEmInit
//
const onEmReady = () => {
  console.log("onEmReady");
  heartbeat();
}

const server = require('./server.js');

//
// TODO: respect params
//
const startServer = (params) =>
{
  server.startServer();
};

module.exports = {

  decodeRequest,            // decode request into UTF-8 JSON string

  registerOnRequestDecoded, // register callback to invoke when the string is ready

  startServer,              // start our internal OneCollector server

  getRequestListner:        // obtain implementation of request listener:
    function()
    {
      return server.getRequestListener;
    }
};

//
// Simple example that illustrates how to use the module
//
if (require.main === module) {
  // register decoder callback
  registerOnRequestDecoded((json) =>
  {
    console.log(colorize(json));
  });
  // Start our own standalone server
  startServer();
}
