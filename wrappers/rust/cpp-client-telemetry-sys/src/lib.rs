#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::*;

fn evt_api_call_wrapper(evt_context: Box<evt_context_t>) -> (evt_status_t, Box<evt_context_t>) {
    let raw_pointer = Box::into_raw(evt_context);

    let mut result: evt_status_t = -1;
    unsafe {
        result = evt_api_call_default(raw_pointer);
    }

    let out_context = unsafe { Box::from_raw(raw_pointer) };
    (result, out_context)
}

pub fn evt_open(config: CString) -> Option<evt_handle_t> {
    let config_bytes = config.to_bytes_with_nul().to_vec();

    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_OPEN,
        handle: 0,
        data: config_bytes.as_ptr() as *mut c_void,
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

pub fn evt_upload(handle: &evt_handle_t) -> evt_status_t {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_UPLOAD,
        handle: *handle,
        data: std::ptr::null_mut(),
        result: 0,
        size: 0,
    });

    evt_api_call_wrapper(context).0
}

pub fn evt_flush(handle: &evt_handle_t) -> evt_status_t {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_FLUSH,
        handle: *handle,
        data: std::ptr::null_mut(),
        result: 0,
        size: 0,
    });

    evt_api_call_wrapper(context).0
}

pub fn evt_log(handle: &evt_handle_t, data: &mut [evt_prop]) -> evt_status_t {
    let data_len = data.len() as u32;
    let data_pointer = data.as_mut_ptr() as *mut c_void;

    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_LOG,
        handle: *handle,
        data: data_pointer,
        result: 0,
        size: data_len,
    });

    evt_api_call_wrapper(context).0
}
