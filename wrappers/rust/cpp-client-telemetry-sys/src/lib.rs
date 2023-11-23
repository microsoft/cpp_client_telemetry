#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::c_void;

fn evt_api_call_wrapper(evt_context: Box<evt_context_t>) -> (evt_status_t, Box<evt_context_t>) {
    let raw_pointer = Box::into_raw(evt_context);

    let mut result: evt_status_t = -1;
    unsafe {
        result = evt_api_call_default(raw_pointer);
    }

    let out_context = unsafe { Box::from_raw(raw_pointer) };
    (result, out_context)
}

fn evt_open(config: *mut c_void) -> Option<evt_handle_t> {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_OPEN,
        handle: 0,
        data: config,
        result: 0,
        size: 0,
    });

    let (result, context) = evt_api_call_wrapper(context);

    if result == -1 {
        return Option::None;
    }

    return Some(context.handle.clone());
}

pub fn evt_close(handle: &evt_handle_t) -> evt_status_t {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_CLOSE,
        handle: *handle,
        data: std::ptr::null_mut(),
        result: 0,
        size: 0,
    });

    let (result, _) = evt_api_call_wrapper(context);
    result
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
