use std::env;
use std::path::PathBuf;

static PROJECT_ROOT: &str = "../../../";

fn write_bindings() {
    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .rust_target(bindgen::RustTarget::Stable_1_68)
        // .raw_line("#![allow(non_upper_case_globals)]")
        // .raw_line("#![allow(non_camel_case_types)]")
        // .raw_line("#![allow(non_snake_case)]")
        .clang_arg(format!("-I{}", PathBuf::from(PROJECT_ROOT).join("lib/include").display()))
        .header("./include/wrapper.hpp")
        //.enable_cxx_namespaces()
        .allowlist_type("Microsoft::Applications::Events::LogManagerProvider")
        .allowlist_recursively(true)
        // STL types must be marked as 'opaque' as bindgen can't handle the internals of these types.
        .opaque_type("std::(.*)")
        //.blocklist_function("std::*")
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        // .wrap_static_fns(true)
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}

fn main() {
    let mat_h_location = PathBuf::from(PROJECT_ROOT).join("lib/include/public/mat.h");
    println!("cargo:rerun-if-changed={}", mat_h_location.to_string_lossy());

    let out_dir = env::var("OUT_DIR").unwrap();
    std::fs::copy("../lib/ClientTelemetry.lib", PathBuf::from(&out_dir).join("ClientTelemetry.lib"))
        .expect("Failed to copy native ClientTelemetry lib");

    // Tell cargo to look for shared libraries in the specified directory
    println!("cargo:rustc-link-search=native={}", out_dir);
    println!("cargo:rustc-link-lib=ClientTelemetry");

    // // TODO use clang crate instead of invoking CLI directly
    // let header_out = Exec::cmd("clang")
    //     .arg("-E")
    //     .arg(mat_h_location)
    //     .arg("-D")
    //     .arg("HAVE_DYNAMIC_C_LIB")
    //     .capture()
    //     .expect("Failed to open clang process")
    //     .stdout_str();

    // let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    // let mat_out_path = out_dir.join("mat.out.h");

    // fs::write(&mat_out_path, header_out).unwrap();

    write_bindings();
}
