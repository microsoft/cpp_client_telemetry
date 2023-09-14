use std::env;
use std::path::PathBuf;

fn main() {
    // Tell cargo to look for shared libraries in the specified directory
    println!("cargo:rustc-link-search=../lib");

    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    // println!("cargo:rustc-link-static-lib=mat");

    // Tell cargo to invalidate the built crate whenever the wrapper changes
    // println!("cargo:rerun-if-changed=../include");

    // #![allow(non_upper_case_globals)]
    // #![allow(non_camel_case_types)]
    // #![allow(non_snake_case)]

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
        .header("../include/mat.out.h")
        .allowlist_file("../include/mat.out.h")
        .c_naming(false)
        .blocklist_type("struct_IMAGE_TLS_DIRECTORY")
        .blocklist_type("struct_PIMAGE_TLS_DIRECTORY")
        .blocklist_type("struct_IMAGE_TLS_DIRECTORY64")
        .blocklist_type("struct_PIMAGE_TLS_DIRECTORY64")
        .blocklist_type("struct__IMAGE_TLS_DIRECTORY64")
        .blocklist_type("IMAGE_TLS_DIRECTORY")
        .blocklist_type("PIMAGE_TLS_DIRECTORY")
        .blocklist_type("IMAGE_TLS_DIRECTORY64")
        .blocklist_type("PIMAGE_TLS_DIRECTORY64")
        .blocklist_type("_IMAGE_TLS_DIRECTORY64")
        .allowlist_type("evt_.*")
        .allowlist_function("evt_.*")
        .allowlist_var("evt_.*")
        .allowlist_recursively(false)
        .layout_tests(false)
        .merge_extern_blocks(true)
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