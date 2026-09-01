/*!
# Physics Subsystem

Physics simulation using a placeholder for ODE integration.
In the full implementation, this would interface with the Open Dynamics Engine.
*/

use bevy::prelude::*;
use log::{info, debug};
use std::time::Duration;

pub mod ode_integration;
pub mod collision;
pub mod forces;

/// Physics plugin for the engine
pub struct PhysicsPlugin;

impl Plugin for PhysicsPlugin {
    fn build(&self, app: &mut App) {
        info!("⚡ Initializing Physics Plugin");
        
        app
            // Add physics resources
            .insert_resource(PhysicsSettings::default())
            .insert_resource(PhysicsWorld::default())
            
            // Add physics systems
            .add_systems(Startup, physics_startup)
            .add_systems(FixedUpdate, (
                integrate_physics,
                update_physics_world,
                apply_forces,
            ).chain())
            .add_systems(Update, handle_physics_debug);
    }
}

/// Physics simulation settings
#[derive(Resource, Debug, Clone)]
pub struct PhysicsSettings {
    pub gravity: Vec3,
    pub timestep: Duration,
    pub max_substeps: u32,
    pub damping: f32,
    pub restitution: f32,
}

impl Default for PhysicsSettings {
    fn default() -> Self {
        Self {
            gravity: Vec3::new(0.0, -9.81, 0.0), // Earth gravity
            timestep: Duration::from_millis(16), // ~60 FPS
            max_substeps: 4,
            damping: 0.1,
            restitution: 0.5,
        }
    }
}

/// Physics world state
#[derive(Resource, Debug, Default)]
pub struct PhysicsWorld {
    pub active_bodies: u32,
    pub collision_pairs: u32,
    pub physics_time: f64,
    pub simulation_running: bool,
}

/// Marker component for physics-enabled entities
#[derive(Component, Debug, Default)]
pub struct RigidBody {
    pub mass: f32,
    pub velocity: Vec3,
    pub angular_velocity: Vec3,
    pub is_kinematic: bool,
}

/// Physics forces component
#[derive(Component, Debug, Default)]
pub struct Forces {
    pub linear: Vec3,
    pub torque: Vec3,
}

/// Collision shape component
#[derive(Component, Debug)]
pub enum CollisionShape {
    Sphere { radius: f32 },
    Box { size: Vec3 },
    Capsule { height: f32, radius: f32 },
    Mesh { vertices: Vec<Vec3> },
}

impl Default for CollisionShape {
    fn default() -> Self {
        Self::Box { size: Vec3::ONE }
    }
}

/// Physics startup system
fn physics_startup(
    mut world: ResMut<PhysicsWorld>,
    settings: Res<PhysicsSettings>,
) {
    info!("🎬 Physics system startup");
    debug!("🌍 Gravity: {:?}", settings.gravity);
    debug!("⏱️  Timestep: {:?}", settings.timestep);
    
    world.simulation_running = true;
    
    // In a full implementation, this would:
    // - Initialize ODE world
    // - Set up collision detection
    // - Configure physics parameters
    info!("⚠️  Using placeholder physics - ODE integration not implemented");
}

/// Main physics integration step
fn integrate_physics(
    time: Res<Time>,
    settings: Res<PhysicsSettings>,
    mut world: ResMut<PhysicsWorld>,
    mut query: Query<(&mut Transform, &mut RigidBody, Option<&Forces>)>,
) {
    if !world.simulation_running {
        return;
    }
    
    let dt = time.delta().as_secs_f32();
    world.physics_time += dt as f64;
    
    // Simple Euler integration (placeholder for ODE)
    for (mut transform, mut body, forces) in query.iter_mut() {
        if body.is_kinematic {
            continue;
        }
        
        // Apply gravity
        let mut total_force = settings.gravity * body.mass;
        
        // Apply external forces
        if let Some(forces) = forces {
            total_force += forces.linear;
        }
        
        // Update velocity (F = ma)
        if body.mass > 0.0 {
            body.velocity += total_force / body.mass * dt;
        }
        
        // Apply damping
        body.velocity *= 1.0 - settings.damping * dt;
        
        // Update position
        transform.translation += body.velocity * dt;
    }
    
    world.active_bodies = query.iter().count() as u32;
}

/// Update physics world state
fn update_physics_world(
    mut world: ResMut<PhysicsWorld>,
    rigid_bodies: Query<&RigidBody>,
) {
    // Count active bodies
    world.active_bodies = rigid_bodies.iter().count() as u32;
    
    // In full implementation, would sync with ODE world
    // and handle collision detection results
}

/// Apply forces to physics bodies
fn apply_forces(
    mut query: Query<&mut Forces>,
) {
    // Reset forces after integration
    for mut forces in query.iter_mut() {
        forces.linear = Vec3::ZERO;
        forces.torque = Vec3::ZERO;
    }
}

/// Handle physics debug visualization
fn handle_physics_debug(
    keyboard_input: Res<Input<KeyCode>>,
    mut world: ResMut<PhysicsWorld>,
    settings: Res<PhysicsSettings>,
) {
    // Toggle physics simulation with F3
    if keyboard_input.just_pressed(KeyCode::F3) {
        world.simulation_running = !world.simulation_running;
        info!("⚡ Physics simulation: {}", 
              if world.simulation_running { "enabled" } else { "paused" });
    }
    
    // Debug info with F4
    if keyboard_input.just_pressed(KeyCode::F4) {
        info!("🔬 Physics Debug Info:");
        info!("   Active bodies: {}", world.active_bodies);
        info!("   Simulation time: {:.2}s", world.physics_time);
        info!("   Gravity: {:?}", settings.gravity);
        info!("   Timestep: {:?}", settings.timestep);
    }
}