/*!
# FFI Interface

Safe C-compatible API for integrating with existing ChaosForge game.
Provides functions for match management, player spawning, and physics simulation.
*/

pub mod types;
pub mod match_api;
pub mod player_api;
pub mod input_api;

// Re-export FFI types for C header generation
pub use types::*;
pub use match_api::*;
pub use player_api::*;
pub use input_api::*;

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_float, c_int};

/// Initialize the multiplayer engine
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_init_engine() -> c_int {
    match crate::initialize_engine() {
        Ok(()) => {
            eprintln!("[CORERIA] Multiplayer engine initialized");
            0
        },
        Err(e) => {
            eprintln!("[CORERIA] Failed to initialize engine: {}", e);
            -1
        }
    }
}

/// Shutdown the multiplayer engine
#[no_mangle]
pub extern "C" fn coreria_shutdown_engine() -> c_int {
    match crate::shutdown_engine() {
        Ok(()) => {
            eprintln!("[CORERIA] Engine shutdown complete");
            0
        },
        Err(e) => {
            eprintln!("[CORERIA] Shutdown error: {}", e);
            -1
        }
    }
}

/// Step the physics simulation by delta_time seconds
/// Returns 1 if update occurred, 0 if skipped (timing)
#[no_mangle]
pub extern "C" fn coreria_step_simulation(delta_time: c_float) -> c_int {
    match crate::with_engine(|engine| engine.step_simulation(delta_time)) {
        Ok(updated) => if updated { 1 } else { 0 },
        Err(e) => {
            eprintln!("[CORERIA] Simulation step error: {}", e);
            -1
        }
    }
}

/// Get current player count across all matches
#[no_mangle]
pub extern "C" fn coreria_get_player_count() -> c_int {
    match crate::with_engine(|engine| engine.get_player_count()) {
        Ok(count) => count as c_int,
        Err(_) => -1,
    }
}

/// Set engine to server mode on specified port
#[no_mangle]
pub extern "C" fn coreria_init_server(port: u16) -> c_int {
    match crate::with_engine(|engine| engine.init_server(port)) {
        Ok(Ok(())) => {
            eprintln!("[CORERIA] Server started on port {}", port);
            0
        },
        Ok(Err(e)) => {
            eprintln!("[CORERIA] Server init error: {}", e);
            -1
        },
        Err(e) => {
            eprintln!("[CORERIA] Engine error: {}", e);
            -2
        }
    }
}

/// Connect to server as client
#[no_mangle]
pub extern "C" fn coreria_init_client(server_addr: *const c_char) -> c_int {
    if server_addr.is_null() {
        return -1;
    }

    let addr_str = unsafe {
        match CStr::from_ptr(server_addr).to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        }
    };

    match crate::with_engine(|engine| engine.init_client(addr_str)) {
        Ok(Ok(())) => {
            eprintln!("[CORERIA] Connected to server {}", addr_str);
            0
        },
        Ok(Err(e)) => {
            eprintln!("[CORERIA] Client init error: {}", e);
            -1
        },
        Err(e) => {
            eprintln!("[CORERIA] Engine error: {}", e);
            -2
        }
    }
}

/// Get performance metrics
#[no_mangle]
pub extern "C" fn coreria_get_performance_metrics(metrics: *mut CPerformanceMetrics) -> c_int {
    if metrics.is_null() {
        return -1;
    }

    match crate::with_engine(|engine| engine.get_performance_metrics()) {
        Ok(perf) => {
            unsafe {
                (*metrics) = CPerformanceMetrics::from_rust(perf);
            }
            0
        },
        Err(_) => -1,
    }
}

/// Error handling: Get last error message
static mut LAST_ERROR: Option<CString> = None;

#[no_mangle]
pub extern "C" fn coreria_get_last_error() -> *const c_char {
    unsafe {
        match &LAST_ERROR {
            Some(err) => err.as_ptr(),
            None => std::ptr::null(),
        }
    }
}

/// Set error message for C code to retrieve
pub fn set_error(message: &str) {
    unsafe {
        LAST_ERROR = CString::new(message).ok();
    }
}