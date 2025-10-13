use std::os::raw::{c_float, c_int};
use crate::ffi::ffi_types::*;

/// Send input action for a specific player
/// player_id: Player performing the action
/// action_type: Action constant (ACTION_MOVE_FORWARD, ACTION_PUNCH, etc.)
/// param1, param2: Action parameters (e.g., movement direction, force)
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_send_input(
    player_id: c_int,
    action_type: c_int,
    param1: c_float,
    param2: c_float,
) -> c_int {
    // Convert C action to Rust InputAction
    let input_action = match c_to_input_action(action_type) {
        Some(action) => action,
        None => return -1, // Invalid action type
    };

    match crate::with_engine(|engine| {
        // TODO: Add input to player's InputBuffer component
        println!("Player {} input: {:?} ({}, {})", player_id, input_action, param1, param2);
        
        // This would integrate with the ECS input processing system
        // For now, just log the input
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Send multiple input actions for a player in a single frame
/// player_id: Player performing the actions
/// actions: Array of input actions
/// action_count: Number of actions in the array
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_send_input_batch(
    player_id: c_int,
    actions: *const CInputAction,
    action_count: c_int,
) -> c_int {
    if actions.is_null() || action_count <= 0 || action_count > 16 {
        return -1; // Sanity checks
    }

    let mut input_actions = Vec::new();
    
    // Convert C actions to Rust actions
    for i in 0..action_count {
        let c_action = unsafe { *actions.offset(i as isize) };
        
        if let Some(rust_action) = c_to_input_action(c_action.action_type) {
            input_actions.push(rust_action);
        }
    }

    match crate::with_engine(|engine| {
        // TODO: Add all inputs to player's InputBuffer for this frame
        println!("Player {} batch input: {} actions", player_id, input_actions.len());
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Process Windows key input (helper for C integration)
/// player_id: Player performing input
/// key_code: Windows virtual key code
/// is_pressed: 1 if key pressed, 0 if released
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_process_key_input(
    player_id: c_int,
    key_code: c_int,
    is_pressed: c_int,
) -> c_int {
    // Convert Windows key code to InputAction
    let input_action = match crate::entities::keycode_to_input_action(key_code as u32) {
        Some(action) => action,
        None => return 0, // Not a recognized key, but not an error
    };

    if is_pressed != 0 {
        // Key pressed - send input action
        coreria_send_input(player_id, input_action_to_c(input_action), 0.0, 0.0)
    } else {
        // Key released - for some actions we might want to send a "stop" signal
        match input_action {
            crate::entities::InputAction::Block => {
                // Stop blocking when key released
                coreria_send_input(player_id, -1, 0.0, 0.0) // Special "stop blocking" code
            },
            _ => 0, // Most actions don't need release handling
        }
    }
}

/// Process mouse input (helper for C integration)
/// player_id: Player performing input
/// button: Mouse button (1=left, 2=right, 3=middle)
/// is_pressed: 1 if pressed, 0 if released
/// mouse_x, mouse_y: Mouse position in window coordinates
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_process_mouse_input(
    player_id: c_int,
    button: c_int,
    is_pressed: c_int,
    mouse_x: c_float,
    mouse_y: c_float,
) -> c_int {
    if is_pressed == 0 {
        return 0; // Only handle mouse press, not release
    }

    let action_type = match button {
        1 => ACTION_PUNCH,      // Left click = punch
        2 => ACTION_KICK,       // Right click = kick  
        3 => ACTION_GRAB,       // Middle click = grab
        _ => return 0,          // Unknown button
    };

    // Use mouse position as action parameters (for direction/targeting)
    coreria_send_input(player_id, action_type, mouse_x, mouse_y)
}

/// Set player movement direction (continuous input)
/// player_id: Player to move
/// forward: Forward/backward input (-1.0 to 1.0)
/// right: Left/right input (-1.0 to 1.0)
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_set_movement_input(
    player_id: c_int,
    forward: c_float,
    right: c_float,
) -> c_int {
    // Clamp inputs to valid range
    let forward_clamped = forward.max(-1.0).min(1.0);
    let right_clamped = right.max(-1.0).min(1.0);

    match crate::with_engine(|engine| {
        // TODO: Update player's movement component directly
        // This is more efficient than sending discrete move actions every frame
        println!("Player {} movement: forward={:.2}, right={:.2}", 
                 player_id, forward_clamped, right_clamped);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Get input buffer for a player (for debugging/replay)
/// player_id: Player to query
/// frame_count: Number of recent frames to return
/// Returns number of frames actually returned, negative on error
#[no_mangle]
pub extern "C" fn coreria_get_input_history(
    player_id: c_int,
    frame_count: c_int,
) -> c_int {
    if frame_count <= 0 || frame_count > 180 { // Max 3 seconds at 60fps
        return -1;
    }

    match crate::with_engine(|engine| {
        // TODO: Query player's InputBuffer component
        // Return the requested number of recent frames
        let frames_available = frame_count.min(60); // Placeholder
        frames_available
    }) {
        Ok(frames) => frames,
        Err(_) => -1,
    }
}

/// Clear input buffer for a player (for state reset)
/// player_id: Player to reset
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_clear_input_buffer(player_id: c_int) -> c_int {
    match crate::with_engine(|engine| {
        // TODO: Clear the player's InputBuffer component
        println!("Clearing input buffer for player {}", player_id);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}