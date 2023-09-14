extern crate bindgen;

use std::fmt::format;
use std::fs;
use std::env;
use std::path::PathBuf;

use bindgen::CargoCallbacks;

fn main() {
    let rust_wrapper_path = env::current_dir().unwrap();
    let libdir_path = rust_wrapper_path
        .join("..\\..\\lib")
        .canonicalize()
        .expect("cannot canonicalize lib path");

    // This is the path to the `c` headers file.
    let headers_path = libdir_path.join("include\\public");
    let headers_path_str = headers_path.to_str().expect("Path is not a valid string");
    let c_header_path = libdir_path.join("include\\public\\mat.h");
    let c_header_path_str = c_header_path.to_str().expect("Path is not a valid string");

    // This is the path to the intermediate object file for our library.
    let obj_path = libdir_path.join("mat.o");
    // This is the path to the static library file.
    let lib_path = libdir_path.join("mat.a");

    // Tell cargo to look for shared libraries in the specified directory
    // println!("cargo:rustc-link-search={}", libdir_path.to_str().unwrap());

    // Tell cargo to tell rustc to link our `hello` library. Cargo will
    // automatically know it must look for a `libmat.a` file.
    // println!("cargo:rustc-link-lib=mat");

    // Tell cargo to invalidate the built crate whenever the header changes.
    // println!("cargo:rerun-if-changed={}", headers_path_str);

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header(c_header_path_str)
        .clang_arg(format!("-I{}", headers_path_str))
        // https://github.com/Rust-SDL2/rust-sdl2/issues/1288
        .blocklist_type("IMAGE_TLS_DIRECTORY")
        .blocklist_type("PIMAGE_TLS_DIRECTORY")
        .blocklist_type("IMAGE_TLS_DIRECTORY64")
        .blocklist_type("PIMAGE_TLS_DIRECTORY64")
        .blocklist_type("_IMAGE_TLS_DIRECTORY64")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    // Write the bindings to the $CARGO_MANIFEST_DIR/bindings.rs file.
    // https://doc.rust-lang.org/cargo/reference/environment-variables.html#environment-variables-cargo-sets-for-build-scripts
    let out_path = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap()).join("src/bindings.rs");
    bindings
        .write_to_file(out_path)
        .expect("Couldn't write bindings!");
}
