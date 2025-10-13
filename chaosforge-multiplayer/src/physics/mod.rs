/*!
# Physics Engine Integration

Rapier3D-based physics simulation for ragdoll characters and combat mechanics.
Conservative joint constraints for stability as defined in research.md.
*/

pub mod rapier_integration;
pub mod joint_constraints;
pub mod combat_forces;

use bevy::prelude::*;

pub use rapier_integration::*;
pub use joint_constraints::*; 
pub use combat_forces::*;

/// Physics plugin for ragdoll simulation
pub struct PhysicsPlugin;

impl Plugin for PhysicsPlugin {
    fn build(&self, app: &mut App) {
        #[cfg(feature = "multiplayer-physics")]
        {
            app
                .add_plugins(bevy_rapier3d::RapierPhysicsPlugin::<()>::default())
                .add_plugins(bevy_rapier3d::RapierDebugRenderPlugin::default())
                .add_systems(Update, (
                    apply_combat_forces,
                    monitor_physics_performance,
                    update_joint_constraints,
                ));
        }
    }
}