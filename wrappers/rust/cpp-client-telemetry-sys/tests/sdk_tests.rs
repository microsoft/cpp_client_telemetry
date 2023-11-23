use cpp_client_telemetry_sys::*;
use std::ffi::CString;

#[test]
fn test_open_close() {
    // Simple sanity check to ensure the SDK was linked correctly.
    let config = r#"{
        "eventCollectorUri": "http://localhost:64099/OneCollector/track",
        "cacheFilePath":"hackathon_storage.db",
        "config":{"host": "*"},
        "name":"Rust-API-Client-0",
        "version":"1.0.0",
        "primaryToken":"99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999",
        "maxTeardownUploadTimeInSec":5,
        "hostMode":false,
        "traceLevelMask": 4294967295,
        "minimumTraceLevel":0,
        "sdkmode":0,
        "compat": {"customTypePrefix": "compat_event"}
    }"#;

    let handle = evt_open(CString::new(config).unwrap()).expect("Failed to get SDK handle");

    assert_eq!(evt_close(&handle), 0);
}