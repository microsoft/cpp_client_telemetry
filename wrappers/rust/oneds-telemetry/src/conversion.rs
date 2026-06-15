use crate::*;
use std::ffi::{CStr, CString};
use std::mem;
use std::os::raw::c_char;

/**
 * This will leak memory
 */
pub fn to_leaky_c_str(value: &str) -> *const i8 {
    let c_str = Box::new(CString::new(value.clone()).unwrap());
    let ptr = c_str.as_ptr() as *const i8;
    mem::forget(c_str);
    return ptr;
}

pub fn to_c_str(value: &str) -> *const c_char {
    let null_terminated_str = format!("{}\0", value);
    let c_str = CStr::from_bytes_with_nul(null_terminated_str.as_bytes())
        .expect("to_c_str: CString conversion failed");
    c_str.as_ptr()
}

pub fn string_prop(name: &str, value: &str, is_pii: bool) -> evt_prop {
    let mut pii_kind = 0;

    if is_pii {
        pii_kind = 1;
    }

    evt_prop {
        name: to_leaky_c_str(name),
        type_: evt_prop_t_TYPE_STRING,
        value: evt_prop_v {
            as_string: to_leaky_c_str(value),
        },
        piiKind: pii_kind,
    }
}

pub fn int_prop(name: &str, value: i64) -> evt_prop {
    evt_prop {
        name: to_c_str(name),
        type_: evt_prop_t_TYPE_INT64,
        value: evt_prop_v {
            as_int64: value.clone(),
        },
        piiKind: 0,
    }
}
