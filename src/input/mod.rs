/*!
# Input Subsystem

Input handling and player control management for Coreria engine.
Integrates with Bevy's input systems and provides game-specific abstractions.
*/

use bevy::prelude::*;
use log::{info, debug};
use std::collections::HashMap;

pub mod gamepad;
pub mod keyboard;
pub mod mouse;

/// Input plugin for the engine
pub struct InputPlugin;

impl Plugin for InputPlugin {
    fn build(&self, app: &mut App) {
        info!("🎮 Initializing Input Plugin");
        
        app
            // Add input resources
            .insert_resource(InputSettings::default())
            .insert_resource(InputState::default())
            
            // Add input systems
            .add_systems(Startup, input_startup)
            .add_systems(Update, (
                process_keyboard_input,
                process_mouse_input,
                process_gamepad_input,
                update_input_state,
            ));
    }
}

/// Input configuration
#[derive(Resource, Debug, Clone)]
pub struct InputSettings {
    pub mouse_sensitivity: f32,
    pub keyboard_repeat_delay: f32,
    pub gamepad_deadzone: f32,
    pub invert_mouse_y: bool,
}

impl Default for InputSettings {
    fn default() -> Self {
        Self {
            mouse_sensitivity: 1.0,
            keyboard_repeat_delay: 0.5,
            gamepad_deadzone: 0.1,
            invert_mouse_y: false,
        }
    }
}

/// Current input state
#[derive(Resource, Debug, Default)]
pub struct InputState {
    pub mouse_position: Vec2,
    pub mouse_delta: Vec2,
    pub active_keys: Vec<KeyCode>,
    pub active_buttons: Vec<MouseButton>,
    pub connected_gamepads: Vec<Gamepad>,
}

/// Game action abstraction
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum GameAction {
    // Movement
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Jump,
    Sprint,
    
    // Combat (matching ChaosForge design)
    Punch,
    Kick,
    Block,
    Grapple,
    
    // System
    Pause,
    Menu,
    Quit,
}

/// Input binding configuration
#[derive(Resource, Debug)]
pub struct InputBindings {
    pub keyboard_bindings: HashMap<KeyCode, GameAction>,
    pub mouse_bindings: HashMap<MouseButton, GameAction>,
    pub gamepad_bindings: HashMap<GamepadButtonType, GameAction>,
}

impl Default for InputBindings {
    fn default() -> Self {
        let mut keyboard_bindings = HashMap::new();
        
        // Movement bindings (WASD + extras)
        keyboard_bindings.insert(KeyCode::W, GameAction::MoveForward);
        keyboard_bindings.insert(KeyCode::S, GameAction::MoveBackward);
        keyboard_bindings.insert(KeyCode::A, GameAction::MoveLeft);
        keyboard_bindings.insert(KeyCode::D, GameAction::MoveRight);
        keyboard_bindings.insert(KeyCode::Space, GameAction::Jump);
        keyboard_bindings.insert(KeyCode::ShiftLeft, GameAction::Sprint);
        
        // Combat bindings
        keyboard_bindings.insert(KeyCode::Q, GameAction::Punch);
        keyboard_bindings.insert(KeyCode::E, GameAction::Kick);
        keyboard_bindings.insert(KeyCode::ControlLeft, GameAction::Block);
        keyboard_bindings.insert(KeyCode::R, GameAction::Grapple);
        
        // System bindings
        keyboard_bindings.insert(KeyCode::Escape, GameAction::Menu);
        keyboard_bindings.insert(KeyCode::P, GameAction::Pause);
        
        let mut mouse_bindings = HashMap::new();
        mouse_bindings.insert(MouseButton::Left, GameAction::Punch);
        mouse_bindings.insert(MouseButton::Right, GameAction::Block);
        mouse_bindings.insert(MouseButton::Middle, GameAction::Grapple);
        
        let mut gamepad_bindings = HashMap::new();
        gamepad_bindings.insert(GamepadButtonType::South, GameAction::Jump); // A/X
        gamepad_bindings.insert(GamepadButtonType::East, GameAction::Punch); // B/Circle
        gamepad_bindings.insert(GamepadButtonType::West, GameAction::Kick);  // X/Square
        gamepad_bindings.insert(GamepadButtonType::North, GameAction::Block); // Y/Triangle
        
        Self {
            keyboard_bindings,
            mouse_bindings,
            gamepad_bindings,
        }
    }
}

/// Input startup system
fn input_startup(
    mut commands: Commands,
    settings: Res<InputSettings>,
) {
    info!("🎬 Input system startup");
    debug!("🖱️  Mouse sensitivity: {}", settings.mouse_sensitivity);
    debug!("⌨️  Keyboard repeat delay: {}", settings.keyboard_repeat_delay);
    debug!("🎮 Gamepad deadzone: {}", settings.gamepad_deadzone);
    
    // Add input bindings as a resource
    commands.insert_resource(InputBindings::default());
}

/// Process keyboard input
fn process_keyboard_input(
    keyboard_input: Res<Input<KeyCode>>,
    bindings: Res<InputBindings>,
    mut input_state: ResMut<InputState>,
) {
    // Update active keys
    input_state.active_keys.clear();
    for key in keyboard_input.get_pressed() {
        input_state.active_keys.push(*key);
    }
    
    // Process game actions
    for (key, action) in &bindings.keyboard_bindings {
        if keyboard_input.just_pressed(*key) {
            debug!("⌨️  Action triggered: {:?} ({})", action, format!("{:?}", key));
        }
    }
}

/// Process mouse input  
fn process_mouse_input(
    mouse_button_input: Res<Input<MouseButton>>,
    mut mouse_motion: EventReader<CursorMoved>,
    bindings: Res<InputBindings>,
    mut input_state: ResMut<InputState>,
    settings: Res<InputSettings>,
) {
    // Update mouse position and delta
    for event in mouse_motion.read() {
        let delta = event.position - input_state.mouse_position;
        input_state.mouse_delta = delta * settings.mouse_sensitivity;
        input_state.mouse_position = event.position;
    }
    
    // Update active mouse buttons
    input_state.active_buttons.clear();
    for button in mouse_button_input.get_pressed() {
        input_state.active_buttons.push(*button);
    }
    
    // Process mouse actions
    for (button, action) in &bindings.mouse_bindings {
        if mouse_button_input.just_pressed(*button) {
            debug!("🖱️  Action triggered: {:?} ({:?})", action, button);
        }
    }
}

/// Process gamepad input
fn process_gamepad_input(
    gamepad_input: Res<Input<GamepadButton>>,
    gamepads: Res<Gamepads>,
    bindings: Res<InputBindings>,
    mut input_state: ResMut<InputState>,
) {
    // Update connected gamepads
    input_state.connected_gamepads = gamepads.iter().collect();
    
    // Process gamepad button actions
    for gamepad in gamepads.iter() {
        for (button_type, action) in &bindings.gamepad_bindings {
            let button = GamepadButton::new(gamepad, *button_type);
            if gamepad_input.just_pressed(button) {
                debug!("🎮 Gamepad action triggered: {:?} ({:?})", action, button_type);
            }
        }
    }
}

/// Update overall input state
fn update_input_state(
    input_state: Res<InputState>,
) {
    // Log input activity periodically for debugging
    // This could be used for input recording/playback systems
    if !input_state.active_keys.is_empty() || !input_state.active_buttons.is_empty() {
        // Periodic debug output could go here
    }
}