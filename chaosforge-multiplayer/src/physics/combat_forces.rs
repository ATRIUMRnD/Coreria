use bevy::prelude::*;

/// Combat force application system
pub struct CombatForce {
    pub direction: Vec3,
    pub magnitude: f32,
    pub duration: f32,
    pub force_type: ForceType,
}

#[derive(Debug, Clone, Copy)]
pub enum ForceType {
    Impulse,     // Instant force (punches, kicks)
    Continuous,  // Sustained force (grappling)
    Explosion,   // Radial force (special abilities)
}

impl CombatForce {
    /// Create a punch force
    pub fn punch(direction: Vec3, style: crate::entities::FightingStyle) -> Self {
        let base_force = match style {
            crate::entities::FightingStyle::Brawler => 800.0,
            crate::entities::FightingStyle::Striker => 600.0,
            crate::entities::FightingStyle::Phantom => 400.0,
            crate::entities::FightingStyle::Titan => 1200.0,
        };

        Self {
            direction: direction.normalize(),
            magnitude: base_force,
            duration: 0.1, // 100ms impact
            force_type: ForceType::Impulse,
        }
    }

    /// Create a kick force (stronger than punch)
    pub fn kick(direction: Vec3, style: crate::entities::FightingStyle) -> Self {
        let mut punch_force = Self::punch(direction, style);
        punch_force.magnitude *= 1.5; // Kicks are 50% stronger
        punch_force.duration = 0.15;  // Slightly longer impact
        punch_force
    }

    /// Create a grab force (sustained)
    pub fn grab(direction: Vec3) -> Self {
        Self {
            direction: direction.normalize(),
            magnitude: 300.0,
            duration: 1.0, // 1 second hold
            force_type: ForceType::Continuous,
        }
    }

    /// Create special ability forces
    pub fn special_ability(style: crate::entities::FightingStyle, target_pos: Vec3, origin: Vec3) -> Self {
        let direction = (target_pos - origin).normalize();
        
        match style {
            crate::entities::FightingStyle::Brawler => {
                // Berserker Rage: Area knockback
                Self {
                    direction: Vec3::Y + direction * 0.5,
                    magnitude: 1500.0,
                    duration: 0.5,
                    force_type: ForceType::Explosion,
                }
            },
            crate::entities::FightingStyle::Striker => {
                // Tornado Kick: Spinning force
                Self {
                    direction: Vec3::new(direction.x, 0.3, direction.z).normalize(),
                    magnitude: 1000.0,
                    duration: 0.8, // Extended spinning
                    force_type: ForceType::Impulse,
                }
            },
            crate::entities::FightingStyle::Phantom => {
                // Phase Step: Minimal force (mostly movement)
                Self {
                    direction,
                    magnitude: 200.0,
                    duration: 0.2,
                    force_type: ForceType::Impulse,
                }
            },
            crate::entities::FightingStyle::Titan => {
                // Ground Slam: Massive area effect
                Self {
                    direction: Vec3::new(0.0, -1.0, 0.0),
                    magnitude: 2000.0,
                    duration: 1.0,
                    force_type: ForceType::Explosion,
                }
            },
        }
    }
}

/// Apply combat forces to physics bodies
pub fn apply_combat_forces(
    // This system would integrate with Rapier physics when enabled
    #[cfg(feature = "multiplayer-physics")]
    mut physics_query: Query<&mut bevy_rapier3d::dynamics::RigidBody>,
    
    combat_query: Query<(&crate::entities::CombatState, &crate::entities::Player, &Transform)>,
) {
    #[cfg(feature = "multiplayer-physics")]
    {
        for (combat_state, player, transform) in combat_query.iter() {
            // Apply forces based on active combat actions
            // This would use actual Rapier RigidBody components
            
            if let Some(combo) = &combat_state.current_combo {
                match combo.combo_type {
                    crate::entities::ComboType::BasicAttack => {
                        let force = CombatForce::punch(transform.forward(), player.style);
                        // Apply force to physics body
                    },
                    crate::entities::ComboType::PowerStrike => {
                        let force = CombatForce::kick(transform.forward(), player.style);
                        // Apply stronger force
                    },
                    crate::entities::ComboType::SpecialMove => {
                        let force = CombatForce::special_ability(
                            player.style, 
                            transform.translation + transform.forward() * 2.0,
                            transform.translation
                        );
                        // Apply special ability force
                    },
                    _ => {}
                }
            }
        }
    }
}

/// Calculate knockback force between two entities
pub fn calculate_knockback(
    attacker_pos: Vec3,
    victim_pos: Vec3, 
    force_magnitude: f32,
    combat_type: crate::entities::ComboType,
) -> Vec3 {
    let direction = (victim_pos - attacker_pos).normalize_or_zero();
    let knockback_mult = match combat_type {
        crate::entities::ComboType::BasicAttack => 1.0,
        crate::entities::ComboType::PowerStrike => 2.0,
        crate::entities::ComboType::Grapple => 0.5,
        crate::entities::ComboType::SpecialMove => 3.0,
    };
    
    direction * force_magnitude * knockback_mult
}