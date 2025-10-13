

/// Conservative joint constraint parameters from research.md
pub struct JointConstraintConfig {
    pub cfm: f32,         // Constraint Force Mixing
    pub erp: f32,         // Error Reduction Parameter  
    pub max_force: f32,   // Maximum joint force
    pub angular_limit: f32, // Joint angular limits
}

impl JointConstraintConfig {
    /// Conservative settings for stability
    pub fn conservative() -> Self {
        Self {
            cfm: 0.001,
            erp: 0.8,
            max_force: 500.0,
            angular_limit: std::f32::consts::PI / 2.0, // 90 degrees
        }
    }

    /// Settings for Brawler style (stronger joints)
    pub fn brawler() -> Self {
        Self {
            cfm: 0.001,
            erp: 0.8, 
            max_force: 750.0,   // +50% strength
            angular_limit: std::f32::consts::PI / 2.0,
        }
    }

    /// Settings for Striker style (more flexible)
    pub fn striker() -> Self {
        Self {
            cfm: 0.002,         // More flexible
            erp: 0.7,
            max_force: 625.0,   // +25% flexibility
            angular_limit: std::f32::consts::PI * 0.75, // More range of motion
        }
    }

    /// Settings for Phantom style (normal flexibility)
    pub fn phantom() -> Self {
        Self::conservative()
    }

    /// Settings for Titan style (very strong joints)
    pub fn titan() -> Self {
        Self {
            cfm: 0.0005,        // Very rigid
            erp: 0.9,
            max_force: 1000.0,  // +100% strength
            angular_limit: std::f32::consts::PI / 3.0, // Limited range for stability
        }
    }

    /// Get config for specific fighting style
    pub fn for_style(style: crate::entities::FightingStyle) -> Self {
        match style {
            crate::entities::FightingStyle::Brawler => Self::brawler(),
            crate::entities::FightingStyle::Striker => Self::striker(),
            crate::entities::FightingStyle::Phantom => Self::phantom(),
            crate::entities::FightingStyle::Titan => Self::titan(),
        }
    }
}