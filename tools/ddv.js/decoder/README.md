# Building Protocol Decoder for WebAssembly

This project contains the files needed to build 1DS C++ SDK Bond protocol decoder for
Javascript sandbox environments, such as web browsers and node. Possible scenarios are
embedding of decoder in cross-platform Javascript, node, browser extensions Electron,
React apps without native code dependency.

# Required dependencies to build

- cmake
- llvm clang toolchain
- emscripten
- knowledge of how C++ compilers and cmake works
- be prepared for a failure.. and know how to overcome it

# How to build

- Nativate to workspace root folder of the git tree
- Install Emscriptent by running 'tools/setup-emsdk*' script for your platform
- Then run *build-emmc.sh* to build all Emscriptent targets, including Decoder

# Output

Resulting output consists of two files:
- build/decoder.js     - bootsrap launcher for node
- build/decoder.wasm   - web assembly module

Launch it as:

```
node decoder.js
```

# Additional scripts

In order to debug the tool 'locally' without emscripten cross-compiling:
- build-gcc-mac.sh     - compile the tool with brew gcc on Mac OS X
- build.sh             - build with the platform-standard compiler (gcc or clang)
