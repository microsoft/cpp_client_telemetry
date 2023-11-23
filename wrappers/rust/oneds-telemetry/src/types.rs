use std::{
    collections::HashMap,
    ffi::{CStr, CString},
    os::raw::c_char,
};

use chrono::{DateTime, Utc};
use log::{debug, error};
use once_cell::sync::Lazy;

use cpp_client_telemetry_sys::{evt_prop, evt_prop_v, evt_prop_t_TYPE_STRING, evt_prop_t_TYPE_TIME};

#[derive(Clone, Debug)]
pub struct StringProperty {
    bytes: Vec<u8>,
}

impl StringProperty {
    pub fn new(value: &str) -> Self {
        let bytes = CString::new(value).expect("Unable to convert value to CString");

        StringProperty {
            bytes: bytes.to_bytes_with_nul().to_vec(),
        }
    }

    pub fn as_ptr(&self) -> *const c_char {
        self.bytes.as_ptr() as *const c_char
    }

    pub fn as_bytes(&self) -> Vec<u8> {
        self.bytes.clone()
    }

    fn to_str(&self) -> Result<&str, std::str::Utf8Error> {
        unsafe { CStr::from_ptr(self.as_ptr()).to_str() }
    }
}

pub static NAME_PROPERTY: Lazy<StringProperty> = Lazy::new(|| StringProperty::new("name"));
pub static TIME_PROPERTY: Lazy<StringProperty> = Lazy::new(|| StringProperty::new("time"));
pub static EMPTY_STRING: Lazy<StringProperty> = Lazy::new(|| StringProperty::new(""));

#[derive(Clone, Debug)]
pub enum TelemetryProperty {
    String(StringProperty),
    Int(i64),
}

pub type TelemetryPropertyBag = HashMap<&'static str, TelemetryProperty>;

#[derive(Clone, Debug)]
pub struct TelemetryItem {
    name: StringProperty,
    data: TelemetryPropertyBag,
    time: Option<DateTime<Utc>>,
}

impl TelemetryItem {
    pub fn new(name: &'static str) -> TelemetryItem {
        TelemetryItem {
            name: StringProperty::new(name),
            data: TelemetryPropertyBag::new(),
            time: Option::None,
        }
    }

    pub fn new_with_time(name: &'static str, time: DateTime<Utc>) -> TelemetryItem {
        TelemetryItem {
            name: StringProperty::new(name),
            data: TelemetryPropertyBag::new(),
            time: Some(time),
        }
    }

    pub fn add_property(&mut self, name: &'static str, value: TelemetryProperty) {
        self.data.insert(name, value);
    }

    pub fn to_evt(&self) -> Box<Vec<evt_prop>> {
        let mut properties = Box::new(Vec::new());

        properties.push(evt_prop {
            name: NAME_PROPERTY.as_ptr(),
            type_: evt_prop_t_TYPE_STRING,
            value: evt_prop_v {
                as_string: self.name.as_ptr(),
            },
            piiKind: 0,
        });

        if self.time != Option::None {
            // Convert UTC time to a Unix timestamp as i64
            let timestamp_i64 = self.time.unwrap().timestamp();

            // Convert the i64 timestamp to u64
            let millis = timestamp_i64 as u64;

            properties.push(evt_prop {
                name: TIME_PROPERTY.as_ptr(),
                type_: evt_prop_t_TYPE_TIME,
                value: evt_prop_v { as_time: millis },
                piiKind: 0,
            });
        }

        for key in self.data.keys() {
            let value = self.data.get(key).expect("Unable to get value");
            match value {
                TelemetryProperty::String(value) => {}
                TelemetryProperty::Int(value) => {}
                _ => error!("Unknown TelemetryProperty Type: [{}]", key),
            }
        }

        debug!("{}", self.name.to_str().unwrap());

        properties.shrink_to_fit();

        properties
    }
}

// // This is new
// impl Drop for TelemetryItem {
//     fn drop(&mut self) {
//         debug!(
//             "[TelemetryItem] Dropping Item: {}",
//             self.name.to_str().unwrap()
//         );
//     }
// }

#[cfg(test)]
mod tests {
    use crate::types::{StringProperty, EMPTY_STRING, NAME_PROPERTY, TIME_PROPERTY};

    #[test]
    fn validate_c_str_arrays() {
        let mut iteration = 0;

        while iteration < 5 {
            iteration = iteration + 1;

            let value1 = StringProperty::new("TEST_VALUE1");
            let value2 = StringProperty::new("TEST_VALUE2");
            println!("{:?}", value1.as_bytes());
            assert_eq!("TEST_VALUE1", value1.to_str().unwrap());
            assert_eq!("TEST_VALUE2", value2.to_str().unwrap());
        }
    }

    #[test]
    fn validate_static_props() {
        let mut iteration = 0;

        while iteration < 5 {
            iteration = iteration + 1;

            assert_eq!("name", NAME_PROPERTY.to_str().unwrap());
            assert_eq!("time", TIME_PROPERTY.to_str().unwrap());
            assert_eq!("", EMPTY_STRING.to_str().unwrap());
        }
    }
}
