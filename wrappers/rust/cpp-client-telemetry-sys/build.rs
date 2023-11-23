use std::env;
use std::fs;
use std::path::PathBuf;
use subprocess::Exec;

static PROJECT_ROOT: &str = "../../../";

fn copy_static_libs() {
    // TODO add compatibility for x86 and ARM
    let cpp_project_out = PathBuf::from(PROJECT_ROOT).join("Solutions/out/Release/x64");
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    std::fs::copy(cpp_project_out.join("win32-lib/ClientTelemetry.lib"), PathBuf::from(&out_dir).join("ClientTelemetry.lib"))
        .expect("Failed to copy ClientTelemetry lib");
    std::fs::copy(cpp_project_out.join("sqlite/sqlite.lib"), out_dir.join("sqlite.lib"))
        .expect("Failed to copy sqlite native library");
    std::fs::copy(cpp_project_out.join("zlib/zlib.lib"), out_dir.join("zlib.lib"))
        .expect("Failed to copy zlib native library");

    // Tell cargo to look for shared libraries in the specified directory
    println!("cargo:rustc-link-search={}", out_dir.display());
    println!("cargo:rustc-link-lib=ClientTelemetry");
    println!("cargo:rustc-link-lib=wininet");
    println!("cargo:rustc-link-lib=crypt32");
    println!("cargo:rustc-link-lib=sqlite");
    println!("cargo:rustc-link-lib=zlib");
}

fn write_bindings() {
    // Precompile header with the appropriate preprocessor options
    let mat_h_location = PathBuf::from(PROJECT_ROOT).join("lib/include/public/mat.h");
    println!("cargo:rerun-if-changed={}", mat_h_location.to_string_lossy());

    // TODO use clang crate instead of invoking CLI directly
    let header_out = Exec::cmd("clang")
        .arg("-E")
        .arg(mat_h_location)
        .arg("-D")
        .arg("MATSDK_STATIC_LIB=1")
        .capture()
        .expect("Failed to open clang process")
        .stdout_str();

    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let mat_out_path = out_dir.join("mat.out.h");

    fs::write(&mat_out_path, header_out).unwrap();

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
        //.clang_arg(format!("-I{}", PathBuf::from(PROJECT_ROOT).join("lib/include").display()))
        .header(PathBuf::from(out_dir).join("mat.out.h").to_string_lossy())
        .allowlist_type("evt_.*")
        .allowlist_function("evt_.*")
        .allowlist_var("evt_.*")
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
    write_bindings();
    copy_static_libs();
}
