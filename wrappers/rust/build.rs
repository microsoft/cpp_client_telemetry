extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    let project_dir    = env::var("CARGO_MANIFEST_DIR").unwrap();
    let usr_local_lib = "/usr/local/lib";
    let include_dir   = "/usr/local/include/mat/";
    let usr_lib = "/usr/lib/arm-linux-gnueabihf/";

    // Tell cargo to tell rustc to link the shared libraries
    println!("cargo:rustc-link-search={}", usr_local_lib); // the "-L" flag
    println!("cargo:rustc-link-search={}", project_dir); // the "-L" flag
    println!("cargo:rustc-link-search={}", usr_lib);

    // 3rd party deps
    println!("cargo:rustc-link-lib=mat");
    //println!("cargo:rustc-link-lib=curl");
    //println!("cargo:rustc-link-lib=z");    // the "-l" flag
    //println!("cargo:rustc-link-lib=atomic");    // the "-l" flag
    //println!("cargo:rustc-link-lib=pthread");    // the "-l" flag
    //println!("cargo:rustc-link-lib=dl");    // the "-l" flag
    //println!("cargo:rustc-link-lib=stdc++");    // the "-l" flag

    // 1DS C++ SDK
//    println!("cargo:rustc-link-lib=static=mat");
    println!("cargo:rustc-link-lib=mat");

//  .header(include_dir.to_owned() + "mat.h")

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header(include_dir.to_owned() + "mat.h")
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}