// This is a comment, and is ignored by the compiler.
// You can test this code by clicking the "Run" button over there ->
// or if you prefer to use your keyboard, you can use the "Ctrl + Enter"
// shortcut.

// This code is editable, feel free to hack it!
// You can always return to the original code by clicking the "Reset" button ->

// This is the main function.

use std::{
    ffi::{CStr, CString},
    mem, thread,
    time::Duration,
};
use telemetry::LogManager;

fn string_prop(name: &str, value: &str, is_pii: bool) -> telemetry::evt_prop {
    let mut pii_kind = 0;

    if is_pii {
        pii_kind = 1;
    }

    telemetry::evt_prop {
        name: to_c_str2(name),
        type_: telemetry::evt_prop_t_TYPE_STRING,
        value: telemetry::evt_prop_v {
            as_string: to_c_str2(value),
        },
        piiKind: pii_kind,
    }
}

fn to_c_str(value: &str) -> *const i8 {
    let c_str = CString::new(value.clone().as_bytes()).unwrap();
    let c_str_bytes = c_str.as_bytes_with_nul().to_owned();
    let ptr = c_str_bytes.as_ptr() as *const i8;

    return ptr;
}

fn to_c_str2(value: &str) -> *const i8 {
    let c_str = Box::new(CString::new(value.clone()).unwrap());
    let ptr = c_str.as_ptr() as *const i8;
    mem::forget(c_str);
    return ptr;
}

fn int_prop(name: &str, value: i64) -> telemetry::evt_prop {
    telemetry::evt_prop {
        name: to_c_str2(name),
        type_: telemetry::evt_prop_t_TYPE_INT64,
        value: telemetry::evt_prop_v {
            as_int64: value.clone(),
        },
        piiKind: 0,
    }
}

pub const API_KEY: &str =
    "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999";

fn main() {
    let mut log_manager = LogManager::LogManager::new();

    if log_manager.start() == false {
        println!("Failed to Start Log Manager.");
    }

    log_manager.stop();

    let config = r#"{
            "eventCollectorUri": "http://localhost:64099/OneCollector/track",
            "cacheFilePath":"offline_storage.db",
            "config":{"host": "*"},
            "name":"Rust-API-Client-0",
            "version":"1.0.0",
            "primaryToken":"99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999",
            "maxTeardownUploadTimeInSec":5,
            "hostMode":false,
            "traceLevelMask": 4294967295,
            "minimumTraceLevel":0,
            "sdkmode":3
        }"#;

    let mut event: Vec<telemetry::evt_prop> = Vec::new();
    event.push(string_prop("name", "Event.Name.RustFFI", false));
    event.push(string_prop("ver", "4.0", false));
    event.push(string_prop("time", "1979-08-12", false));
    event.push(int_prop("popSample", 100));
    event.push(string_prop("iKey", API_KEY, false));
    event.push(int_prop("flags", 0xffffffff));
    event.push(string_prop("cV", "12345", false));

    println!("Testing C -> RUST FFI ...");
    let handle = telemetry::evt_open(config);
    println!("EVT_HANDLE: {}", handle);

    loop {
        let mut event_data: [telemetry::evt_prop; 7] = [
            string_prop("name", "Event.Name.RustFFI", false),
            string_prop("ver", "4.0", false),
            string_prop("time", "1979-08-12", false),
            int_prop("popSample", 100),
            string_prop("iKey", API_KEY, false),
            int_prop("flags", 0xffffffff),
            string_prop("cV", "12345", false),
        ];

        println!("EVT_LOG: {}", telemetry::evt_log(&handle, &mut event_data));
        println!("EVT_FLUSH: {}", telemetry::evt_flush(&handle));
        println!("EVT_UPLOAD: {}", telemetry::evt_upload(&handle));
        thread::sleep(Duration::from_secs(3));
    }

    // println!("EVT_UPLOAD: {}", telemetry::evt_upload(&handle));
    println!("EVT_CLOSE: {}", telemetry::evt_close(&handle));
}
