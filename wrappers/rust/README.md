Steps
-----

1. Build 1DS C++ SDK as a shared library.
2. Intall rust.
3. Use 'bindgen' to generate the wrapper.

From the root folder of the build tree:

```
BUILD_SHARED_LIBS=ON ./build.sh
cd wrappers/rust
./install-rust.sh
./build.sh
```

