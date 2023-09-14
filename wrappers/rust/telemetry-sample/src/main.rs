// This is a comment, and is ignored by the compiler.
// You can test this code by clicking the "Run" button over there ->
// or if you prefer to use your keyboard, you can use the "Ctrl + Enter"
// shortcut.

// This code is editable, feel free to hack it!
// You can always return to the original code by clicking the "Reset" button ->

// This is the main function.

use log::{error, Level};
use std::{thread, time::Duration};

pub const API_KEY: &str =
    "99999999999999999999999999999999-99999999-9999-9999-9999-999999999999-9999";

fn main() {
    // Setup Log Appender for the log crate.
    log::set_logger(&oneds_telemetry::appender::LOGGER).unwrap();
    log::set_max_level(Level::Debug.to_level_filter());

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
            "sdkmode":0,
            "compat": {"customTypePrefix": "compat_event"}
        }"#;

    oneds_telemetry::init(config);

    loop {
        error!(target: "app_events", "App Error: {}, Port: {}", "Connection Error", 22);
        thread::sleep(Duration::from_secs(3));
    }

    // telemetry::shutdown();
}
