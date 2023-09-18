use std::{ffi::CStr, mem, os::raw::c_void};

use crate::{client_library::Library, *};

type evt_api_call_t = extern "stdcall" fn(context: *mut evt_context_t) -> evt_status_t;

pub fn evt_open(config: *mut c_void) -> Option<evt_handle_t> {
    let mut context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_OPEN,
        handle: 0,
        data: config,
        result: 0,
        size: 0,
    });

    let raw_pointer = Box::into_raw(context);

    let ct_lib = Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t =
        unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    let result = evt_api_call_default(raw_pointer);

    if result == -1 {
        return Option::None;
    }

    context = unsafe { Box::from_raw(raw_pointer) };

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

    let raw_pointer = Box::into_raw(context);

    let ct_lib = Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t =
        unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}

pub fn evt_upload(handle: &evt_handle_t) -> evt_status_t {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_UPLOAD,
        handle: *handle,
        data: std::ptr::null_mut(),
        result: 0,
        size: 0,
    });

    let raw_pointer = Box::into_raw(context);

    let ct_lib = Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t =
        unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    let status = evt_api_call_default(raw_pointer);

    println!("UPLOAD STATUS >> {}", status);

    status
}

pub fn evt_flush(handle: &evt_handle_t) -> evt_status_t {
    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_FLUSH,
        handle: *handle,
        data: std::ptr::null_mut(),
        result: 0,
        size: 0,
    });

    let raw_pointer = Box::into_raw(context);

    let ct_lib = Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t =
        unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    let status = evt_api_call_default(raw_pointer);

    println!("FLUSH STATUS >> {}", status);

    status
}

pub fn evt_log_vec(handle: &evt_handle_t, data_box: &mut Box<Vec<evt_prop>>) -> evt_status_t {
    // Extract the inner vector from the Box
    let data = &mut **data_box;

    let val = unsafe { CStr::from_ptr(data[0].name) };
    let val2 = unsafe { CStr::from_ptr(data[0].value.as_string) };
    println!(
        "MAGIC >> [{}]:{}",
        val.to_str().unwrap(),
        val2.to_str().unwrap()
    );

    let data_pointer = data.as_mut_ptr() as *mut c_void;
    let data_len = data.len() as u32;

    let context: Box<evt_context_t> = Box::new(evt_context_t {
        call: evt_call_t_EVT_OP_LOG,
        handle: *handle,
        data: data_pointer,
        result: 0,
        size: data_len,
    });

    let raw_pointer = Box::into_raw(context);

    let ct_lib = Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t =
        unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    let status = evt_api_call_default(raw_pointer);

    status
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

    let raw_pointer = Box::into_raw(context);

    let ct_lib = Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t =
        unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}
