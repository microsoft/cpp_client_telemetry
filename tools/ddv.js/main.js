#!/usr/bin/env node

'use strict';
const fs = require('fs');

const console_log = (offset, length) => {
  const bytes = new Uint8Array(native_memory.buffer, offset, length);
  let string = new TextDecoder('utf8').decode(bytes);
  console.log(string);
};

const native_ready = () => {
  console.log("C++ code started");
};

var Module = {};
Module['noInitialRun'] = true;

const native = require('./out/main-native.js');
console.log("WebAssembly starting...");
console.log(native);
