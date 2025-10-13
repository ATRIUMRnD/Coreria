use bevy::prelude::*;
use serde::{Deserialize, Serialize};

/// Core player entity component
#[derive(Component, Debug, Clone, Reflect, Serialize, Deserialize)]
#[reflect(Component)]
pub struct Player {
    pub id: u32,
    pub name: String,
    pub style: FightingStyle,
    pub is_local: bool,
    pub spawn_time: f64,
}

impl Player {
    pub fn new(id: u32, style: FightingStyle) -> Self {
        Self {
            id,
            name: format!("Player_{}", id),
            style,
            is_local: false,
            spawn_time: 0.0, // Will be set by spawn system
        }
    }
}

/// The four distinct fighting styles from the specification
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize, Reflect)]
pub enum FightingStyle {
    /// Tank-style fighter: High mass, powerful but slow attacks
    /// +20% mass, +50% joint strength, slower movement
    Brawler,
    
    /// Speed-based fighter: Fast strikes and mobility
    /// -10% mass, +25% joint flexibility, quick attacks  
    Striker,
    
    /// Stealth fighter: Reduced collision response, phase abilities
    /// -20% mass, reduced collision response, special mobility
    Phantom,
    
    /// Heavy fighter: Maximum mass and strength
    /// +50% mass, +100% joint strength, devastating attacks
    Titan,
}

impl FightingStyle {
    /// Get mass multiplier for physics calculations
    pub fn mass_multiplier(self) -> f32 {
        match self {
            FightingStyle::Brawler => 1.2,  // +20% mass
            FightingStyle::Striker => 0.9,  // -10% mass
            FightingStyle::Phantom => 0.8,  // -20% mass
            FightingStyle::Titan => 1.5,    // +50% mass
        }
    }

    /// Get speed multiplier for movement
    pub fn speed_multiplier(self) -> f32 {
        match self {
            FightingStyle::Brawler => 0.8,  // 20% slower
            FightingStyle::Striker => 1.3,  // 30% faster
            FightingStyle::Phantom => 1.1,  // 10% faster
            FightingStyle::Titan => 0.6,    // 40% slower
        }
    }

    /// Get joint strength multiplier
    pub fn joint_strength_multiplier(self) -> f32 {
        match self {
            FightingStyle::Brawler => 1.5,  // +50% joint strength
            FightingStyle::Striker => 1.25, // +25% flexibility
            FightingStyle::Phantom => 1.0,  // Normal strength
            FightingStyle::Titan => 2.0,    // +100% strength
        }
    }

    /// Get attack damage multiplier
    pub fn damage_multiplier(self) -> f32 {
        match self {
            FightingStyle::Brawler => 1.3,  // Heavy hits
            FightingStyle::Striker => 1.0,  // Normal damage
            FightingStyle::Phantom => 0.8,  // Light but fast
            FightingStyle::Titan => 1.8,    // Devastating
        }
    }

    /// Get special ability description
    pub fn special_ability(self) -> &'static str {
        match self {
            FightingStyle::Brawler => "Berserker Rage: Temporary invulnerability",
            FightingStyle::Striker => "Tornado Kick: Multi-hit spinning attack", 
            FightingStyle::Phantom => "Phase Step: Brief intangibility",
            FightingStyle::Titan => "Ground Slam: Area knockback attack",
        }
    }

    /// Get style color for visual representation
    pub fn color(self) -> Color {
        match self {
            FightingStyle::Brawler => Color::srgb(1.0, 0.0, 0.0), // Red
            FightingStyle::Striker => Color::srgb(0.0, 0.0, 1.0), // Blue
            FightingStyle::Phantom => Color::srgb(0.5, 0.0, 0.5), // Purple
            FightingStyle::Titan => Color::srgb(1.0, 0.5, 0.0), // Orange
        }
    }

    /// All available styles for iteration
    pub fn all_styles() -> [FightingStyle; 4] {
        [
            FightingStyle::Brawler,
            FightingStyle::Striker, 
            FightingStyle::Phantom,
            FightingStyle::Titan,
        ]
    }
}

impl Default for FightingStyle {
    fn default() -> Self {
        FightingStyle::Brawler
    }
}

/// Style-specific properties component
#[derive(Component, Debug, Clone, Reflect)]
#[reflect(Component)]
pub struct StyleProperties {
    pub mass_multiplier: f32,
    pub speed_multiplier: f32,
    pub strength_multiplier: f32,
    pub damage_multiplier: f32,
    pub special_cooldown: f32,
    pub special_ready: bool,
}

impl StyleProperties {
    pub fn from_style(style: FightingStyle) -> Self {
        Self {
            mass_multiplier: style.mass_multiplier(),
            speed_multiplier: style.speed_multiplier(),
            strength_multiplier: style.joint_strength_multiplier(),
            damage_multiplier: style.damage_multiplier(),
            special_cooldown: 0.0,
            special_ready: true,
        }
    }
}

/// Health and damage tracking
#[derive(Component, Debug, Clone, Reflect, Serialize, Deserialize)]
#[reflect(Component)]
pub struct Health {
    pub current: f32,
    pub maximum: f32,
    pub regeneration_rate: f32,
    pub last_damage_time: f64,
    pub is_alive: bool,
}

impl Health {
    pub fn new(max_health: f32) -> Self {
        Self {
            current: max_health,
            maximum: max_health,
            regeneration_rate: 1.0, // 1 HP per second
            last_damage_time: 0.0,
            is_alive: true,
        }
    }

    pub fn take_damage(&mut self, amount: f32, time: f64) {
        self.current = (self.current - amount).max(0.0);
        self.last_damage_time = time;
        self.is_alive = self.current > 0.0;
    }

    pub fn heal(&mut self, amount: f32) {
        self.current = (self.current + amount).min(self.maximum);
    }

    pub fn is_full_health(&self) -> bool {
        self.current >= self.maximum
    }

    pub fn health_percentage(&self) -> f32 {
        self.current / self.maximum
    }
}

/// Player physics update system
pub fn update_player_physics(
    mut query: Query<(&Player, &mut StyleProperties, &mut Health)>,
    time: Res<Time>,
) {
    let current_time = time.elapsed_seconds_f64();
    
    for (_player, mut style_props, mut health) in query.iter_mut() {
        // Update special ability cooldown
        if !style_props.special_ready {
            style_props.special_cooldown -= time.delta_seconds();
            if style_props.special_cooldown <= 0.0 {
                style_props.special_ready = true;
                style_props.special_cooldown = 0.0;
            }
        }

        // Natural health regeneration (after 3 seconds without damage)
        if health.is_alive && current_time - health.last_damage_time > 3.0 {
            let regen_amount = health.regeneration_rate * time.delta_seconds();
            health.heal(regen_amount);
        }
    }
}

/// Spawn a new player with the specified fighting style
pub fn spawn_player(
    commands: &mut Commands,
    player_id: u32,
    style: FightingStyle,
    position: Vec3,
) -> Entity {
    commands.spawn((
        Player::new(player_id, style),
        StyleProperties::from_style(style),
        Health::new(100.0),
        Transform::from_translation(position),
        GlobalTransform::default(),
        Name::new(format!("Player_{}", player_id)),
    )).id()
}