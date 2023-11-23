#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::{ffi::{c_void}, os::raw};

fn evt_api_call_wrapper(context_ptr: *mut evt_context_t) -> Option<evt_handle_t> {
    let mut result = -1;
    unsafe {
        if let Some(cb) = evt_api_call {
            result = cb(context_ptr);
        }
    }

    if result == -1 {
        return Option::None;
    }

    // Box is now responsible for destroying the context
    let context = unsafe { Box::from_raw(context_ptr) };

    Some(context.handle.clone())
}

pub fn evt_open(config: *mut c_void) -> Option<evt_handle_t> {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_OPEN,
        handle: 0,
        data: config,
        result: 0,
        size: 0,
    });

    evt_api_call_wrapper(Box::into_raw(context))
}

pub fn evt_close(handle: &evt_handle_t) -> evt_status_t {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_CLOSE,
        handle: *handle,
        data: std::ptr::null_mut(),
        result: 0,
        size: 0,
    });

    let raw_pointer = Box::into_raw(context);

    let mut result: evt_status_t = -1;
    unsafe {
        if let Some(cb) = evt_api_call {
            result = cb(raw_pointer);
            let _context = Box::from_raw(raw_pointer);
        }
    }

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
    }
}
