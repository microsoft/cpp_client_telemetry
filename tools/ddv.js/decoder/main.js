#!/usr/bin/env node

'use strict';
const fs = require('fs');
const http = require('http');
const url = require('url');

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
const decodeRequest = (contentType, contentEncoding, body) => {
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

// Invoked when decoded result is ready
global.onRequestDecoded = (buffer, size) => {
  console.log("Request decoded: ", buffer, size);
  // Print decoded body
  console.log(native.UTF8ToString(buffer, size));
};

// Called when C++ shuts down
global.onEmDone = () => {
  console.log("EM C++ code stopping...");
};

// Pass strings from C++ to Javascript console
global.console_log = (offset, length) => {
  // TODO: can also use UTF8ToString
  const bytes = new Uint8Array(native_HEAPU8.buffer, offset, length);
  let string = new TextDecoder('utf8').decode(bytes);
  console.log(string);
};

// Consider https://nodejs.org/api/globals.html#globals_queuemicrotask_callback
// for buffer passing from node.js scope to WebAssembly for decoding
console.log("WebAssembly starting...");

// Periodic heartbeat in Javascript
var seq = 0;
const heartbeat = () => {
  // Uncomment to debug JS hearbeats:
  // console.log("JS heartbeat:", seq);
  seq++;
/*
  var arr = new Uint8Array([21, 31]);
  // This may synchronously call onRequestDecoded
  decodeRequest("bond/compact-binary", "gzip", arr);
 */
  setTimeout(heartbeat, 1000);
};

// Invoked async after onEmInit 
const onEmReady = () => {
  console.log("onEmReady");
  heartbeat();
}

/************************************************************************************************/
/* Refactor this into module                                                                    */
/************************************************************************************************/
function console_log_time(arg) {
  console.log(new Date(), arg);
}

/**
 * Create HTTP server object
 */
const httpServerListener = (request, response) => {
  const q = url.parse(request.url, true).query;
  let pathname = url.parse(request.url).pathname;
  console_log_time(request.method + " " + pathname);
  switch (pathname) {
    case "/OneCollector/1.0/":
      // TODO: verify that the content-type is matching what we can decode
      let content_length = request.headers['content-length']
      console_log_time("Content-Length: "+content_length);

      let content_type = request.headers['content-type'];
      let content_encoding = request.headers['content-encoding'];
      if (typeof request.body === 'undefined')
      {
        request.body = [];
      };
  
      request.on('error', (err) => {
        console_log_time("error: "+err);
      }).
      on('data', (chunk) => {
        console_log_time("body parts: "+request.body.length);
        console_log_time("+new chunk: "+chunk.length+" bytes");
        request.body.push(chunk);
      }).
      on('end', () => {
        // This may synchronously call onRequestDecoded
        let buffer = Buffer.concat(request.body);
        console_log_time("Body size: "+buffer.length);
        // console.log(buffer.toString('base64'));
        decodeRequest(content_type, content_encoding, buffer);
        response.writeHead(200, {"Content-Type": "text/plain"});
        response.write("OK");
        response.end();
      });
      break;
    default:
      break;
  }
};

const httpServer = http.createServer(httpServerListener);

httpServer.on('checkContinue', function(req, res) {
  console_log_time('Expect: 100-continue');
  res.writeContinue();
  setTimeout(function() { httpServerListener(req, res); }, 100);
});

/**
 * Start listening to HTTP connections on port 8081
 */
httpServer.listen(8081, function () {
  console_log_time('http server is listening on port 8081');
});