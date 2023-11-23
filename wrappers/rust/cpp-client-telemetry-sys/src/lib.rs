#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

fn test_fn() {
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_open_close() {
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

        let result = evt_open(config.as_ptr() as *mut c_void);
        assert_eq!(result, Some(0));
    }
}
