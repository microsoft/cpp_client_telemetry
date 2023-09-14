use crate::*;
use std::ffi::CString;
use std::mem;

/**
 * This will convert a &str value to a c-string array that is compatible with
 * the library; however, if improperly used this will run the risk of leaking
 * memory as this uses mem::forget to ensure that the string data is not freed
 * after making the conversion. This may mean that in order to pass the data
 * from rust to c that we need some wraper structs that convert into the c
 * structs so that the data is not lost in the call.
 */
pub fn to_leaky_c_str(value: &str) -> *const i8 {
    let c_str = Box::new(CString::new(value.clone()).unwrap());
    let ptr = c_str.as_ptr() as *const i8;
    mem::forget(c_str);
    return ptr;
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
        name: to_leaky_c_str(name),
        type_: evt_prop_t_TYPE_INT64,
        value: evt_prop_v {
            as_int64: value.clone(),
        },
        piiKind: 0,
    }
}
