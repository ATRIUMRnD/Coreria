use std::os::raw::{c_int, c_uint};
use crate::ffi::types::*;

/// Send input action for a player
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_send_input(
    player_id: c_uint,
    action: CInputAction
) -> c_int {
    if let Some(rust_action) = action.to_rust() {
        eprintln!("[CORERIA] Player {} input: {:?}", player_id, rust_action);
        
        // Would add to input buffer and process
        0
    } else {
        crate::ffi::set_error("Invalid input action");
        -1
    }
}

/// Send multiple input actions in a batch
#[no_mangle]
pub extern "C" fn coreria_send_input_batch(
    player_id: c_uint,
    actions: *const CInputAction,
    action_count: c_int
) -> c_int {
    if actions.is_null() || action_count <= 0 || action_count > 16 {
        crate::ffi::set_error("Invalid input batch");
        return -1;
    }

    let action_slice = unsafe {
        std::slice::from_raw_parts(actions, action_count as usize)
    };

    let mut valid_actions = Vec::new();
    for c_action in action_slice {
        if let Some(rust_action) = c_action.to_rust() {
            valid_actions.push(rust_action);
        }
    }

    if valid_actions.is_empty() {
        return -1;
    }

    eprintln!("[CORERIA] Player {} batch input: {} actions", 
             player_id, valid_actions.len());
    
    // Would add all actions to input buffer
    0
}

/// Clear input buffer for a player
#[no_mangle]
pub extern "C" fn coreria_clear_input_buffer(player_id: c_uint) -> c_int {
    eprintln!("[CORERIA] Clearing input buffer for player {}", player_id);
    
    // Would clear the InputBuffer component
    0
}

/// Check if a specific action is currently active for a player
#[no_mangle]
pub extern "C" fn coreria_is_action_active(
    _player_id: c_uint,
    action: CInputAction
) -> c_int {
    if let Some(_rust_action) = action.to_rust() {
        // Would check input buffer for recent action
        0 // Not active
    } else {
        -1 // Invalid action
    }
}

/// Process a Windows keycode and convert to input action
/// Returns action_type on success, -1 if key not mapped
#[no_mangle]
pub extern "C" fn coreria_keycode_to_action(keycode: c_uint) -> c_int {
    if let Some(action) = crate::entities::keycode_to_input_action(keycode) {
        CInputAction::from_rust(action).action_type
    } else {
        -1
    }
}

/// Get input buffer size for a player
#[no_mangle]
pub extern "C" fn coreria_get_input_buffer_size(_player_id: c_uint) -> c_int {
    // Would return actual buffer size from InputBuffer component
    0
}

/// Enable/disable input processing for a player
#[no_mangle] 
pub extern "C" fn coreria_set_input_enabled(player_id: c_uint, enabled: c_int) -> c_int {
    let enabled_bool = enabled != 0;
    eprintln!("[CORERIA] Setting input enabled={} for player {}", 
             enabled_bool, player_id);
    
    // Would modify input processing state
    0
}