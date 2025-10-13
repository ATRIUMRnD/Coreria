use std::os::raw::{c_float, c_int};
use std::ptr;
use crate::ffi::ffi_types::*;

/// Create a new match
/// max_players: Maximum number of players (2-8)
/// time_limit: Match duration in seconds (0 = no limit)
/// Returns match ID on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_create_match(max_players: c_int, time_limit: c_float) -> c_int {
    if max_players < 2 || max_players > 8 {
        return -1;
    }
    
    match crate::with_engine(|engine| {
        let match_id = engine.create_match(max_players as usize);
        match_id
    }) {
        Ok(match_id) => {
            println!("Created match {} for {} players ({}s limit)", 
                     match_id, max_players, time_limit);
            match_id as c_int
        },
        Err(_) => -1,
    }
}

/// Get current state of a match
/// match_id: Match to query
/// Returns pointer to match state struct, or null if not found
#[no_mangle]
pub extern "C" fn coreria_get_match_state(match_id: c_int) -> *const CMatchState {
    match crate::with_engine(|engine| {
        // TODO: Query ECS for match entity and convert to C struct
        let match_state = CMatchState {
            match_id,
            status: MATCH_WAITING,
            player_count: 0,
            max_players: 8,
            time_remaining: 300.0,
            winner_id: -1,
            players: [CPlayerState::default(); 8],
        };
        
        // Note: Unsafe memory management - would need proper lifetime handling
        Box::into_raw(Box::new(match_state))
    }) {
        Ok(ptr) => ptr,
        Err(_) => ptr::null(),
    }
}

/// Start a match (if enough players have joined)
/// match_id: Match to start
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_start_match(match_id: c_int) -> c_int {
    match crate::with_engine(|engine| {
        // TODO: Update match status to Starting/InProgress
        println!("Starting match {}", match_id);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// End a match and declare winner
/// match_id: Match to end
/// winner_id: Winning player ID (-1 for no winner/draw)
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_end_match(match_id: c_int, winner_id: c_int) -> c_int {
    match crate::with_engine(|engine| {
        // TODO: Update match status to Finished
        println!("Ending match {} - winner: {}", match_id, winner_id);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Check if a match is full
/// match_id: Match to check
/// Returns 1 if full, 0 if not full, negative on error
#[no_mangle]
pub extern "C" fn coreria_is_match_full(match_id: c_int) -> c_int {
    match crate::with_engine(|engine| {
        // TODO: Check match player count vs max_players
        false // Placeholder
    }) {
        Ok(is_full) => if is_full { 1 } else { 0 },
        Err(_) => -1,
    }
}

/// Get list of all active matches
/// match_ids: Output array of match IDs (allocated by caller)
/// max_matches: Maximum number of matches to return
/// Returns actual number of matches returned, negative on error
#[no_mangle]
pub extern "C" fn coreria_list_matches(match_ids: *mut c_int, max_matches: c_int) -> c_int {
    if match_ids.is_null() || max_matches <= 0 {
        return -1;
    }

    match crate::with_engine(|engine| {
        // TODO: Query ECS for all match entities
        // For now, return a single dummy match
        let match_count = 1.min(max_matches as usize);
        
        for i in 0..match_count {
            unsafe {
                *match_ids.offset(i as isize) = i as c_int;
            }
        }
        
        match_count as c_int
    }) {
        Ok(count) => count,
        Err(_) => -1,
    }
}

/// Set match time limit
/// match_id: Target match
/// time_limit: New time limit in seconds (0 = no limit)
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_set_match_time_limit(match_id: c_int, time_limit: c_float) -> c_int {
    if time_limit < 0.0 {
        return -1;
    }
    
    match crate::with_engine(|engine| {
        // TODO: Update match entity's time_limit field
        println!("Setting match {} time limit to {}s", match_id, time_limit);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}

/// Get match remaining time
/// match_id: Match to check
/// Returns remaining time in seconds, negative on error
#[no_mangle]
pub extern "C" fn coreria_get_match_time_remaining(match_id: c_int) -> c_float {
    match crate::with_engine(|engine| {
        // TODO: Calculate remaining time from match start_time and time_limit
        300.0 // Placeholder - 5 minutes
    }) {
        Ok(time_remaining) => time_remaining,
        Err(_) => -1.0,
    }
}

/// Force respawn a player in their current match
/// player_id: Player to respawn
/// Returns 0 on success, negative on error
#[no_mangle]
pub extern "C" fn coreria_respawn_player(player_id: c_int) -> c_int {
    match crate::with_engine(|engine| {
        // TODO: Reset player health, position to spawn point
        println!("Respawning player {}", player_id);
    }) {
        Ok(_) => 0,
        Err(_) => -1,
    }
}