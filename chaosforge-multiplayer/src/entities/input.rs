use bevy::prelude::*;
use serde::{Deserialize, Serialize};
use std::collections::VecDeque;

/// Input buffer for storing and validating player inputs
#[derive(Component, Debug, Clone, Serialize, Deserialize)]
pub struct InputBuffer {
    pub frames: VecDeque<InputFrame>,
    pub max_frames: usize,
    pub current_frame: u64,
}

impl Default for InputBuffer {
    fn default() -> Self {
        Self {
            frames: VecDeque::new(),
            max_frames: 180, // 3 seconds at 60 FPS
            current_frame: 0,
        }
    }
}

impl InputBuffer {
    pub fn new(max_frames: usize) -> Self {
        Self {
            frames: VecDeque::new(),
            max_frames,
            current_frame: 0,
        }
    }

    /// Add new input frame to buffer
    pub fn push_input(&mut self, actions: Vec<InputAction>) {
        let frame = InputFrame {
            frame_number: self.current_frame,
            timestamp_secs: std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap_or_default().as_secs_f64(),
            actions,
        };

        self.frames.push_back(frame);
        self.current_frame += 1;

        // Remove old frames
        while self.frames.len() > self.max_frames {
            self.frames.pop_front();
        }
    }

    /// Get inputs from a specific frame range
    pub fn get_inputs_in_range(&self, start_frame: u64, end_frame: u64) -> Vec<&InputFrame> {
        self.frames
            .iter()
            .filter(|frame| frame.frame_number >= start_frame && frame.frame_number <= end_frame)
            .collect()
    }

    /// Get most recent input frame
    pub fn latest_frame(&self) -> Option<&InputFrame> {
        self.frames.back()
    }

    /// Check if a specific action was performed in the last N frames
    pub fn action_in_recent_frames(&self, action: InputAction, frames_back: usize) -> bool {
        let start_frame = self.current_frame.saturating_sub(frames_back as u64);
        
        for frame in &self.frames {
            if frame.frame_number >= start_frame {
                if frame.actions.contains(&action) {
                    return true;
                }
            }
        }
        false
    }
}

/// Single frame of input data
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct InputFrame {
    pub frame_number: u64,
    pub timestamp_secs: f64, // Unix timestamp for network serialization
    pub actions: Vec<InputAction>,
}

impl Default for InputFrame {
    fn default() -> Self {
        use std::time::{SystemTime, UNIX_EPOCH};
        Self {
            frame_number: 0,
            timestamp_secs: SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default().as_secs_f64(),
            actions: Vec::new(),
        }
    }
}

/// Individual input actions
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Reflect)]
pub enum InputAction {
    // Movement
    MoveForward,
    MoveBackward,
    MoveLeft, 
    MoveRight,
    Jump,
    Crouch,
    
    // Combat
    Punch,
    Kick,
    Block,
    Grab,
    
    // Special abilities (style-specific)
    Special1,
    Special2,
    
    // Camera/UI
    CameraLeft,
    CameraRight,
    Menu,
}

impl InputAction {
    /// Get movement vector from input action
    pub fn movement_vector(self) -> Option<Vec2> {
        match self {
            InputAction::MoveForward => Some(Vec2::new(0.0, 1.0)),
            InputAction::MoveBackward => Some(Vec2::new(0.0, -1.0)), 
            InputAction::MoveLeft => Some(Vec2::new(-1.0, 0.0)),
            InputAction::MoveRight => Some(Vec2::new(1.0, 0.0)),
            _ => None,
        }
    }

    /// Check if action is a combat action
    pub fn is_combat_action(self) -> bool {
        matches!(self, 
            InputAction::Punch | 
            InputAction::Kick | 
            InputAction::Block | 
            InputAction::Grab |
            InputAction::Special1 |
            InputAction::Special2
        )
    }

    /// Check if action is a movement action
    pub fn is_movement_action(self) -> bool {
        matches!(self,
            InputAction::MoveForward |
            InputAction::MoveBackward |
            InputAction::MoveLeft |
            InputAction::MoveRight |
            InputAction::Jump |
            InputAction::Crouch
        )
    }
}

/// System to process input buffers and convert to game actions
pub fn process_input_buffer(
    mut input_query: Query<(Entity, &mut InputBuffer, &crate::entities::Player)>,
    mut combat_query: Query<&mut crate::entities::CombatState>,
    mut transform_query: Query<&mut Transform>,
    time: Res<Time>,
) {
    for (entity, input_buffer, player) in input_query.iter_mut() {
        if let Some(latest_frame) = input_buffer.latest_frame().cloned() {
            // Process movement actions
            let mut movement = Vec2::ZERO;
            for action in &latest_frame.actions {
                if let Some(move_vec) = action.movement_vector() {
                    movement += move_vec;
                }
            }

            // Apply movement to transform (placeholder - would integrate with physics)
            if let Ok(mut transform) = transform_query.get_mut(entity) {
                let speed = player.style.speed_multiplier() * 5.0 * time.delta_seconds();
                let movement_3d = Vec3::new(movement.x, 0.0, -movement.y) * speed;
                transform.translation += movement_3d;
            }

            // Process combat actions
            if let Ok(mut combat_state) = combat_query.get_mut(entity) {
                for action in &latest_frame.actions {
                    if action.is_combat_action() {
                        match action {
                            InputAction::Block => {
                                combat_state.is_blocking = true;
                            },
                            InputAction::Punch | InputAction::Kick => {
                                // Start basic attack combo
                                if combat_state.current_combo.is_none() {
                                    combat_state.current_combo = Some(
                                        crate::entities::ActiveCombo {
                                            combo_type: crate::entities::ComboType::BasicAttack,
                                            sequence: vec![
                                                crate::entities::ComboInput {
                                                    action: crate::entities::CombatAction::Punch,
                                                    timing_frame: 0,
                                                    required_precision: 0.3,
                                                }
                                            ],
                                            current_step: 0,
                                            timing_window: 0.5, // 30 frames at 60 FPS
                                            damage_bonus: 1.0,
                                        }
                                    );
                                }
                            },
                            _ => {}
                        }
                    }
                }
                
                // Clear blocking if not actively held
                if !latest_frame.actions.contains(&InputAction::Block) {
                    combat_state.is_blocking = false;
                }
            }
        }
    }
}

/// Convert Windows key codes to input actions (for FFI integration)
pub fn keycode_to_input_action(keycode: u32) -> Option<InputAction> {
    match keycode {
        // WASD movement
        0x57 => Some(InputAction::MoveForward),  // W
        0x53 => Some(InputAction::MoveBackward), // S  
        0x41 => Some(InputAction::MoveLeft),     // A
        0x44 => Some(InputAction::MoveRight),    // D
        
        // Space for jump
        0x20 => Some(InputAction::Jump),
        
        // Left Shift for crouch
        0xA0 => Some(InputAction::Crouch),
        
        // Mouse buttons (approximate codes)
        0x01 => Some(InputAction::Punch),        // Left mouse
        0x02 => Some(InputAction::Kick),         // Right mouse
        
        // Keyboard combat
        0x46 => Some(InputAction::Block),        // F
        0x47 => Some(InputAction::Grab),         // G
        
        // Special abilities
        0x51 => Some(InputAction::Special1),     // Q
        0x45 => Some(InputAction::Special2),     // E
        
        // Camera
        0x25 => Some(InputAction::CameraLeft),   // Left arrow
        0x27 => Some(InputAction::CameraRight),  // Right arrow
        
        // Menu
        0x1B => Some(InputAction::Menu),         // Escape
        
        _ => None,
    }
}