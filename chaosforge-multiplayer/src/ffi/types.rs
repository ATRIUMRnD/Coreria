/*!
# FFI Types

C-compatible data structures for the FFI boundary.
These match the data model specifications with C-friendly layout.
*/

use std::os::raw::{c_char, c_float, c_int, c_uint};

/// C-compatible player state structure
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CPlayerState {
    pub player_id: c_uint,
    pub x: c_float,
    pub y: c_float, 
    pub z: c_float,
    pub qx: c_float,  // Rotation quaternion
    pub qy: c_float,
    pub qz: c_float,
    pub qw: c_float,
    pub vx: c_float,  // Velocity
    pub vy: c_float,
    pub vz: c_float,
    pub health: c_float,
    pub style_id: c_int,
    pub is_alive: c_int, // 0 = false, 1 = true
}

impl CPlayerState {
    pub fn from_rust(
        player: &crate::entities::Player,
        transform: &bevy::prelude::Transform,
        health: &crate::entities::Health,
        velocity: bevy::prelude::Vec3,
    ) -> Self {
        Self {
            player_id: player.id,
            x: transform.translation.x,
            y: transform.translation.y,
            z: transform.translation.z,
            qx: transform.rotation.x,
            qy: transform.rotation.y,
            qz: transform.rotation.z,
            qw: transform.rotation.w,
            vx: velocity.x,
            vy: velocity.y,
            vz: velocity.z,
            health: health.current,
            style_id: player.style as c_int,
            is_alive: if health.is_alive { 1 } else { 0 },
        }
    }
}

/// C-compatible match state structure  
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CMatchState {
    pub match_id: c_uint,
    pub status: c_int,        // MatchStatus as integer
    pub player_count: c_int,
    pub max_players: c_int,
    pub time_remaining: c_float,
    pub winner_id: c_int,     // -1 if no winner
}

impl CMatchState {
    pub fn from_rust(match_state: &crate::game::MatchState, elapsed_time: f64) -> Self {
        let status_code = match match_state.status {
            crate::game::MatchStatus::Waiting => 0,
            crate::game::MatchStatus::Starting { .. } => 1,
            crate::game::MatchStatus::InProgress => 2,
            crate::game::MatchStatus::Finished { .. } => 3,
        };

        let time_remaining = if matches!(match_state.status, crate::game::MatchStatus::InProgress) {
            (match_state.time_limit as f64 - elapsed_time).max(0.0) as c_float
        } else {
            match_state.time_limit
        };

        Self {
            match_id: match_state.match_id,
            status: status_code,
            player_count: match_state.connected_players.len() as c_int,
            max_players: match_state.max_players as c_int,
            time_remaining,
            winner_id: match_state.winner.map(|id| id as c_int).unwrap_or(-1),
        }
    }
}

/// C-compatible input action
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CInputAction {
    pub action_type: c_int,   // InputAction as integer
    pub param1: c_float,      // Additional parameter (e.g., movement strength)
    pub param2: c_float,      // Additional parameter  
}

impl CInputAction {
    pub fn to_rust(self) -> Option<crate::entities::InputAction> {
        match self.action_type {
            0 => Some(crate::entities::InputAction::MoveForward),
            1 => Some(crate::entities::InputAction::MoveBackward),
            2 => Some(crate::entities::InputAction::MoveLeft),
            3 => Some(crate::entities::InputAction::MoveRight),
            4 => Some(crate::entities::InputAction::Jump),
            5 => Some(crate::entities::InputAction::Crouch),
            6 => Some(crate::entities::InputAction::Punch),
            7 => Some(crate::entities::InputAction::Kick),
            8 => Some(crate::entities::InputAction::Block),
            9 => Some(crate::entities::InputAction::Grab),
            10 => Some(crate::entities::InputAction::Special1),
            11 => Some(crate::entities::InputAction::Special2),
            12 => Some(crate::entities::InputAction::CameraLeft),
            13 => Some(crate::entities::InputAction::CameraRight),
            14 => Some(crate::entities::InputAction::Menu),
            _ => None,
        }
    }

    pub fn from_rust(action: crate::entities::InputAction) -> Self {
        let action_type = match action {
            crate::entities::InputAction::MoveForward => 0,
            crate::entities::InputAction::MoveBackward => 1,
            crate::entities::InputAction::MoveLeft => 2,
            crate::entities::InputAction::MoveRight => 3,
            crate::entities::InputAction::Jump => 4,
            crate::entities::InputAction::Crouch => 5,
            crate::entities::InputAction::Punch => 6,
            crate::entities::InputAction::Kick => 7,
            crate::entities::InputAction::Block => 8,
            crate::entities::InputAction::Grab => 9,
            crate::entities::InputAction::Special1 => 10,
            crate::entities::InputAction::Special2 => 11,
            crate::entities::InputAction::CameraLeft => 12,
            crate::entities::InputAction::CameraRight => 13,
            crate::entities::InputAction::Menu => 14,
        };

        Self {
            action_type,
            param1: 0.0,
            param2: 0.0,
        }
    }
}

/// C-compatible performance metrics
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CPerformanceMetrics {
    pub frame_time_ms: c_float,
    pub physics_time_ms: c_float,
    pub network_latency_ms: c_float,
    pub update_rate: c_float,
    pub player_count: c_int,
    pub memory_usage_mb: c_int,
}

impl CPerformanceMetrics {
    pub fn from_rust(metrics: crate::PerformanceMetrics) -> Self {
        Self {
            frame_time_ms: metrics.frame_time.as_millis() as c_float,
            physics_time_ms: metrics.physics_time.as_millis() as c_float,
            network_latency_ms: metrics.network_latency.as_millis() as c_float,
            update_rate: metrics.update_rate,
            player_count: metrics.player_count as c_int,
            memory_usage_mb: (metrics.memory_usage / (1024 * 1024)) as c_int,
        }
    }
}

/// Fighting style constants for C code
pub const STYLE_BRAWLER: c_int = 0;
pub const STYLE_STRIKER: c_int = 1;
pub const STYLE_PHANTOM: c_int = 2;
pub const STYLE_TITAN: c_int = 3;

/// Convert C style ID to Rust enum
pub fn style_from_c_int(style_id: c_int) -> crate::entities::FightingStyle {
    match style_id {
        STYLE_BRAWLER => crate::entities::FightingStyle::Brawler,
        STYLE_STRIKER => crate::entities::FightingStyle::Striker,
        STYLE_PHANTOM => crate::entities::FightingStyle::Phantom,
        STYLE_TITAN => crate::entities::FightingStyle::Titan,
        _ => crate::entities::FightingStyle::Brawler, // Default fallback
    }
}

/// Convert Rust fighting style to C int
pub fn style_to_c_int(style: crate::entities::FightingStyle) -> c_int {
    match style {
        crate::entities::FightingStyle::Brawler => STYLE_BRAWLER,
        crate::entities::FightingStyle::Striker => STYLE_STRIKER,
        crate::entities::FightingStyle::Phantom => STYLE_PHANTOM,
        crate::entities::FightingStyle::Titan => STYLE_TITAN,
    }
}