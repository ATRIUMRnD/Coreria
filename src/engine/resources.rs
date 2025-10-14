/*!
# Engine Resources

Global engine resources and shared state.
*/

use bevy::prelude::*;
use std::collections::HashMap;

/// Global engine statistics
#[derive(Resource, Debug, Default)]
pub struct EngineStats {
    pub uptime_seconds: f64,
    pub total_entities: usize,
    pub active_systems: usize,
    pub memory_usage_mb: f64,
}

/// Engine capabilities detection
#[derive(Resource, Debug, Clone)]
pub struct SystemCapabilities {
    pub opengl_version: String,
    pub graphics_vendor: String,
    pub max_texture_size: u32,
    pub supports_compute_shaders: bool,
    pub available_memory_mb: u64,
}

impl Default for SystemCapabilities {
    fn default() -> Self {
        Self {
            opengl_version: "Unknown".to_string(),
            graphics_vendor: "Unknown".to_string(),
            max_texture_size: 1024,
            supports_compute_shaders: false,
            available_memory_mb: 0,
        }
    }
}

/// Runtime configuration that can be modified
#[derive(Resource, Debug, Clone)]
pub struct RuntimeConfig {
    pub debug_mode: bool,
    pub physics_enabled: bool,
    pub networking_enabled: bool,
    pub audio_enabled: bool,
    pub render_debug_info: bool,
}

impl Default for RuntimeConfig {
    fn default() -> Self {
        Self {
            debug_mode: cfg!(debug_assertions),
            physics_enabled: true,
            networking_enabled: true,
            audio_enabled: true,
            render_debug_info: false,
        }
    }
}