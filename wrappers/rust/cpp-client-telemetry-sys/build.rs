use std::env;
use std::fs;
use std::path::PathBuf;
use subprocess::Exec;

static PROJECT_ROOT: &str = "../../../";

fn write_bindings(header_path: &PathBuf) {
    let header_path_string = String::from(header_path.to_string_lossy());

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
        .header(&header_path_string)
        .allowlist_file(&header_path_string)
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

fn main() {
    let mat_h_location = PathBuf::from(PROJECT_ROOT).join("lib/include/public/mat.h");
    println!("cargo:rerun-if-changed={}", mat_h_location.to_string_lossy());

    let out_dir = env::var("OUT_DIR").unwrap();
    std::fs::copy("../lib/ClientTelemetry.lib", PathBuf::from(&out_dir).join("ClientTelemetry.lib"))
        .expect("Failed to copy native ClientTelemetry lib");

    // Tell cargo to look for shared libraries in the specified directory
    println!("cargo:rustc-link-search=native={}", out_dir);
    println!("cargo:rustc-link-lib=ClientTelemetry");

    // TODO use clang crate instead of invoking CLI directly
    let header_out = Exec::cmd("clang")
        .arg("-E")
        .arg(mat_h_location)
        .arg("-D")
        .arg("HAVE_DYNAMIC_C_LIB")
        .capture()
        .expect("Failed to open clang process")
        .stdout_str();

    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let mat_out_path = out_dir.join("mat.out.h");

    fs::write(&mat_out_path, header_out).unwrap();

    write_bindings(&mat_out_path);
}
