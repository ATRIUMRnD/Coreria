use std::os::raw::{c_float, c_int, c_uint};
use crate::ffi::types::*;

/// Add a player to a match with specified fighting style
/// Returns player_id on success, 0 on failure
#[no_mangle]
pub extern "C" fn coreria_add_player(match_id: c_uint, style_id: c_int) -> c_uint {
    let style = style_from_c_int(style_id);
    
    match crate::with_engine(|engine| engine.add_player(match_id, style)) {
        Ok(player_id) => {
            eprintln!("[CORERIA] Added player {} with style {:?} to match {}", 
                     player_id, style, match_id);
            player_id
        },
        Err(e) => {
            crate::ffi::set_error(&format!("Failed to add player: {}", e));
            0
        }
    }
}

/// Remove a player from their current match
#[no_mangle]
pub extern "C" fn coreria_remove_player(player_id: c_uint) -> c_int {
    eprintln!("[CORERIA] Removing player {}", player_id);
    // Would implement actual player removal
    0
}

/// Get player state information
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_get_player_state(player_id: c_uint, state: *mut CPlayerState) -> c_int {
    if state.is_null() {
        return -1;
    }

    // Would query actual player state from ECS
    // For now, return placeholder data
    unsafe {
        *state = CPlayerState {
            player_id,
            x: 0.0,
            y: 2.0,
            z: 0.0,
            qx: 0.0,
            qy: 0.0, 
            qz: 0.0,
            qw: 1.0,
            vx: 0.0,
            vy: 0.0,
            vz: 0.0,
            health: 100.0,
            style_id: STYLE_BRAWLER,
            is_alive: 1,
        };
    }

    0
}

/// Apply force to a player (for combat/physics)
/// force_x, force_y, force_z: Force vector components
#[no_mangle]
pub extern "C" fn coreria_apply_force(
    player_id: c_uint, 
    force_x: c_float, 
    force_y: c_float, 
    force_z: c_float
) -> c_int {
    let force_magnitude = (force_x * force_x + force_y * force_y + force_z * force_z).sqrt();
    
    if force_magnitude > 10000.0 {
        crate::ffi::set_error("Force magnitude too large");
        return -1;
    }

    eprintln!("[CORERIA] Applying force ({}, {}, {}) to player {}", 
             force_x, force_y, force_z, player_id);
    
    // Would apply force to physics body
    0
}

/// Set player position (for respawning/teleporting)
#[no_mangle]
pub extern "C" fn coreria_set_player_position(
    player_id: c_uint,
    x: c_float,
    y: c_float, 
    z: c_float
) -> c_int {
    eprintln!("[CORERIA] Setting player {} position to ({}, {}, {})", 
             player_id, x, y, z);
    
    // Would update player transform in ECS
    0
}

/// Deal damage to a player
#[no_mangle]
pub extern "C" fn coreria_damage_player(
    attacker_id: c_uint,
    victim_id: c_uint,
    damage: c_float
) -> c_int {
    if damage < 0.0 || damage > 1000.0 {
        crate::ffi::set_error("Invalid damage amount");
        return -1;
    }

    eprintln!("[CORERIA] Player {} dealt {} damage to player {}", 
             attacker_id, damage, victim_id);
    
    // Would apply damage using combat system
    0
}

/// Get all player IDs in a match
/// Returns number of players, fills player_ids array
#[no_mangle]
pub extern "C" fn coreria_get_match_players(
    match_id: c_uint,
    player_ids: *mut c_uint,
    max_players: c_int
) -> c_int {
    if player_ids.is_null() || max_players <= 0 {
        return -1;
    }

    eprintln!("[CORERIA] Getting players for match {}", match_id);
    
    // Would query actual players in match
    // For now, return empty list
    0
}

/// Check if player is alive
#[no_mangle]
pub extern "C" fn coreria_is_player_alive(player_id: c_uint) -> c_int {
    // Would check actual health component
    // For now, assume all players are alive
    1
}

/// Respawn a dead player
#[no_mangle]
pub extern "C" fn coreria_respawn_player(player_id: c_uint) -> c_int {
    eprintln!("[CORERIA] Respawning player {}", player_id);
    
    // Would reset health and position
    0
}