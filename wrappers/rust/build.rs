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
    println!("cargo:rerun-if-changed={}", headers_path_str);

    // // Run `clang` to compile the `mat.c` file into a `mat.o` object file.
    // // Unwrap if it is not possible to spawn the process.
    // if !std::process::Command::new("clang")
    //     .arg("-c")
    //     .arg("-o")
    //     .arg(&obj_path)
    //     .arg(libdir_path.join("hello.c"))
    //     .output()
    //     .expect("could not spawn `clang`")
    //     .status
    //     .success()
    // {
    //     // Panic if the command was not successful.
    //     panic!("could not compile object file");
    // }

    // // Run `ar` to generate the `libhello.a` file from the `hello.o` file.
    // // Unwrap if it is not possible to spawn the process.
    // if !std::process::Command::new("ar")
    //     .arg("rcs")
    //     .arg(lib_path)
    //     .arg(obj_path)
    //     .output()
    //     .expect("could not spawn `ar`")
    //     .status
    //     .success()
    // {
    //     // Panic if the command was not successful.
    //     panic!("could not emit library file");
    // }

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header(c_header_path_str)
        .clang_arg(format!("-I{}", headers_path_str))
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(CargoCallbacks))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap()).join("bindings.rs");
    bindings
        .write_to_file(out_path)
        .expect("Couldn't write bindings!");
}
