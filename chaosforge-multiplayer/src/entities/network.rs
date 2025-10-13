use bevy::prelude::*;
use serde::{Deserialize, Serialize};

/// Network state tracking for multiplayer synchronization
#[derive(Component, Debug, Clone, Reflect, Serialize, Deserialize)]
#[reflect(Component)]
pub struct NetworkState {
    pub player_id: u32,
    pub is_local: bool,
    pub last_sync_time: f64,
    pub prediction_error: f32,
    pub packet_loss: f32,
    pub rtt: f32, // Round-trip time in milliseconds
}

impl Default for NetworkState {
    fn default() -> Self {
        Self {
            player_id: 0,
            is_local: true,
            last_sync_time: 0.0,
            prediction_error: 0.0,
            packet_loss: 0.0,
            rtt: 0.0,
        }
    }
}

/// Network synchronization event for player state updates
#[derive(Event, Debug, Clone, Serialize, Deserialize)]
pub struct PlayerStateSync {
    pub player_id: u32,
    pub position: Vec3,
    pub rotation: Quat,
    pub velocity: Vec3,
    pub health: f32,
    pub frame_number: u64,
    pub timestamp: f64,
}

/// System to synchronize network state for multiplayer
pub fn sync_network_state(
    mut network_query: Query<(Entity, &mut NetworkState, &Transform, &crate::entities::Health)>,
    mut sync_events: EventWriter<PlayerStateSync>,
    time: Res<Time>,
) {
    let current_time = time.elapsed_seconds_f64();
    
    for (entity, mut network_state, transform, health) in network_query.iter_mut() {
        // Update sync timing
        network_state.last_sync_time = current_time;
        
        // For local players, send state to network
        if network_state.is_local {
            sync_events.send(PlayerStateSync {
                player_id: network_state.player_id,
                position: transform.translation,
                rotation: transform.rotation,
                velocity: Vec3::ZERO, // TODO: Get from physics component
                health: health.current,
                frame_number: 0, // TODO: Get from game state
                timestamp: current_time,
            });
        }
    }
}

/// Network message types for client-server communication
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum NetworkMessage {
    // Client -> Server
    JoinMatch {
        player_name: String,
        fighting_style: crate::entities::FightingStyle,
    },
    PlayerInput {
        player_id: u32,
        frame: u64,
        actions: Vec<crate::entities::InputAction>,
        timestamp: f64,
    },
    LeaveMatch {
        player_id: u32,
    },
    
    // Server -> Client  
    MatchJoined {
        player_id: u32,
        match_id: u32,
    },
    GameState {
        frame: u64,
        players: Vec<PlayerStateSync>,
        timestamp: f64,
    },
    PlayerDamage {
        attacker_id: u32,
        victim_id: u32,
        damage: f32,
        combo_type: Option<crate::entities::ComboType>,
    },
    MatchEnd {
        winner_id: Option<u32>,
        match_id: u32,
    },
}

/// Network configuration for server instances
#[derive(Resource, Debug, Clone)]
pub struct ServerConfig {
    pub port: u16,
}

/// Network configuration for client instances  
#[derive(Resource, Debug, Clone)]
pub struct ClientConfig {
    pub server_addr: String,
}

/// Placeholder network plugin (would integrate with bevy_renet)
pub struct NetworkPlugin;

impl Plugin for NetworkPlugin {
    fn build(&self, app: &mut App) {
        app
            .add_event::<PlayerStateSync>()
            .add_systems(Update, (
                sync_network_state,
                handle_network_messages,
            ));
    }
}

/// System to handle incoming network messages
fn handle_network_messages(
    // This would integrate with bevy_renet for actual networking
    mut sync_events: EventReader<PlayerStateSync>,
    mut network_query: Query<(&mut NetworkState, &mut Transform)>,
) {
    for sync_event in sync_events.read() {
        // Find the corresponding network entity and update it
        for (mut network_state, mut transform) in network_query.iter_mut() {
            if network_state.player_id == sync_event.player_id && !network_state.is_local {
                // Update remote player position from network
                transform.translation = sync_event.position;
                transform.rotation = sync_event.rotation;
                network_state.last_sync_time = sync_event.timestamp;
            }
        }
    }
}