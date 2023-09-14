use std::os::raw::c_void;

use crate::*;

pub struct LogManager {
    call_api: evt_api_call_t,
    is_started: bool,
    configuration: Option<String>,
    log_handle: Option<Box<i64>>
}

impl LogManager {
    pub fn new() -> Self {
        let ct_lib = LoadLibrary::Library::new("ClientTelemetry.dll").unwrap();
        let call_api: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

        LogManager { 
            call_api: call_api,
            is_started: false,
            configuration: Option::None,
            log_handle: Option::None
        }
    }
    
    pub fn start(&mut self) -> bool {
        if self.configuration.is_none() {
            return false;
        }

        if self.is_started {
            return true;
        }

        let config_bytes = self.configuration.clone().unwrap().into_bytes();
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

        let result = (self.call_api)(raw_pointer);
    
        if result == -1 {
            return false;
        }

        context = unsafe { Box::from_raw(raw_pointer) };
        self.log_handle = Some(Box::new(context.handle));
        self.is_started = true;

        return true;
    }

    pub fn stop(&mut self) {
        if self.is_started == false {
            return;
        }

        let mut c_chars: Vec<i8> = Vec::new();
        c_chars.push(0); // null
        let null_config_data = c_chars.as_mut_ptr() as *mut c_void;
        let handle = self.log_handle.as_ref().unwrap();

        let context: Box<evt_context_t> = Box::new(evt_context_t { 
            call: evt_call_t_EVT_OP_FLUSHANDTEARDOWN, 
            handle: *handle.as_ref(), 
            data: null_config_data,
            result: 0, 
            size: 0 
        });

        let raw_pointer = Box::into_raw(context);
    
        let _ = (self.call_api)(raw_pointer);
    }
}