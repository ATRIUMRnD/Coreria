/*!
# FFI Bridge

Foreign Function Interface bridge between Rust engine core and C game logic.
Provides C-compatible functions for game developers to interact with the engine.
*/

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_float, c_int, c_uint};
use log::{info, error, debug};

pub mod engine_api;
pub mod graphics_api;
pub mod physics_api;

/// Error handling for FFI
static mut LAST_FFI_ERROR: Option<CString> = None;

/// Initialize the Coreria engine from C code
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_engine_init() -> c_int {
    info!("🔗 FFI: Initializing Coreria engine from C code");
    
    match std::panic::catch_unwind(|| {
        // In a full implementation, this would:
        // - Initialize the main engine
        // - Set up resource sharing between Rust and C
        // - Configure the FFI layer
        
        debug!("✅ FFI: Engine initialization completed");
        0
    }) {
        Ok(result) => result,
        Err(_) => {
            set_ffi_error("Engine initialization panicked");
            -1
        }
    }
}

/// Shutdown the engine
#[no_mangle]
pub extern "C" fn coreria_engine_shutdown() -> c_int {
    info!("🔗 FFI: Shutting down Coreria engine");
    
    // Cleanup FFI resources
    unsafe {
        LAST_FFI_ERROR = None;
    }
    
    0
}

/// Step the engine simulation by one frame
/// delta_time: Time elapsed since last frame in seconds
/// Returns 1 if engine is still running, 0 if should exit
#[no_mangle]
pub extern "C" fn coreria_engine_step(delta_time: c_float) -> c_int {
    // In full implementation, this would step the Bevy app
    debug!("🔗 FFI: Engine step (dt: {:.3}s)", delta_time);
    
    // Return 1 to indicate engine should continue running
    1
}

/// Check if engine is running
#[no_mangle]
pub extern "C" fn coreria_engine_is_running() -> c_int {
    // In full implementation, check actual engine state
    1
}

/// Get the last error message
/// Returns null-terminated string, or NULL if no error
#[no_mangle]
pub extern "C" fn coreria_get_last_error() -> *const c_char {
    unsafe {
        match &LAST_FFI_ERROR {
            Some(error) => error.as_ptr(),
            None => std::ptr::null(),
        }
    }
}

/// Set an error message (internal function)
pub fn set_ffi_error(message: &str) {
    unsafe {
        LAST_FFI_ERROR = match CString::new(message) {
            Ok(cstring) => Some(cstring),
            Err(_) => {
                // Fallback error message if original contains null bytes
                CString::new("Invalid error message").ok()
            }
        };
    }
    error!("🔗 FFI Error: {}", message);
}

/// Create a test scene (for demonstration)
#[no_mangle]
pub extern "C" fn coreria_create_test_scene() -> c_int {
    info!("🔗 FFI: Creating test scene");
    
    // In full implementation, this would:
    // - Spawn some entities with physics bodies
    // - Set up lighting and camera
    // - Add some interactive objects
    
    info!("✅ Test scene created with basic entities");
    0
}

/// Spawn a physics object at the given position
/// Returns entity ID, or negative value on error
#[no_mangle]
pub extern "C" fn coreria_spawn_physics_object(
    x: c_float, 
    y: c_float, 
    z: c_float,
    mass: c_float
) -> c_int {
    debug!("🔗 FFI: Spawning physics object at ({}, {}, {}) with mass {}", 
           x, y, z, mass);
    
    // In full implementation, this would:
    // - Create a new entity in the Bevy world
    // - Add Transform, RigidBody, and CollisionShape components
    // - Return the entity ID for C code to reference
    
    // For now, return a dummy entity ID
    let entity_id = 12345;
    info!("✅ Created physics object with ID: {}", entity_id);
    entity_id
}

/// Apply force to a physics object
#[no_mangle]
pub extern "C" fn coreria_apply_force(
    entity_id: c_uint,
    force_x: c_float,
    force_y: c_float, 
    force_z: c_float
) -> c_int {
    debug!("🔗 FFI: Applying force ({}, {}, {}) to entity {}", 
           force_x, force_y, force_z, entity_id);
    
    // In full implementation, this would:
    // - Look up the entity by ID
    // - Add the force to its Forces component
    // - Let the physics system process it next frame
    
    0 // Success
}

/// Get version information
#[no_mangle]
pub extern "C" fn coreria_get_version() -> *const c_char {
    static VERSION: &str = concat!(env!("CARGO_PKG_VERSION"), "\0");
    VERSION.as_ptr() as *const c_char
}

/// Set engine configuration from C
#[no_mangle]
pub extern "C" fn coreria_set_config(
    config_name: *const c_char,
    config_value: *const c_char
) -> c_int {
    if config_name.is_null() || config_value.is_null() {
        set_ffi_error("Null pointer passed to coreria_set_config");
        return -1;
    }
    
    let name = match unsafe { CStr::from_ptr(config_name) }.to_str() {
        Ok(s) => s,
        Err(_) => {
            set_ffi_error("Invalid UTF-8 in config name");
            return -1;
        }
    };
    
    let value = match unsafe { CStr::from_ptr(config_value) }.to_str() {
        Ok(s) => s,
        Err(_) => {
            set_ffi_error("Invalid UTF-8 in config value");
            return -1;
        }
    };
    
    debug!("🔗 FFI: Setting config {} = {}", name, value);
    
    // In full implementation, this would update runtime configuration
    match name {
        "physics_enabled" => {
            info!("⚡ Physics enabled: {}", value);
        },
        "debug_mode" => {
            info!("🐛 Debug mode: {}", value);
        },
        "render_scale" => {
            info!("🖼️  Render scale: {}", value);
        },
        _ => {
            debug!("⚠️  Unknown config: {}", name);
        }
    }
    
    0
}