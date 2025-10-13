/*!
# Game State Management

Match lifecycle, game rules, and state coordination for multiplayer sessions.
*/

use bevy::prelude::*;
use serde::{Deserialize, Serialize};

/// Core game state plugin
pub struct GameStatePlugin;

impl Plugin for GameStatePlugin {
    fn build(&self, app: &mut App) {
        app
            .add_systems(Update, (
                update_match_timer,
                check_win_conditions,
                manage_player_respawns,
            ))
            .register_type::<MatchState>()
            .register_type::<GameRules>();
    }
}

/// Match state tracking
#[derive(Component, Debug, Clone, Reflect, Serialize, Deserialize)]
#[reflect(Component)]
pub struct MatchState {
    pub match_id: u32,
    pub status: MatchStatus,
    pub max_players: usize,
    pub connected_players: Vec<u32>,
    pub start_time: f64,
    pub time_limit: f32, // Match duration in seconds
    pub winner: Option<u32>,
}

impl MatchState {
    pub fn new(match_id: u32, max_players: usize) -> Self {
        Self {
            match_id,
            status: MatchStatus::Waiting,
            max_players,
            connected_players: Vec::new(),
            start_time: 0.0,
            time_limit: 300.0, // 5 minutes default
            winner: None,
        }
    }

    pub fn add_player(&mut self, player_id: u32) -> bool {
        if self.connected_players.len() < self.max_players 
           && !self.connected_players.contains(&player_id) {
            self.connected_players.push(player_id);
            
            // Auto-start if minimum players reached
            if self.connected_players.len() >= 2 && matches!(self.status, MatchStatus::Waiting) {
                self.status = MatchStatus::Starting { countdown: 5.0 };
            }
            
            true
        } else {
            false
        }
    }

    pub fn remove_player(&mut self, player_id: u32) {
        self.connected_players.retain(|&id| id != player_id);
        
        // End match if too few players
        if self.connected_players.len() < 2 && matches!(self.status, MatchStatus::InProgress) {
            self.status = MatchStatus::Finished { winner: None };
        }
    }

    pub fn is_full(&self) -> bool {
        self.connected_players.len() >= self.max_players
    }

    pub fn player_count(&self) -> usize {
        self.connected_players.len()
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize, Reflect)]
pub enum MatchStatus {
    Waiting,
    Starting { countdown: f32 },
    InProgress,
    Finished { winner: Option<u32> },
}

/// Game rules configuration
#[derive(Component, Debug, Clone, Reflect)]
#[reflect(Component)]  
pub struct GameRules {
    pub win_condition: WinCondition,
    pub respawn_enabled: bool,
    pub respawn_time: f32,
    pub friendly_fire: bool,
    pub arena_bounds: ArenaBounds,
}

impl Default for GameRules {
    fn default() -> Self {
        Self {
            win_condition: WinCondition::LastPlayerStanding,
            respawn_enabled: true,
            respawn_time: 5.0,
            friendly_fire: false,
            arena_bounds: ArenaBounds::default(),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Reflect)]
pub enum WinCondition {
    LastPlayerStanding,
    FirstToKills(u32),
    TimeLimit,
    KingOfTheHill,
}

/// Arena boundaries and respawn points
#[derive(Debug, Clone, Reflect)]
pub struct ArenaBounds {
    pub min: Vec3,
    pub max: Vec3,
    pub death_plane_y: f32,
    pub respawn_points: Vec<Vec3>,
}

impl Default for ArenaBounds {
    fn default() -> Self {
        Self {
            min: Vec3::new(-20.0, -5.0, -20.0),
            max: Vec3::new(20.0, 15.0, 20.0),
            death_plane_y: -10.0,
            respawn_points: vec![
                Vec3::new(-5.0, 2.0, -5.0),
                Vec3::new(5.0, 2.0, -5.0),
                Vec3::new(-5.0, 2.0, 5.0),
                Vec3::new(5.0, 2.0, 5.0),
            ],
        }
    }
}

/// Player score tracking
#[derive(Component, Debug, Clone, Default, Reflect)]
#[reflect(Component)]
pub struct PlayerScore {
    pub kills: u32,
    pub deaths: u32,
    pub damage_dealt: f32,
    pub damage_taken: f32,
    pub survival_time: f32,
}

impl PlayerScore {
    pub fn kill_death_ratio(&self) -> f32 {
        if self.deaths == 0 {
            self.kills as f32
        } else {
            self.kills as f32 / self.deaths as f32
        }
    }
}

/// System to update match timers
fn update_match_timer(
    mut match_query: Query<&mut MatchState>,
    time: Res<Time>,
) {
    for mut match_state in match_query.iter_mut() {
        match &mut match_state.status {
            MatchStatus::Starting { countdown } => {
                *countdown -= time.delta_seconds();
                if *countdown <= 0.0 {
                    match_state.status = MatchStatus::InProgress;
                    match_state.start_time = time.elapsed_seconds_f64();
                    info!("Match {} started!", match_state.match_id);
                }
            },
            MatchStatus::InProgress => {
                let elapsed = time.elapsed_seconds_f64() - match_state.start_time;
                if elapsed >= match_state.time_limit as f64 {
                    match_state.status = MatchStatus::Finished { winner: None };
                    info!("Match {} ended due to time limit", match_state.match_id);
                }
            },
            _ => {}
        }
    }
}

/// System to check win conditions
fn check_win_conditions(
    match_query: Query<&MatchState>,
    player_query: Query<(&crate::entities::Player, &crate::entities::Health, &PlayerScore)>,
    mut commands: Commands,
) {
    for match_state in match_query.iter() {
        if !matches!(match_state.status, MatchStatus::InProgress) {
            continue;
        }

        let alive_players: Vec<_> = player_query
            .iter()
            .filter(|(player, health, _)| {
                match_state.connected_players.contains(&player.id) && health.is_alive
            })
            .collect();

        // Check for last player standing
        if alive_players.len() <= 1 {
            let winner_id = alive_players.first().map(|(player, _, _)| player.id);
            
            // Update match state (would need mutable query in real implementation)
            info!("Match {} ended - Winner: {:?}", match_state.match_id, winner_id);
        }
    }
}

/// System to manage player respawns
fn manage_player_respawns(
    mut player_query: Query<(Entity, &crate::entities::Player, &mut crate::entities::Health, &mut Transform)>,
    game_rules_query: Query<&GameRules>,
    match_query: Query<&MatchState>,
    time: Res<Time>,
) {
    let game_rules = game_rules_query.get_single();
    let match_state = match_query.get_single();

    if let (Ok(rules), Ok(match_state)) = (game_rules, match_state) {
        if !matches!(match_state.status, MatchStatus::InProgress) || !rules.respawn_enabled {
            return;
        }

        for (entity, player, mut health, mut transform) in player_query.iter_mut() {
            // Check if player fell out of bounds
            if transform.translation.y < rules.arena_bounds.death_plane_y && health.is_alive {
                let damage_amount = health.current;
                health.take_damage(damage_amount, time.elapsed_seconds_f64());
                info!("Player {} fell out of bounds", player.id);
            }

            // Respawn dead players
            if !health.is_alive && rules.respawn_enabled {
                // TODO: Add respawn timer logic
                let respawn_point = rules.arena_bounds.respawn_points
                    [player.id as usize % rules.arena_bounds.respawn_points.len()];
                
                transform.translation = respawn_point;
                health.current = health.maximum;
                health.is_alive = true;
                
                info!("Player {} respawned at {:?}", player.id, respawn_point);
            }
        }
    }
}