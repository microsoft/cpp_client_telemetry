#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use log::debug;
use once_cell::sync::Lazy;
use std::{sync::RwLock, ffi::CString};
use types::{StringProperty, TelemetryItem, TelemetryProperty};
use cpp_client_telemetry_sys::{
    evt_open,
    evt_close,
    evt_log,
    evt_flush,
    evt_upload,
    evt_prop
};

pub mod appender;
pub mod types;

#[derive(Clone)]
pub struct LogManager {
    is_started: bool,
    configuration: CString,
    log_handle: Option<i64>,
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

    log_manager.flush(true);
}

pub fn offline_flush() {
    let mut log_manager = GLOBAL_LOG_MANAGER
        .write()
        .expect("GLOBAL_LOG_MANAGER RwLock poisoned");

    log_manager.flush(false);
}

impl LogManager {
    fn new() -> Self {
        LogManager {
            is_started: false,
            configuration: CString::new("{}").unwrap(),
            log_handle: Option::None,
        }
    }

    /**
     * Validates that a configuration has been set and
     */
    pub fn start(&mut self) -> bool {
        if self.is_started {
            return true;
        }

        let handle = evt_open(&self.configuration);

        if handle.is_none() {
            debug!("LogManager.start: failed");
            return false;
        }

        self.log_handle = handle;
        self.is_started = true;

        debug!("LogManager.start: success");

        return true;
    }

    pub fn flush(&mut self, upload: bool) {
        if self.is_started == false {
            return;
        }

        let handle = self.log_handle.as_ref().unwrap();
        evt_flush(handle);
        debug!("LogManager.flush(EVT_OP_FLUSH)");

        if upload {
            evt_upload(handle);
            debug!("LogManager.flush(EVT_OP_UPLOAD)");
        }
    }

    pub fn stop(&mut self) {
        if self.is_started == false {
            return;
        }

        self.flush(false);

        let handle = self.log_handle.as_ref().unwrap();
        evt_close(handle);
    }

    pub fn configure(&mut self, config: &str) {
        self.configuration = CString::new(config).unwrap();
        debug!("LogManager.configure:\n{}", config);
    }

    pub fn trace(&mut self, message: &str) {
        let mut item = TelemetryItem::new("trace");

        item.add_property(
            "message",
            TelemetryProperty::String(StringProperty::new(message)),
        );

        self.track_item(&item);
    }

    pub fn track_item(&mut self, item: &TelemetryItem) {
        let mut event_props = item.to_evt();

        debug!(
            "LogManager.track_item(event_props_len:{})",
            event_props.len()
        );

        let handle = self.log_handle.clone().unwrap();

        // TODO
        // evt_log_vec(&handle, &mut event_props);
    }

    pub fn track_evt(&self, mut event_props: &mut [evt_prop]) {
        debug!(
            "LogManager.track_evt(event_props_len:{})",
            event_props.len()
        );

        let handle = self.log_handle.clone().unwrap();
        evt_log(&handle, &mut event_props);
    }
}
