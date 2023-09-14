use std::os::raw::c_void;

use crate::*;

pub type evt_api_call_t = extern "stdcall" fn(context: *mut evt_context_t) -> evt_status_t;

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

    let ct_lib = client_library::Library::new("ClientTelemetry.dll").unwrap();
    let evt_api_call_default: evt_api_call_t =
        unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

    return evt_api_call_default(raw_pointer);
}
