#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use chrono;
use internals::evt_api_call_t;
use log::debug;
use once_cell::sync::Lazy;
use std::{
    ffi::{c_void, CString},
    sync::RwLock,
};

pub mod appender;
mod client_library;
mod helpers;
mod internals;

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[derive(Clone)]
pub struct LogManager {
    call_api: evt_api_call_t,
    is_started: bool,
    configuration: Option<String>,
    log_handle: Option<Box<i64>>,
}

/// The global `Tracer` provider singleton.
static GLOBAL_LOG_MANAGER: Lazy<RwLock<LogManager>> = Lazy::new(|| RwLock::new(LogManager::new()));

pub fn log_manager_provider() -> LogManager {
    GLOBAL_LOG_MANAGER
        .read()
        .expect("GLOBAL_LOG_MANAGER RwLock poisoned")
        .clone()
}

pub fn init(config: &str) {
    let mut log_manager = GLOBAL_LOG_MANAGER
        .write()
        .expect("GLOBAL_LOG_MANAGER RwLock poisoned");

    if log_manager.is_started == false {
        log_manager.configure(config);
        log_manager.start();
    }
}

pub fn flush() {
    let mut log_manager = GLOBAL_LOG_MANAGER
        .write()
        .expect("GLOBAL_LOG_MANAGER RwLock poisoned");

    log_manager.flush();
}

impl LogManager {
    pub fn new() -> Self {
        let ct_lib = client_library::Library::new("ClientTelemetry.dll").unwrap();
        let call_api: evt_api_call_t = unsafe { ct_lib.get_proc("evt_api_call_default").unwrap() };

        LogManager {
            call_api: call_api,
            is_started: false,
            configuration: Option::None,
            log_handle: Option::None,
        }
    }

    /**
     * Validates that a configuration has been set and
     */
    pub fn start(&mut self) -> bool {
        if self.configuration.is_none() {
            return false;
        }

        if self.is_started {
            return true;
        }

        // We don't need to leak this value
        let config = self.configuration.clone().unwrap();
        let c_str = Box::new(CString::new(config.as_str()).unwrap());
        let config_data = c_str.as_ptr() as *mut c_void;

        let mut context: Box<evt_context_t> = Box::new(evt_context_t {
            call: evt_call_t_EVT_OP_OPEN,
            handle: 0,
            data: config_data,
            result: 0,
            size: 0,
        });

        let raw_pointer = Box::into_raw(context);

        let result = (self.call_api)(raw_pointer);

        if result == -1 {
            debug!("LogManager.start: failed");
            return false;
        }

        context = unsafe { Box::from_raw(raw_pointer) };
        self.log_handle = Some(Box::new(context.handle));
        self.is_started = true;

        debug!("LogManager.start: success");

        return true;
    }

    pub fn flush(&mut self) {
        if self.is_started == false {
            return;
        }

        let mut c_chars: Vec<i8> = Vec::new();
        c_chars.push(0); // null
        let null_config_data = c_chars.as_mut_ptr() as *mut c_void;
        let handle = self.log_handle.as_ref().unwrap();

        let flush_ctx: Box<evt_context_t> = Box::new(evt_context_t {
            call: evt_call_t_EVT_OP_FLUSH,
            handle: *handle.as_ref(),
            data: null_config_data,
            result: 0,
            size: 0,
        });

        let flush_ctx_ptr = Box::into_raw(flush_ctx);

        let _ = (self.call_api)(flush_ctx_ptr);
        debug!("LogManager.flush(EVT_OP_FLUSH)");

        let upload_ctx: Box<evt_context_t> = Box::new(evt_context_t {
            call: evt_call_t_EVT_OP_UPLOAD,
            handle: *handle.as_ref(),
            data: null_config_data,
            result: 0,
            size: 0,
        });

        let upload_ctx_ptr = Box::into_raw(upload_ctx);

        let _ = (self.call_api)(upload_ctx_ptr);
        debug!("LogManager.flush(EVT_OP_UPLOAD)");
    }

    pub fn stop(&mut self) {
        if self.is_started == false {
            return;
        }

        let mut c_chars: Vec<i8> = Vec::new();
        c_chars.push(0); // null
        let null_config_data = c_chars.as_mut_ptr() as *mut c_void;
        let handle = self.log_handle.as_ref().unwrap();

        let flush_ctx: Box<evt_context_t> = Box::new(evt_context_t {
            call: evt_call_t_EVT_OP_FLUSH,
            handle: *handle.as_ref(),
            data: null_config_data,
            result: 0,
            size: 0,
        });

        let flush_ctx_ptr = Box::into_raw(flush_ctx);

        let _ = (self.call_api)(flush_ctx_ptr);

        let close_ctx: Box<evt_context_t> = Box::new(evt_context_t {
            call: evt_call_t_EVT_OP_CLOSE,
            handle: *handle.as_ref(),
            data: null_config_data,
            result: 0,
            size: 0,
        });

        let close_ctx_ptr = Box::into_raw(close_ctx);

        let _ = (self.call_api)(close_ctx_ptr);
    }

    pub fn configure(&mut self, config: &str) {
        self.configuration = Some(String::from(config.clone()));
        debug!("LogManager.configure:\n{}", config);
    }

    pub fn log_event(&self, name: &str) {
        let evt_time = chrono::Utc::now();

        let mut event_props: [evt_prop; 2] = [
            helpers::string_prop("name", name, false),
            helpers::int_prop("time", evt_time.timestamp_millis()),
        ];

        self.log(&mut event_props);
    }

    pub fn trace(&self, message: &str) {
        let evt_time = chrono::Utc::now();

        let mut event_props: [evt_prop; 3] = [
            helpers::string_prop("name", "trace", false),
            helpers::int_prop("time", evt_time.timestamp_millis()),
            helpers::string_prop("message", message, false),
        ];

        self.log(&mut event_props);
    }

    pub fn log(&self, mut event_props: &mut [evt_prop]) {
        debug!("LogManager.log(event_props_len:{})", event_props.len());
        let handle = self.log_handle.clone().unwrap();
        internals::evt_log(&handle, &mut event_props);
    }
}
