use std::os::raw::{c_int, c_uint};
use crate::ffi::types::*;

/// Create a new match with specified max players
/// Returns match_id on success, 0 on failure
#[no_mangle]
pub extern "C" fn coreria_create_match(max_players: c_int) -> c_uint {
    if max_players < 2 || max_players > 8 {
        crate::ffi::set_error("Invalid player count (must be 2-8)");
        return 0;
    }

    match crate::with_engine(|engine| engine.create_match(max_players as usize)) {
        Ok(match_id) => {
            eprintln!("[CORERIA] Created match {} for {} players", match_id, max_players);
            match_id
        },
        Err(e) => {
            crate::ffi::set_error(&format!("Failed to create match: {}", e));
            0
        }
    }
}

/// Get current match state
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_get_match_state(match_id: c_uint, state: *mut CMatchState) -> c_int {
    if state.is_null() {
        return -1;
    }

    // This would query the actual match state from the engine
    // For now, return a placeholder
    unsafe {
        *state = CMatchState {
            match_id,
            status: 1, // Starting
            player_count: 0,
            max_players: 4,
            time_remaining: 300.0,
            winner_id: -1,
        };
    }
    
    0
}

/// End a match
#[no_mangle]
pub extern "C" fn coreria_end_match(match_id: c_uint) -> c_int {
    eprintln!("[CORERIA] Ending match {}", match_id);
    // Would implement actual match termination
    0
}

/// Get all active match IDs
/// Returns number of matches, fills match_ids array
#[no_mangle]
pub extern "C" fn coreria_get_active_matches(match_ids: *mut c_uint, max_matches: c_int) -> c_int {
    if match_ids.is_null() || max_matches <= 0 {
        return -1;
    }

    // Would query actual active matches
    // For now, return empty list
    0
}