#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

pub mod LogManager;
mod LoadLibrary;
use std::{mem, os::raw::c_void};

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

type evt_api_call_t = extern "stdcall" fn(
    context: *mut evt_context_t
) -> evt_status_t;

pub fn evt_open(config: &str) -> evt_handle_t {
    let config_string = String::from(config);
    let config_bytes = config_string.into_bytes();
    let mut c_chars: Vec<i8> = config_bytes.iter().map(| c | *c as i8).collect::<Vec<i8>>();
    c_chars.push(0); // null terminator
    
    let config_data = c_chars.as_mut_ptr() as *mut c_void;
    let mut context: Box<evt_context_t> = Box::new(evt_context_t { 
        call: evt_call_t_EVT_OP_OPEN, 
        handle: 0, 
        data: config_data, 
        result: 0, 
        size: 0 
    });

    let raw_pointer = Box::into_raw(context);
    
    let ct_lib = LoadLibrary::Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    evt_api_call_default(raw_pointer);        
    context = unsafe { Box::from_raw(raw_pointer) };

    return context.handle.clone();
}

pub fn evt_close(handle: &evt_handle_t) -> evt_status_t {
    let mut c_chars: Vec<i8> = Vec::new();
    c_chars.push(0); // null
    let null_config_data = c_chars.as_mut_ptr() as *mut c_void;

    let mut context: Box<evt_context_t> = Box::new(evt_context_t { 
        call: evt_call_t_EVT_OP_FLUSHANDTEARDOWN, 
        handle: *handle, 
        data: null_config_data,
        result: 0, 
        size: 0 
    });

    let raw_pointer = Box::into_raw(context);
    
    let ct_lib = LoadLibrary::Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}

pub fn evt_upload(handle: &evt_handle_t) -> evt_status_t {
    let mut c_chars: Vec<i8> = Vec::new();
    c_chars.push(0); // null
    let null_config_data = c_chars.as_mut_ptr() as *mut c_void;

    let mut context: Box<evt_context_t> = Box::new(evt_context_t { 
        call: evt_call_t_EVT_OP_UPLOAD, 
        handle: *handle, 
        data: null_config_data,
        result: 0, 
        size: 0 
    });

    let raw_pointer = Box::into_raw(context);
    
    let ct_lib = LoadLibrary::Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}

pub fn evt_flush(handle: &evt_handle_t) -> evt_status_t {
    let mut c_chars: Vec<i8> = Vec::new();
    c_chars.push(0); // null
    let null_config_data = c_chars.as_mut_ptr() as *mut c_void;

    let mut context: Box<evt_context_t> = Box::new(evt_context_t { 
        call: evt_call_t_EVT_OP_FLUSH, 
        handle: *handle, 
        data: null_config_data,
        result: 0, 
        size: 0 
    });

    let raw_pointer = Box::into_raw(context);
    
    let ct_lib = LoadLibrary::Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}

pub fn evt_log_vec(handle: &evt_handle_t, data: &mut Vec<evt_prop>) -> evt_status_t {
    data.shrink_to_fit();
    assert!(data.len() == data.capacity());
    let data_pointer = data.as_mut_ptr()  as *mut c_void;
    let data_len = data.len() as u32;
    mem::forget(data); // prevent deallocation in Rust
                       // The array is still there but no Rust object
                       // feels responsible. We only have ptr/len now
                       // to reach it.

    println!("LOG LEN: {}", data_len);

    let mut context: Box<evt_context_t> = Box::new(evt_context_t { 
        call: evt_call_t_EVT_OP_LOG, 
        handle: *handle, 
        data: data_pointer,
        result: 0, 
        size: data_len
    });

    let raw_pointer = Box::into_raw(context);
    
    let ct_lib = LoadLibrary::Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}

pub fn evt_log(handle: &evt_handle_t, data: &mut [evt_prop]) -> evt_status_t {
    let data_len = data.len() as u32;
    let data_pointer = data.as_mut_ptr()  as *mut c_void;

    let mut context: Box<evt_context_t> = Box::new(evt_context_t { 
        call: evt_call_t_EVT_OP_LOG, 
        handle: *handle, 
        data: data_pointer,
        result: 0, 
        size: data_len
    });

    let raw_pointer = Box::into_raw(context);
    
    let ct_lib = LoadLibrary::Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}