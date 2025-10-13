/*!
# FFI Module

C-compatible interface for integration with existing ChaosForge game.
Provides safe Rust functions callable from C code.
*/

pub mod types;
pub mod player_ffi;
pub mod match_ffi;
pub mod input_ffi;

use std::os::raw::{c_char, c_float, c_int};
use std::ffi::{CStr, CString};
use std::ptr;

pub use types::*;
pub use player_ffi::*;
pub use match_ffi::*;
pub use input_ffi::*;

/// Initialize the multiplayer engine  
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_initialize_engine() -> c_int {
    println!("[CORERIA] Engine initialize called");
    0
}

/// Initialize the multiplayer engine (full version)
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_initialize_engine_full() -> c_int {
    match crate::initialize_engine() {
        Ok(_) => {
            println!("ChaosForge multiplayer engine initialized");
            0
        },
        Err(e) => {
            eprintln!("Failed to initialize engine: {}", e);
            -1
        }
    }
}

/// Shutdown the multiplayer engine
/// Returns 0 on success
#[no_mangle]
pub extern "C" fn coreria_shutdown_engine() -> c_int {
    match crate::shutdown_engine() {
        Ok(_) => {
            println!("Engine shutdown complete");
            0
        },
        Err(e) => {
            eprintln!("Failed to shutdown engine: {}", e);
            -1
        }
    }
}

/// Step the simulation by one frame
/// delta_time: Frame time in seconds (typically 0.016 for 60 FPS)
/// Returns 1 if update occurred, 0 if skipped due to timing
#[no_mangle]
pub extern "C" fn coreria_step_simulation(delta_time: c_float) -> c_int {
    match crate::with_engine(|engine| engine.step_simulation(delta_time)) {
        Ok(updated) => if updated { 1 } else { 0 },
        Err(_) => -1,
    }
}

/// Get engine performance metrics
/// Returns pointer to metrics struct, or null on error
#[no_mangle]
pub extern "C" fn coreria_get_performance_metrics() -> *const CPerformanceMetrics {
    match crate::with_engine(|engine| engine.get_performance_metrics()) {
        Ok(metrics) => {
            // Convert to C-compatible struct
            let c_metrics = CPerformanceMetrics {
                frame_time_ms: metrics.frame_time.as_millis() as c_float,
                physics_time_ms: metrics.physics_time.as_millis() as c_float,
                network_latency_ms: metrics.network_latency.as_millis() as c_float,
                update_rate: metrics.update_rate,
                player_count: metrics.player_count as c_int,
                memory_usage_bytes: metrics.memory_usage as c_int,
            };
            
            // Note: This is unsafe - would need proper memory management
            Box::into_raw(Box::new(c_metrics))
        },
        Err(_) => ptr::null(),
    }
}

/// Initialize as server
/// port: Server port number
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_init_server(port: c_int) -> c_int {
    if port <= 0 || port > 65535 {
        return -1;
    }
    
    match crate::with_engine(|engine| engine.init_server(port as u16)) {
        Ok(Ok(_)) => 0,
        _ => -1,
    }
}

/// Initialize as client
/// server_addr: Null-terminated server address string
/// Returns 0 on success, negative on error  
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
        Ok(Ok(_)) => 0,
        _ => -1,
    }
}

/// Convert C string to Rust String safely
pub fn c_str_to_string(c_str: *const c_char) -> Result<String, &'static str> {
    if c_str.is_null() {
        return Err("Null pointer");
    }
    
    unsafe {
        CStr::from_ptr(c_str)
            .to_str()
            .map(|s| s.to_owned())
            .map_err(|_| "Invalid UTF-8")
    }
}

/// Convert Rust String to C string (caller must free)
pub fn string_to_c_str(s: String) -> *mut c_char {
    match CString::new(s) {
        Ok(c_string) => c_string.into_raw(),
        Err(_) => ptr::null_mut(),
    }
}

/// Free C string allocated by Rust
#[no_mangle]
pub extern "C" fn coreria_free_string(s: *mut c_char) {
    if !s.is_null() {
        unsafe {
            let _ = CString::from_raw(s);
        }
    }
}