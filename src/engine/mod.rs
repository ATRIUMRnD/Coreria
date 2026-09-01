/*!
# Coreria Engine Core

The main engine implementation using Bevy ECS for system orchestration.
Handles initialization, main loop, and integration between subsystems.
*/

use bevy::prelude::*;
use log::{info, error, debug};
use std::time::Duration;

pub mod events;
pub mod resources;
pub mod systems;

use crate::graphics::GraphicsPlugin;
use crate::physics::PhysicsPlugin;
use crate::input::InputPlugin;

/// Main engine state
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EngineState {
    Initializing,
    Running,
    Paused,
    Shutdown,
}

/// Core Coreria Engine
pub struct CoreiaEngine {
    app: App,
    state: EngineState,
}

impl CoreiaEngine {
    /// Create a new engine instance
    pub fn new() -> Self {
        info!("🏗️  Creating new Coreria Engine instance");
        
        let mut app = App::new();
        
        // Add core Bevy plugins
        app.add_plugins(DefaultPlugins.set(WindowPlugin {
            primary_window: Some(Window {
                title: "Coreria Game Engine".to_string(),
                resolution: (1920.0, 1080.0).into(),
                ..default()
            }),
            ..default()
        }));
        
        // Add engine-specific plugins
        app.add_plugins((
            GraphicsPlugin,
            PhysicsPlugin,
            InputPlugin,
        ));
        
        // Add engine resources
        app.insert_resource(EngineConfig::default())
           .insert_resource(PerformanceMetrics::default());
        
        // Add core systems
        app.add_systems(Startup, systems::engine_startup)
           .add_systems(Update, (
               systems::update_performance_metrics,
               systems::handle_engine_events,
           ))
           .add_systems(PostUpdate, systems::cleanup_systems);
        
        Self {
            app,
            state: EngineState::Initializing,
        }
    }
    
    /// Initialize the engine
    pub fn initialize(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        info!("⚙️  Initializing Coreria Engine...");
        
        // Validate system requirements
        self.validate_system()?;
        
        // Initialize subsystems
        debug!("🎨 Initializing graphics subsystem...");
        debug!("⚡ Initializing physics subsystem...");
        debug!("🎮 Initializing input subsystem...");
        
        self.state = EngineState::Running;
        info!("✅ Engine initialization complete");
        
        Ok(())
    }
    
    /// Run the main engine loop
    pub fn run(mut self) {
        info!("🚀 Starting engine main loop...");
        self.app.run();
    }
    
    /// Validate system requirements
    fn validate_system(&self) -> Result<(), Box<dyn std::error::Error>> {
        // Check OpenGL support
        debug!("🔍 Checking OpenGL support...");
        // TODO: Actual OpenGL capability detection
        
        // Check available memory
        debug!("🔍 Checking system memory...");
        // TODO: Memory availability check
        
        Ok(())
    }
    
    /// Get current engine state
    pub fn state(&self) -> EngineState {
        self.state
    }
}

impl Default for CoreiaEngine {
    fn default() -> Self {
        Self::new()
    }
}

/// Engine configuration
#[derive(Resource, Debug, Clone)]
pub struct EngineConfig {
    pub target_fps: f64,
    pub vsync_enabled: bool,
    pub physics_timestep: Duration,
    pub max_frame_time: Duration,
}

impl Default for EngineConfig {
    fn default() -> Self {
        Self {
            target_fps: 60.0,
            vsync_enabled: true,
            physics_timestep: Duration::from_millis(16), // ~60 FPS
            max_frame_time: Duration::from_millis(33),   // ~30 FPS minimum
        }
    }
}

/// Performance monitoring
#[derive(Resource, Debug, Default)]
pub struct PerformanceMetrics {
    pub frame_count: u64,
    pub avg_frame_time: Duration,
    pub min_frame_time: Duration,
    pub max_frame_time: Duration,
    pub last_fps: f64,
}

impl PerformanceMetrics {
    pub fn update(&mut self, frame_time: Duration) {
        self.frame_count += 1;
        
        if self.frame_count == 1 {
            self.min_frame_time = frame_time;
            self.max_frame_time = frame_time;
            self.avg_frame_time = frame_time;
        } else {
            self.min_frame_time = self.min_frame_time.min(frame_time);
            self.max_frame_time = self.max_frame_time.max(frame_time);
            
            // Rolling average
            let alpha = 0.1; // Smoothing factor
            let current_ms = self.avg_frame_time.as_secs_f64();
            let new_ms = frame_time.as_secs_f64();
            self.avg_frame_time = Duration::from_secs_f64(current_ms * (1.0 - alpha) + new_ms * alpha);
        }
        
        self.last_fps = 1.0 / frame_time.as_secs_f64();
    }
}