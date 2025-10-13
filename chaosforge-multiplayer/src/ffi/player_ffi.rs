use std::os::raw::{c_char, c_float, c_int};
use std::ptr;
use crate::ffi::ffi_types::*;

/// Add a player to a match
/// match_id: ID of the match to join
/// style_id: Fighting style (STYLE_BRAWLER, STYLE_STRIKER, etc.)
/// player_name: Null-terminated player name string
/// Returns player ID on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_add_player(
    match_id: c_int,
    style_id: c_int,
    player_name: *const c_char,
) -> c_int {
    // Validate style
    let style = match c_to_fighting_style(style_id) {
        Some(s) => s,
        None => return -1,
    };

    // Get player name or use default
    let name = if player_name.is_null() {
        format!("Player_{}", rand::random::<u32>())
    } else {
        match crate::ffi::c_str_to_string(player_name) {
            Ok(s) => s,
            Err(_) => return -1,
        }
    };

    // Add player via engine
    match crate::with_engine(|engine| {
        let player_id = engine.add_player(match_id as u32, style);
        player_id
    }) {
        Ok(player_id) => {
            println!("Added player '{}' with style {:?} (ID: {})", name, style, player_id);
            player_id as c_int
        },
        Err(_) => -1,
    }
}

/// Remove a player from their current match
/// player_id: ID of player to remove
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_remove_player(player_id: c_int) -> c_int {
    // TODO: Implement player removal logic
    println!("Removing player {}", player_id);
    0
}

/// Get current state of a specific player
/// player_id: ID of player to query
/// Returns pointer to player state struct, or null if not found
#[no_mangle]
pub extern "C" fn coreria_get_player_state(player_id: c_int) -> *const CPlayerState {
    match crate::with_engine(|engine| {
        // TODO: Query ECS for player entity and convert to C struct
        let player_state = CPlayerState {
            player_id,
            x: 0.0, y: 2.0, z: 0.0, // Default spawn position
            qx: 0.0, qy: 0.0, qz: 0.0, qw: 1.0, // Identity rotation
            vx: 0.0, vy: 0.0, vz: 0.0, // No velocity
            health: 100.0,
            max_health: 100.0,
            style_id: STYLE_BRAWLER,
            is_alive: 1,
            is_blocking: 0,
        };
        
        // Note: This is unsafe memory management - would need proper lifetime handling
        Box::into_raw(Box::new(player_state))
    }) {
        Ok(ptr) => ptr,
        Err(_) => ptr::null(),
    }
}

/// Apply force to a player's ragdoll
/// player_id: Target player ID
/// fx, fy, fz: Force vector components
/// force_type: Type of force (0=impulse, 1=continuous, 2=explosion)
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_apply_force(
    player_id: c_int,
    fx: c_float,
    fy: c_float,
    fz: c_float,
    force_type: c_int,
) -> c_int {
    let force_vector = bevy::math::Vec3::new(fx, fy, fz);
    
    match crate::with_engine(|engine| {
        // TODO: Apply force to player's ragdoll via physics system
        println!("Applying force {:?} to player {}", force_vector, player_id);
        // This would integrate with the physics engine
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Set player position (for teleporting/respawning)
/// player_id: Target player ID
/// x, y, z: New position coordinates
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_set_player_position(
    player_id: c_int,
    x: c_float,
    y: c_float,
    z: c_float,
) -> c_int {
    let position = bevy::math::Vec3::new(x, y, z);
    
    match crate::with_engine(|engine| {
        // TODO: Update player Transform component
        println!("Setting player {} position to {:?}", player_id, position);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Get all active players in a match
/// match_id: Match to query
/// player_states: Output array of player states (must be allocated by caller)
/// max_players: Maximum number of players to return
/// Returns actual number of players returned, negative on error
#[no_mangle]
pub extern "C" fn coreria_get_match_players(
    match_id: c_int,
    player_states: *mut CPlayerState,
    max_players: c_int,
) -> c_int {
    if player_states.is_null() || max_players <= 0 {
        return -1;
    }

    match crate::with_engine(|engine| {
        // TODO: Query all players in the specified match
        let player_count = engine.get_player_count().min(max_players as usize);
        
        // Fill the output array with player data
        for i in 0..player_count {
            let player_state = CPlayerState {
                player_id: i as c_int,
                x: (i as f32) * 2.0 - 4.0, // Spread players out
                y: 2.0,
                z: 0.0,
                qx: 0.0, qy: 0.0, qz: 0.0, qw: 1.0,
                vx: 0.0, vy: 0.0, vz: 0.0,
                health: 100.0,
                max_health: 100.0,
                style_id: (i % 4) as c_int, // Cycle through styles
                is_alive: 1,
                is_blocking: 0,
            };
            
            unsafe {
                *player_states.offset(i as isize) = player_state;
            }
        }
        
        player_count as c_int
    }) {
        Ok(count) => count,
        Err(_) => -1,
    }
}

/// Trigger a player's special ability
/// player_id: Player to activate special for
/// target_x, target_y, target_z: Target position for ability (if applicable)
/// Returns 0 on success, negative on error (e.g., cooldown not ready)
#[no_mangle]
pub extern "C" fn coreria_use_special_ability(
    player_id: c_int,
    target_x: c_float,
    target_y: c_float,
    target_z: c_float,
) -> c_int {
    let target_pos = bevy::math::Vec3::new(target_x, target_y, target_z);
    
    match crate::with_engine(|engine| {
        // TODO: Trigger special ability based on player's fighting style
        println!("Player {} using special ability targeting {:?}", player_id, target_pos);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Check if a player's special ability is ready
/// player_id: Player to check
/// Returns 1 if ready, 0 if on cooldown, negative on error
#[no_mangle]
pub extern "C" fn coreria_is_special_ready(player_id: c_int) -> c_int {
    match crate::with_engine(|engine| {
        // TODO: Check StyleProperties component for special_ready flag
        true // Placeholder - always ready for now
    }) {
        Ok(ready) => if ready { 1 } else { 0 },
        Err(_) => -1,
    }
}