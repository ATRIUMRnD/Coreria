use std::os::raw::{c_char, c_float, c_int};

/// C-compatible fighting style constants
pub const STYLE_BRAWLER: c_int = 0;
pub const STYLE_STRIKER: c_int = 1;
pub const STYLE_PHANTOM: c_int = 2;
pub const STYLE_TITAN: c_int = 3;

/// C-compatible player state structure
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CPlayerState {
    pub player_id: c_int,
    pub x: c_float,
    pub y: c_float,
    pub z: c_float,
    pub qx: c_float, // Rotation quaternion
    pub qy: c_float,
    pub qz: c_float,
    pub qw: c_float,
    pub vx: c_float, // Velocity
    pub vy: c_float,
    pub vz: c_float,
    pub health: c_float,
    pub max_health: c_float,
    pub style_id: c_int,
    pub is_alive: c_int, // 0 = false, 1 = true
    pub is_blocking: c_int,
}

impl Default for CPlayerState {
    fn default() -> Self {
        Self {
            player_id: 0,
            x: 0.0, y: 0.0, z: 0.0,
            qx: 0.0, qy: 0.0, qz: 0.0, qw: 1.0,
            vx: 0.0, vy: 0.0, vz: 0.0,
            health: 100.0,
            max_health: 100.0,
            style_id: STYLE_BRAWLER,
            is_alive: 1,
            is_blocking: 0,
        }
    }
}

/// C-compatible match state structure
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CMatchState {
    pub match_id: c_int,
    pub status: c_int,        // MatchStatus enum as int
    pub player_count: c_int,
    pub max_players: c_int,
    pub time_remaining: c_float,
    pub winner_id: c_int,     // -1 if no winner
    pub players: [CPlayerState; 8], // Fixed array for simplicity
}

impl Default for CMatchState {
    fn default() -> Self {
        Self {
            match_id: 0,
            status: 0, // Waiting
            player_count: 0,
            max_players: 8,
            time_remaining: 300.0, // 5 minutes
            winner_id: -1,
            players: [CPlayerState::default(); 8],
        }
    }
}

/// C-compatible input action structure
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CInputAction {
    pub frame_number: c_int,
    pub action_type: c_int,   // InputAction enum as int
    pub param1: c_float,      // Action parameters (e.g., movement direction)
    pub param2: c_float,
}

/// Input action constants
pub const ACTION_MOVE_FORWARD: c_int = 0;
pub const ACTION_MOVE_BACKWARD: c_int = 1;
pub const ACTION_MOVE_LEFT: c_int = 2;
pub const ACTION_MOVE_RIGHT: c_int = 3;
pub const ACTION_JUMP: c_int = 4;
pub const ACTION_CROUCH: c_int = 5;
pub const ACTION_PUNCH: c_int = 6;
pub const ACTION_KICK: c_int = 7;
pub const ACTION_BLOCK: c_int = 8;
pub const ACTION_GRAB: c_int = 9;
pub const ACTION_SPECIAL1: c_int = 10;
pub const ACTION_SPECIAL2: c_int = 11;

/// C-compatible performance metrics
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct CPerformanceMetrics {
    pub frame_time_ms: c_float,
    pub physics_time_ms: c_float,
    pub network_latency_ms: c_float,
    pub update_rate: c_float,
    pub player_count: c_int,
    pub memory_usage_bytes: c_int,
}

impl Default for CPerformanceMetrics {
    fn default() -> Self {
        Self {
            frame_time_ms: 16.0,   // Target 60 FPS
            physics_time_ms: 4.0,  // Target <16ms
            network_latency_ms: 50.0,
            update_rate: 60.0,
            player_count: 0,
            memory_usage_bytes: 0,
        }
    }
}

/// Match status constants
pub const MATCH_WAITING: c_int = 0;
pub const MATCH_STARTING: c_int = 1;
pub const MATCH_IN_PROGRESS: c_int = 2;
pub const MATCH_FINISHED: c_int = 3;

/// Convert Rust FightingStyle to C int
pub fn fighting_style_to_c(style: crate::entities::FightingStyle) -> c_int {
    match style {
        crate::entities::FightingStyle::Brawler => STYLE_BRAWLER,
        crate::entities::FightingStyle::Striker => STYLE_STRIKER,
        crate::entities::FightingStyle::Phantom => STYLE_PHANTOM,
        crate::entities::FightingStyle::Titan => STYLE_TITAN,
    }
}

/// Convert C int to Rust FightingStyle
pub fn c_to_fighting_style(style_id: c_int) -> Option<crate::entities::FightingStyle> {
    match style_id {
        STYLE_BRAWLER => Some(crate::entities::FightingStyle::Brawler),
        STYLE_STRIKER => Some(crate::entities::FightingStyle::Striker),
        STYLE_PHANTOM => Some(crate::entities::FightingStyle::Phantom),
        STYLE_TITAN => Some(crate::entities::FightingStyle::Titan),
        _ => None,
    }
}

/// Convert Rust InputAction to C int
pub fn input_action_to_c(action: crate::entities::InputAction) -> c_int {
    match action {
        crate::entities::InputAction::MoveForward => ACTION_MOVE_FORWARD,
        crate::entities::InputAction::MoveBackward => ACTION_MOVE_BACKWARD,
        crate::entities::InputAction::MoveLeft => ACTION_MOVE_LEFT,
        crate::entities::InputAction::MoveRight => ACTION_MOVE_RIGHT,
        crate::entities::InputAction::Jump => ACTION_JUMP,
        crate::entities::InputAction::Crouch => ACTION_CROUCH,
        crate::entities::InputAction::Punch => ACTION_PUNCH,
        crate::entities::InputAction::Kick => ACTION_KICK,
        crate::entities::InputAction::Block => ACTION_BLOCK,
        crate::entities::InputAction::Grab => ACTION_GRAB,
        crate::entities::InputAction::Special1 => ACTION_SPECIAL1,
        crate::entities::InputAction::Special2 => ACTION_SPECIAL2,
        _ => -1, // Unknown action
    }
}

/// Convert C int to Rust InputAction
pub fn c_to_input_action(action_type: c_int) -> Option<crate::entities::InputAction> {
    match action_type {
        ACTION_MOVE_FORWARD => Some(crate::entities::InputAction::MoveForward),
        ACTION_MOVE_BACKWARD => Some(crate::entities::InputAction::MoveBackward),
        ACTION_MOVE_LEFT => Some(crate::entities::InputAction::MoveLeft),
        ACTION_MOVE_RIGHT => Some(crate::entities::InputAction::MoveRight),
        ACTION_JUMP => Some(crate::entities::InputAction::Jump),
        ACTION_CROUCH => Some(crate::entities::InputAction::Crouch),
        ACTION_PUNCH => Some(crate::entities::InputAction::Punch),
        ACTION_KICK => Some(crate::entities::InputAction::Kick),
        ACTION_BLOCK => Some(crate::entities::InputAction::Block),
        ACTION_GRAB => Some(crate::entities::InputAction::Grab),
        ACTION_SPECIAL1 => Some(crate::entities::InputAction::Special1),
        ACTION_SPECIAL2 => Some(crate::entities::InputAction::Special2),
        _ => None,
    }
}