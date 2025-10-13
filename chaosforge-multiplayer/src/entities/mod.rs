/*!
# Entity System

ECS components for players, ragdolls, combat, and game state.

Following the data model specification with Bevy ECS components
for multiplayer ragdoll combat mechanics.
*/

pub mod player;
pub mod ragdoll;
pub mod combat;
pub mod input;
pub mod network;

// Re-export all entity types
pub use player::*;
pub use ragdoll::*;
pub use combat::*;
pub use input::*;
pub use network::*;

use bevy::prelude::*;

/// Entity plugin registering all component systems
pub struct PlayerPlugin;

impl Plugin for PlayerPlugin {
    fn build(&self, app: &mut App) {
        app
            .add_event::<combat::DamageEvent>()
            .add_event::<network::PlayerStateSync>()
            .add_systems(Update, (
                player::update_player_physics,
                ragdoll::update_ragdoll_bodies,
                combat::process_combat_actions,
                input::process_input_buffer,
                network::sync_network_state,
            ))
            .register_type::<Player>()
            .register_type::<RagdollBody>()
            .register_type::<Health>()
            .register_type::<CombatState>()
            .register_type::<NetworkState>();
    }
}