// https://fasterthanli.me/series/making-our-own-ping/part-4
use std::{
    ffi::{c_void, CString},
    mem::transmute_copy,
    os::raw::c_char,
    ptr::NonNull,
};

type HModule = NonNull<c_void>;
type FarProc = NonNull<c_void>;

extern "stdcall" {
    fn LoadLibraryA(name: *const c_char) -> Option<HModule>;
    fn GetProcAddress(module: HModule, name: *const c_char) -> Option<FarProc>;
}

pub struct Library {
    module: HModule,
}

impl Library {
    pub fn new(name: &str) -> Option<Self> {
        let name = CString::new(name).expect("invalid library name");
        let res = unsafe { LoadLibraryA(name.as_ptr()) };
        res.map(|module| Library { module })
    }

    // pub fn new(path: &Path) -> Option<Self> {
    //     let src = Path::join(&env::current_dir().unwrap(), "..\\lib\\ClientTelemetry.dll");
    // }

    pub unsafe fn get_proc<T>(&self, name: &str) -> Option<T> {
        let name = CString::new(name).expect("invalid proc name");
        let res = GetProcAddress(self.module, name.as_ptr());
        res.map(|proc| transmute_copy(&proc))
    }
}
