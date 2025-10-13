/*!
# ChaosForge Multiplayer Engine

A high-performance multiplayer physics engine for Gang Beasts-style ragdoll combat.

## Architecture

- **ECS**: Bevy Entity-Component-System for game state
- **Physics**: Rapier3D for stable ragdoll simulation  
- **Network**: bevy_renet for real-time multiplayer
- **FFI**: Safe C integration for legacy game compatibility

## Features

- 4-8 player multiplayer arena combat
- 4 unique fighting styles with distinct physics
- Ragdoll-based character system (6 bodies + joints)
- Client prediction with server reconciliation
- 60 FPS target with <16ms physics steps

## Usage

```rust
use chaosforge_multiplayer::*;

// Initialize engine
let mut engine = MultiplayerEngine::new();
engine.create_match(8);
engine.spawn_player(FightingStyle::Brawler);
```

## C FFI Interface

```c
#include "chaosforge_multiplayer.h"

int32_t match_id = coreria_create_match(4);
int32_t player_id = coreria_add_player(match_id, STYLE_STRIKER);
coreria_step_simulation(0.016f); // 60 FPS
```
*/

pub mod entities;
pub mod physics;
pub mod network;
pub mod game;
pub mod ffi;

// Re-export core types
pub use entities::*;
pub use physics::*;
pub use network::*; 
pub use game::*;

use bevy::prelude::*;
use once_cell::sync::Lazy;
use std::sync::Mutex;

/// Global engine instance for FFI access
static ENGINE_INSTANCE: Lazy<Mutex<Option<MultiplayerEngine>>> = 
    Lazy::new(|| Mutex::new(None));

/// Core multiplayer engine combining ECS, physics, and networking
#[derive(Resource)]
pub struct MultiplayerEngine {
    pub app: App,
    pub is_server: bool,
    pub tick_rate: f64,
    pub last_update: std::time::Instant,
}

impl Default for MultiplayerEngine {
    fn default() -> Self {
        Self::new()
    }
}

impl MultiplayerEngine {
    /// Create new multiplayer engine instance
    pub fn new() -> Self {
        let mut app = App::new();
        
        app
            .add_plugins(MinimalPlugins)
            .add_plugins(bevy::time::TimePlugin)
            .add_plugins((
                entities::PlayerPlugin,
                physics::PhysicsPlugin,
                game::GameStatePlugin,
            ));

        // Add networking plugins if feature enabled
        #[cfg(feature = "multiplayer-physics")]
        {
            app.add_plugins(network::NetworkPlugin);
        }

        Self {
            app,
            is_server: false,
            tick_rate: 60.0,
            last_update: std::time::Instant::now(),
        }
    }

    /// Initialize as server instance
    pub fn init_server(&mut self, port: u16) -> Result<(), String> {
        self.is_server = true;
        info!("Starting server on port {}", port);
        
        #[cfg(feature = "multiplayer-physics")]
        {
            // Server-specific initialization
            self.app.insert_resource(network::ServerConfig { port });
        }
        
        Ok(())
    }

    /// Initialize as client instance  
    pub fn init_client(&mut self, server_addr: &str) -> Result<(), String> {
        self.is_server = false;
        info!("Connecting to server at {}", server_addr);
        
        #[cfg(feature = "multiplayer-physics")]
        {
            // Client-specific initialization
            self.app.insert_resource(network::ClientConfig {
                server_addr: server_addr.to_string(),
            });
        }
        
        Ok(())
    }

    /// Create a new match instance
    pub fn create_match(&mut self, max_players: usize) -> u32 {
        let match_id = uuid::Uuid::new_v4().as_u128() as u32;
        
        self.app.world.spawn((
            game::MatchState::new(match_id, max_players),
            Name::new(format!("Match_{}", match_id)),
        ));
        
        info!("Created match {} for {} players", match_id, max_players);
        match_id
    }

    /// Add player to match
    pub fn add_player(&mut self, match_id: u32, style: entities::FightingStyle) -> u32 {
        let player_id = uuid::Uuid::new_v4().as_u128() as u32;
        
        self.app.world.spawn((
            entities::Player::new(player_id, style),
            entities::RagdollBody::default(),
            entities::Health::new(100.0),
            entities::NetworkState::default(),
            Name::new(format!("Player_{}", player_id)),
        ));
        
        info!("Added player {} with style {:?} to match {}", 
              player_id, style, match_id);
        player_id
    }

    /// Step physics and game simulation
    pub fn step_simulation(&mut self, delta_time: f32) -> bool {
        let now = std::time::Instant::now();
        let elapsed = now.duration_since(self.last_update).as_secs_f64();
        
        // Maintain target tick rate
        if elapsed < (1.0 / self.tick_rate) {
            return false; // Not time for next update
        }
        
        self.last_update = now;
        
        // Update Bevy systems
        self.app.update();
        
        true
    }

    /// Get player count across all matches
    pub fn get_player_count(&self) -> usize {
        self.app.world
            .query::<&entities::Player>()
            .iter(&self.app.world)
            .count()
    }

    /// Get performance metrics
    pub fn get_performance_metrics(&self) -> PerformanceMetrics {
        PerformanceMetrics {
            frame_time: std::time::Duration::from_millis(16), // Target 60 FPS
            physics_time: std::time::Duration::from_millis(4), // Target <16ms
            network_latency: std::time::Duration::from_millis(50),
            update_rate: self.tick_rate as f32,
            player_count: self.get_player_count(),
            memory_usage: 0, // TODO: Implement memory tracking
        }
    }
}

/// Performance monitoring data
#[derive(Debug, Clone)]
pub struct PerformanceMetrics {
    pub frame_time: std::time::Duration,
    pub physics_time: std::time::Duration,  
    pub network_latency: std::time::Duration,
    pub update_rate: f32,
    pub player_count: usize,
    pub memory_usage: usize,
}

/// Initialize global engine instance (called from FFI)
pub fn initialize_engine() -> Result<(), String> {
    let mut engine_guard = ENGINE_INSTANCE.lock()
        .map_err(|e| format!("Failed to lock engine mutex: {}", e))?;
    
    if engine_guard.is_some() {
        return Err("Engine already initialized".to_string());
    }
    
    *engine_guard = Some(MultiplayerEngine::new());
    info!("Multiplayer engine initialized successfully");
    Ok(())
}

/// Get reference to global engine instance
pub fn with_engine<F, R>(f: F) -> Result<R, String>
where
    F: FnOnce(&mut MultiplayerEngine) -> R,
{
    let mut engine_guard = ENGINE_INSTANCE.lock()
        .map_err(|e| format!("Failed to lock engine mutex: {}", e))?;
    
    match engine_guard.as_mut() {
        Some(engine) => Ok(f(engine)),
        None => Err("Engine not initialized".to_string()),
    }
}

/// Shutdown engine and cleanup resources
pub fn shutdown_engine() -> Result<(), String> {
    let mut engine_guard = ENGINE_INSTANCE.lock()
        .map_err(|e| format!("Failed to lock engine mutex: {}", e))?;
    
    *engine_guard = None;
    info!("Multiplayer engine shutdown complete");
    Ok(())
}