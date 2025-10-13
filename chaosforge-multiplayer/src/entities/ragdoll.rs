use bevy::prelude::*;
use serde::{Deserialize, Serialize};

/// Ragdoll body component with 6-body structure
#[derive(Component, Debug, Clone, Reflect, Serialize, Deserialize)]
#[reflect(Component)]
pub struct RagdollBody {
    /// Head entity with physics body
    pub head: Option<Entity>,
    
    /// Torso entity (center of mass)
    pub torso: Option<Entity>,
    
    /// Left arm entity
    pub left_arm: Option<Entity>,
    
    /// Right arm entity  
    pub right_arm: Option<Entity>,
    
    /// Left leg entity
    pub left_leg: Option<Entity>,
    
    /// Right leg entity
    pub right_leg: Option<Entity>,
    
    /// Physics joint handles connecting bodies
    pub joints: Vec<Entity>,
    
    /// Total mass of ragdoll
    pub total_mass: f32,
    
    /// Style-specific physics scaling
    pub mass_scale: f32,
}

impl Default for RagdollBody {
    fn default() -> Self {
        Self {
            head: None,
            torso: None,
            left_arm: None,
            right_arm: None,
            left_leg: None,
            right_leg: None,
            joints: Vec::new(),
            total_mass: 65.0, // Average human mass in kg
            mass_scale: 1.0,
        }
    }
}

impl RagdollBody {
    /// Create ragdoll with style-specific mass scaling
    pub fn with_style_scaling(mass_scale: f32) -> Self {
        Self {
            mass_scale,
            total_mass: 65.0 * mass_scale,
            ..Default::default()
        }
    }

    /// Get mass for specific body part
    pub fn body_part_mass(&self, part: BodyPart) -> f32 {
        let base_mass = match part {
            BodyPart::Head => 5.0,      // Lighter for stability
            BodyPart::Torso => 40.0,    // Center of mass
            BodyPart::LeftArm | BodyPart::RightArm => 8.0,  // Moderate for combat
            BodyPart::LeftLeg | BodyPart::RightLeg => 12.0, // Heavier for grounding
        };
        base_mass * self.mass_scale
    }

    /// Check if ragdoll is fully constructed
    pub fn is_complete(&self) -> bool {
        self.head.is_some()
            && self.torso.is_some() 
            && self.left_arm.is_some()
            && self.right_arm.is_some()
            && self.left_leg.is_some()
            && self.right_leg.is_some()
            && self.joints.len() >= 5 // Minimum joints needed
    }

    /// Get all body part entities
    pub fn all_bodies(&self) -> Vec<Entity> {
        let mut bodies = Vec::new();
        if let Some(entity) = self.head { bodies.push(entity); }
        if let Some(entity) = self.torso { bodies.push(entity); }
        if let Some(entity) = self.left_arm { bodies.push(entity); }
        if let Some(entity) = self.right_arm { bodies.push(entity); }
        if let Some(entity) = self.left_leg { bodies.push(entity); }
        if let Some(entity) = self.right_leg { bodies.push(entity); }
        bodies
    }
}

/// Individual body parts of the ragdoll
#[derive(Debug, Clone, Copy, PartialEq, Eq, Reflect)]
pub enum BodyPart {
    Head,
    Torso,
    LeftArm,
    RightArm, 
    LeftLeg,
    RightLeg,
}

/// Component for individual ragdoll body parts
#[derive(Component, Debug, Clone, Reflect)]
#[reflect(Component)]
pub struct RagdollPart {
    pub part_type: BodyPart,
    pub parent_ragdoll: Entity,
    pub mass: f32,
    pub size: Vec3,
    pub collision_groups: u32,
}

impl RagdollPart {
    pub fn new(part_type: BodyPart, parent: Entity, mass: f32) -> Self {
        let size = match part_type {
            BodyPart::Head => Vec3::new(0.3, 0.3, 0.3),      // Spherical head
            BodyPart::Torso => Vec3::new(0.6, 0.8, 0.3),     // Rectangular torso
            BodyPart::LeftArm | BodyPart::RightArm => Vec3::new(0.15, 0.6, 0.15), // Cylindrical arms
            BodyPart::LeftLeg | BodyPart::RightLeg => Vec3::new(0.2, 0.8, 0.2),   // Cylindrical legs
        };

        Self {
            part_type,
            parent_ragdoll: parent,
            mass,
            size,
            collision_groups: 0b0001, // Default collision group
        }
    }
}

/// Physics joint connecting ragdoll parts
#[derive(Component, Debug, Clone, Reflect)]
#[reflect(Component)]
pub struct RagdollJoint {
    pub joint_type: JointType,
    pub body_a: Entity,
    pub body_b: Entity,
    pub anchor_a: Vec3,
    pub anchor_b: Vec3,
    pub limits: JointLimits,
    pub max_force: f32,
    pub cfm: f32, // Constraint Force Mixing
    pub erp: f32, // Error Reduction Parameter
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Reflect)]
pub enum JointType {
    Ball,   // Ball joint (shoulders, hips)
    Hinge,  // Hinge joint (elbows, knees)
    Fixed,  // Fixed joint (head to torso)
}

/// Joint constraint limits
#[derive(Debug, Clone, Reflect)]
pub struct JointLimits {
    pub linear_min: Option<Vec3>,
    pub linear_max: Option<Vec3>,
    pub angular_min: Option<Vec3>,
    pub angular_max: Option<Vec3>,
}

impl JointLimits {
    /// Conservative limits for stability (from research.md)
    pub fn conservative() -> Self {
        Self {
            linear_min: None,
            linear_max: None,
            angular_min: Some(Vec3::splat(-std::f32::consts::PI / 2.0)),
            angular_max: Some(Vec3::splat(std::f32::consts::PI / 2.0)),
        }
    }

    /// More flexible limits for striker-style characters
    pub fn flexible() -> Self {
        Self {
            linear_min: None,
            linear_max: None,
            angular_min: Some(Vec3::splat(-std::f32::consts::PI * 0.75)),
            angular_max: Some(Vec3::splat(std::f32::consts::PI * 0.75)),
        }
    }

    /// Rigid limits for titan-style characters
    pub fn rigid() -> Self {
        Self {
            linear_min: None,
            linear_max: None,
            angular_min: Some(Vec3::splat(-std::f32::consts::PI / 3.0)),
            angular_max: Some(Vec3::splat(std::f32::consts::PI / 3.0)),
        }
    }
}

/// System to update ragdoll physics bodies
pub fn update_ragdoll_bodies(
    mut ragdoll_query: Query<(Entity, &mut RagdollBody, &crate::entities::StyleProperties)>,
    mut part_query: Query<(&mut Transform, &RagdollPart)>,
    time: Res<Time>,
) {
    for (ragdoll_entity, mut ragdoll, style_props) in ragdoll_query.iter_mut() {
        // Update mass scaling based on fighting style
        ragdoll.mass_scale = style_props.mass_multiplier;
        ragdoll.total_mass = 65.0 * ragdoll.mass_scale;

        // Update individual body part masses and positions
        for body_entity in ragdoll.all_bodies() {
            if let Ok((mut transform, part)) = part_query.get_mut(body_entity) {
                // Apply style-specific physics modifications
                // This is where we'd integrate with the physics engine
                // For now, just ensure the component is updated
                let _scaled_mass = part.mass * style_props.mass_multiplier;
                
                // Basic stability checks (placeholder for ODE/Rapier integration)
                if transform.translation.y < -10.0 {
                    // Reset if fallen too far
                    transform.translation.y = 2.0;
                }
            }
        }
    }
}

/// Spawn a complete ragdoll for a player
pub fn spawn_ragdoll(
    commands: &mut Commands,
    player_entity: Entity,
    position: Vec3,
    style_mass_scale: f32,
) -> RagdollBody {
    let mut ragdoll = RagdollBody::with_style_scaling(style_mass_scale);

    // Spawn head
    let head_pos = position + Vec3::new(0.0, 1.7, 0.0);
    let head = commands.spawn((
        RagdollPart::new(BodyPart::Head, player_entity, ragdoll.body_part_mass(BodyPart::Head)),
        Transform::from_translation(head_pos),
        GlobalTransform::default(),
        Name::new("RagdollHead"),
    )).id();
    ragdoll.head = Some(head);

    // Spawn torso (center of mass)
    let torso_pos = position + Vec3::new(0.0, 1.0, 0.0);
    let torso = commands.spawn((
        RagdollPart::new(BodyPart::Torso, player_entity, ragdoll.body_part_mass(BodyPart::Torso)),
        Transform::from_translation(torso_pos),
        GlobalTransform::default(),
        Name::new("RagdollTorso"),
    )).id();
    ragdoll.torso = Some(torso);

    // Spawn arms
    let left_arm_pos = position + Vec3::new(-0.8, 1.0, 0.0);
    let left_arm = commands.spawn((
        RagdollPart::new(BodyPart::LeftArm, player_entity, ragdoll.body_part_mass(BodyPart::LeftArm)),
        Transform::from_translation(left_arm_pos),
        GlobalTransform::default(),
        Name::new("RagdollLeftArm"),
    )).id();
    ragdoll.left_arm = Some(left_arm);

    let right_arm_pos = position + Vec3::new(0.8, 1.0, 0.0);
    let right_arm = commands.spawn((
        RagdollPart::new(BodyPart::RightArm, player_entity, ragdoll.body_part_mass(BodyPart::RightArm)),
        Transform::from_translation(right_arm_pos),
        GlobalTransform::default(),
        Name::new("RagdollRightArm"),
    )).id();
    ragdoll.right_arm = Some(right_arm);

    // Spawn legs
    let left_leg_pos = position + Vec3::new(-0.3, 0.2, 0.0);
    let left_leg = commands.spawn((
        RagdollPart::new(BodyPart::LeftLeg, player_entity, ragdoll.body_part_mass(BodyPart::LeftLeg)),
        Transform::from_translation(left_leg_pos),
        GlobalTransform::default(),
        Name::new("RagdollLeftLeg"),
    )).id();
    ragdoll.left_leg = Some(left_leg);

    let right_leg_pos = position + Vec3::new(0.3, 0.2, 0.0);
    let right_leg = commands.spawn((
        RagdollPart::new(BodyPart::RightLeg, player_entity, ragdoll.body_part_mass(BodyPart::RightLeg)),
        Transform::from_translation(right_leg_pos),
        GlobalTransform::default(),
        Name::new("RagdollRightLeg"),
    )).id();
    ragdoll.right_leg = Some(right_leg);

    // Create joints connecting the bodies
    // Head to torso (fixed joint)
    let head_joint = commands.spawn((
        RagdollJoint {
            joint_type: JointType::Fixed,
            body_a: torso,
            body_b: head,
            anchor_a: Vec3::new(0.0, 0.4, 0.0),
            anchor_b: Vec3::new(0.0, -0.15, 0.0),
            limits: JointLimits::rigid(),
            max_force: 1000.0,
            cfm: 0.001,
            erp: 0.8,
        },
        Name::new("HeadJoint"),
    )).id();
    ragdoll.joints.push(head_joint);

    // Arms to torso (ball joints for shoulder movement)
    let left_shoulder = commands.spawn((
        RagdollJoint {
            joint_type: JointType::Ball,
            body_a: torso,
            body_b: left_arm,
            anchor_a: Vec3::new(-0.3, 0.3, 0.0),
            anchor_b: Vec3::new(0.0, 0.3, 0.0),
            limits: JointLimits::conservative(),
            max_force: 500.0 * style_mass_scale,
            cfm: 0.001,
            erp: 0.8,
        },
        Name::new("LeftShoulderJoint"),
    )).id();
    ragdoll.joints.push(left_shoulder);

    let right_shoulder = commands.spawn((
        RagdollJoint {
            joint_type: JointType::Ball,
            body_a: torso,
            body_b: right_arm,
            anchor_a: Vec3::new(0.3, 0.3, 0.0),
            anchor_b: Vec3::new(0.0, 0.3, 0.0),
            limits: JointLimits::conservative(),
            max_force: 500.0 * style_mass_scale,
            cfm: 0.001,
            erp: 0.8,
        },
        Name::new("RightShoulderJoint"),
    )).id();
    ragdoll.joints.push(right_shoulder);

    // Legs to torso (ball joints for hip movement)
    let left_hip = commands.spawn((
        RagdollJoint {
            joint_type: JointType::Ball,
            body_a: torso,
            body_b: left_leg,
            anchor_a: Vec3::new(-0.15, -0.4, 0.0),
            anchor_b: Vec3::new(0.0, 0.4, 0.0),
            limits: JointLimits::conservative(),
            max_force: 800.0 * style_mass_scale,
            cfm: 0.001,
            erp: 0.8,
        },
        Name::new("LeftHipJoint"),
    )).id();
    ragdoll.joints.push(left_hip);

    let right_hip = commands.spawn((
        RagdollJoint {
            joint_type: JointType::Ball,
            body_a: torso,
            body_b: right_leg,
            anchor_a: Vec3::new(0.15, -0.4, 0.0),
            anchor_b: Vec3::new(0.0, 0.4, 0.0),
            limits: JointLimits::conservative(),
            max_force: 800.0 * style_mass_scale,
            cfm: 0.001,
            erp: 0.8,
        },
        Name::new("RightHipJoint"),
    )).id();
    ragdoll.joints.push(right_hip);

    ragdoll
}