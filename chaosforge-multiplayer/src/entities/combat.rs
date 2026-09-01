use bevy::prelude::*;
use serde::{Deserialize, Serialize};

/// Combat state tracking for players
#[derive(Component, Debug, Clone, Reflect, Serialize, Deserialize)]
#[reflect(Component)]
pub struct CombatState {
    pub current_combo: Option<ActiveCombo>,
    pub combo_meter: f32,
    pub last_hit_time: f64,
    pub hit_streak: u32,
    pub damage_multiplier: f32,
    pub is_blocking: bool,
    pub block_stamina: f32,
}

impl Default for CombatState {
    fn default() -> Self {
        Self {
            current_combo: None,
            combo_meter: 0.0,
            last_hit_time: 0.0,
            hit_streak: 0,
            damage_multiplier: 1.0,
            is_blocking: false,
            block_stamina: 100.0,
        }
    }
}

/// Active combo sequence being performed
#[derive(Debug, Clone, Serialize, Deserialize, Reflect)]
pub struct ActiveCombo {
    pub combo_type: ComboType,
    pub sequence: Vec<ComboInput>,
    pub current_step: usize,
    pub timing_window: f32,
    pub damage_bonus: f32,
}

/// Types of combos available
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Reflect)]
pub enum ComboType {
    BasicAttack,
    PowerStrike,
    Grapple,
    SpecialMove,
}

/// Individual input in a combo sequence
#[derive(Debug, Clone, Copy, Serialize, Deserialize, Reflect)]
pub struct ComboInput {
    pub action: CombatAction,
    pub timing_frame: u32,
    pub required_precision: f32, // 0.0 = loose timing, 1.0 = frame-perfect
}

/// Combat actions that can be performed
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Reflect)]
pub enum CombatAction {
    Punch,
    Kick,
    Block,
    Grab,
    Jump,
    Dash,
    Special1,
    Special2,
}

/// Combat damage event
#[derive(Event, Debug, Clone)]
pub struct DamageEvent {
    pub attacker: Entity,
    pub victim: Entity,
    pub damage: f32,
    pub damage_type: DamageType,
    pub knockback: Vec3,
    pub combo_used: Option<ComboType>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DamageType {
    Physical,
    Special,
    Environmental,
}

/// System to process combat actions and resolve damage
pub fn process_combat_actions(
    mut combat_query: Query<(Entity, &mut CombatState, &crate::entities::Player, &Transform)>,
    _damage_events: EventWriter<DamageEvent>,
    time: Res<Time>,
) {
    let current_time = time.elapsed_seconds_f64();
    
    for (_entity, mut combat_state, _player, _transform) in combat_query.iter_mut() {
        // Update combo timing
        if let Some(ref mut combo) = combat_state.current_combo {
            combo.timing_window -= time.delta_seconds();
            
            // Combo expired
            if combo.timing_window <= 0.0 {
                combat_state.current_combo = None;
                combat_state.combo_meter *= 0.5; // Decay on failed combo
            }
        }

        // Update block stamina
        if combat_state.is_blocking {
            combat_state.block_stamina -= 20.0 * time.delta_seconds(); // Blocking drains stamina
            if combat_state.block_stamina <= 0.0 {
                combat_state.is_blocking = false; // Force break
            }
        } else if combat_state.block_stamina < 100.0 {
            combat_state.block_stamina += 15.0 * time.delta_seconds(); // Regen when not blocking
            combat_state.block_stamina = combat_state.block_stamina.min(100.0);
        }

        // Decay hit streak over time
        if current_time - combat_state.last_hit_time > 3.0 {
            combat_state.hit_streak = 0;
            combat_state.damage_multiplier = 1.0;
        }
    }
}

/// Apply damage from one entity to another
pub fn apply_damage(
    attacker_entity: Entity,
    victim_entity: Entity,
    base_damage: f32,
    damage_type: DamageType,
    knockback: Vec3,
    combo: Option<ComboType>,
    attacker_query: &Query<&crate::entities::Player>,
    victim_query: &mut Query<(&mut crate::entities::Health, &mut CombatState)>,
    damage_events: &mut EventWriter<DamageEvent>,
) -> bool {
    // Get attacker style for damage calculation
    let attacker_damage_mult = if let Ok(attacker) = attacker_query.get(attacker_entity) {
        attacker.style.damage_multiplier()
    } else {
        1.0
    };

    // Apply damage to victim
    if let Ok((mut health, mut combat_state)) = victim_query.get_mut(victim_entity) {
        let mut final_damage = base_damage * attacker_damage_mult;
        
        // Check if victim is blocking
        if combat_state.is_blocking && damage_type == DamageType::Physical {
            final_damage *= 0.3; // 70% damage reduction when blocking
            combat_state.block_stamina -= 15.0; // Blocking costs stamina
        }

        // Apply combo damage bonus
        if let Some(_combo_type) = combo {
            final_damage *= 1.5; // 50% bonus for combo attacks
        }

        // Apply damage
        health.take_damage(final_damage, 0.0); // TODO: Use proper time

        // Send damage event
        damage_events.send(DamageEvent {
            attacker: attacker_entity,
            victim: victim_entity,
            damage: final_damage,
            damage_type,
            knockback,
            combo_used: combo,
        });

        true
    } else {
        false
    }
}

/// Validate combo input sequence
pub fn validate_combo_input(
    current_combo: &mut ActiveCombo,
    input_action: CombatAction,
    input_time: f32,
) -> bool {
    if current_combo.current_step >= current_combo.sequence.len() {
        return false; // Combo already complete
    }

    let expected_input = &current_combo.sequence[current_combo.current_step];
    
    // Check if the action matches
    if expected_input.action != input_action {
        return false;
    }

    // Check timing window (3-frame buffer from research.md)
    let timing_tolerance = 3.0 / 60.0; // 3 frames at 60 FPS
    let timing_error = (input_time - (expected_input.timing_frame as f32 / 60.0)).abs();
    
    if timing_error <= timing_tolerance {
        current_combo.current_step += 1;
        current_combo.timing_window = timing_tolerance; // Reset window for next input
        true
    } else {
        false
    }
}