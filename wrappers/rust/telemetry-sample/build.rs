use std::env;
use std::fs;
use std::path::Path;

fn main() {
    println!("cargo:rustc-link-search=../lib");
    // println!("cargo:rustc-link-search=native={}", libdir.display());

    let out_dir = env::var("OUT_DIR").unwrap();
    let src = Path::join(&env::current_dir().unwrap(), "..\\lib\\ClientTelemetry.dll");
    let dest = Path::join(Path::new(&out_dir), Path::new("ClientTelemetry.dll"));
    fs::copy(src, dest).unwrap();
}