/*!
# Engine Events

Custom events for engine lifecycle and inter-system communication.
*/

use bevy::prelude::*;

/// Engine lifecycle events
#[derive(Event, Debug, Clone)]
pub enum EngineEvent {
    Initialize,
    Pause,
    Resume,
    Shutdown,
}

/// Performance warning events
#[derive(Event, Debug, Clone)]
pub struct PerformanceWarning {
    pub system_name: String,
    pub frame_time_ms: f64,
    pub threshold_ms: f64,
}

/// Graphics-related events
#[derive(Event, Debug, Clone)]
pub enum GraphicsEvent {
    WindowResize { width: u32, height: u32 },
    FullscreenToggle,
    VSyncToggle,
}

/// Physics events
#[derive(Event, Debug, Clone)]
pub enum PhysicsEvent {
    Collision { entity_a: Entity, entity_b: Entity },
    ForceApplied { entity: Entity, force: Vec3 },
}