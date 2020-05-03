'use strict';
const fs = require('fs');

// const { WASI } = require('wasi');

const native_module = require('out/main-native.js');

const memory = new WebAssembly.Memory({ initial: 256, maximum: 256 });

let native_memory = memory;

/*
const wasi = new WASI({
  args: process.argv,
  env: process.env,
  preopens: {
    '/sandbox': './sandbox'
  },
  memory: memory.buffer
});
*/

const log = (offset, length) => {
  const bytes = new Uint8Array(native_memory.buffer, offset, length);
  let string = new TextDecoder('utf8').decode(bytes);
  console.log(string);
};

const importObject = {
  // wasi_snapshot_preview1: wasi.wasiImport,
  env: {
    abortStackOverflow: () => { throw new Error('overflow'); },
//    table: new WebAssembly.Table({ initial: 0, maximum: 0, element: 'anyfunc' }),
//    __table_base: 0,
//    tableBase: 0,
    memory: memory,
    nullFunc_ii: () => {},
    nullFunc_iidiiii: () => {},
    nullFunc_iiii: () => {},
    nullFunc_jiji: () => {},
    nullFunc_v: () => {},
    nullFunc_vii: () => {},
//    __memory_base: 1024,
//    memoryBase: 1024,
//    STACKTOP: 0,
//    STACK_MAX: memory.buffer.byteLength,
    console_log: log
  }
};

(async () => {
  const wasm = await WebAssembly.compile(fs.readFileSync('./out/main-native.wasm'));
  WebAssembly.instantiate(wasm, importObject).then(
    (instance) =>
    {
      // console.log(instance);
      native_memory = instance.exports.memory;
      // instance.env.console = console;
      //
      instance.exports.getHello();
      instance.exports.getHello();
      instance.exports.getHello();
      instance.exports.getHello();
    },
    (reason) =>
    {
      console.log(reason);
    }
  );
  // This will launch main
  // wasi.start(instance);
})();
