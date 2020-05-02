'use strict';
const fs = require('fs');

const { WASI } = require('wasi');

let memory = new WebAssembly.Memory({ initial: 200 });
let native_memory = memory;

const wasi = new WASI({
  args: process.argv,
  env: process.env,
  preopens: {
    '/sandbox': './sandbox'
  },
  memory: memory.buffer
});

const log = (offset, length) => {
  const bytes = new Uint8Array(native_memory.buffer, offset, length);
  let string = new TextDecoder('utf8').decode(bytes);
  console.log(string);
};

const importObject = {
  wasi_snapshot_preview1: wasi.wasiImport,
  env: {
    console_log: log,
    memory
  }
};

(async () => {
  const wasm = await WebAssembly.compile(fs.readFileSync('./main.wasm'));
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
