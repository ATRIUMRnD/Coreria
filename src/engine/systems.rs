/*!
# Engine Systems

Core Bevy systems that handle engine lifecycle, performance monitoring,
and coordination between subsystems.
*/

use bevy::prelude::*;
use log::{info, debug};
use std::time::Instant;

use super::{EngineConfig, PerformanceMetrics, EngineState};

/// System that runs once at engine startup
pub fn engine_startup(
    mut commands: Commands,
    config: Res<EngineConfig>,
) {
    info!("🎬 Engine startup system executing...");
    
    // Spawn main engine entity
    commands.spawn((
        Name::new("CoreiaEngine"),
        Transform::default(),
        GlobalTransform::default(),
    ));
    
    info!("📊 Target FPS: {}", config.target_fps);
    info!("⏱️  Physics timestep: {:?}", config.physics_timestep);
    info!("🖥️  VSync: {}", if config.vsync_enabled { "enabled" } else { "disabled" });
}

/// System to update performance metrics
pub fn update_performance_metrics(
    time: Res<Time>,
    mut metrics: ResMut<PerformanceMetrics>,
) {
    let frame_time = time.delta();
    metrics.update(frame_time);
    
    // Log performance every 5 seconds
    if metrics.frame_count % 300 == 0 {  // Assuming 60 FPS
        debug!("📊 Performance - FPS: {:.1}, Frame time: {:.2}ms", 
              metrics.last_fps, 
              metrics.avg_frame_time.as_secs_f64() * 1000.0);
    }
}

/// Handle engine-level events and state changes
pub fn handle_engine_events(
    keyboard_input: Res<Input<KeyCode>>,
    mut exit: EventWriter<AppExit>,
) {
    // Handle global shortcuts
    if keyboard_input.pressed(KeyCode::AltLeft) && keyboard_input.just_pressed(KeyCode::F4) {
        info!("🚪 Alt+F4 pressed - shutting down engine");
        exit.send(AppExit);
    }
    
    if keyboard_input.pressed(KeyCode::ControlLeft) && keyboard_input.just_pressed(KeyCode::C) {
        info!("⏹️  Ctrl+C pressed - shutting down engine");
        exit.send(AppExit);
    }
}

/// Cleanup system that runs in PostUpdate
pub fn cleanup_systems() {
    // Placeholder for cleanup logic
    // This would handle things like:
    // - Memory cleanup
    // - Resource deallocation  
    // - Temporary entity removal
}