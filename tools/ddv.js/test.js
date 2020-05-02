const iw = require('inline-webassembly');

const memory = new WebAssembly.Memory({ initial: 1 });

const log = (offset, length) => {
  const bytes = new Uint8Array(memory.buffer, offset, length);
  const string = new TextDecoder('utf8').decode(bytes);

  console.log(string);
};

(async () => {
  const wasm = await iw(`
  (module
    (import "env" "memory" (memory 1))
    (import "env" "log" (func $log (param i32 i32)))

    (data (i32.const 0) "Hello, World!")

    (func (export "hello")
      i32.const 0
      i32.const 13
      call $log
    )
  )`, {
    env: { log, memory }
  });

  wasm.hello();
})();
