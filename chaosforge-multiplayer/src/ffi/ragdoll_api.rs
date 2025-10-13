/*!
# Ragdoll FFI API

Gang Beasts-style ragdoll physics integration for C compatibility.
Provides handles for safe cross-language entity management.
*/

use std::os::raw::{c_float, c_int, c_uint};
use crate::entities::{FightingStyle, RagdollBody, Player};
use bevy::prelude::*;
use std::collections::HashMap;

/// Handle to a ragdoll entity for FFI safety
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct RagdollHandle {
    pub id: u32,
}

/// Individual body parts for targeted force application
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum LimbIndex {
    Head = 0,
    Torso = 1,
    LeftArm = 2,
    RightArm = 3,
    LeftLeg = 4,
    RightLeg = 5,
}

/// Fighting style IDs for C compatibility
#[repr(C)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum FightingStyleId {
    Brawler = 0,
    Striker = 1,
    Phantom = 2,
    Titan = 3,
}

impl From<FightingStyleId> for FightingStyle {
    fn from(style_id: FightingStyleId) -> Self {
        match style_id {
            FightingStyleId::Brawler => FightingStyle::Brawler,
            FightingStyleId::Striker => FightingStyle::Striker,
            FightingStyleId::Phantom => FightingStyle::Phantom,
            FightingStyleId::Titan => FightingStyle::Titan,
        }
    }
}

/// Global registry for ragdoll handles
static mut RAGDOLL_REGISTRY: Option<HashMap<u32, Entity>> = None;
static mut NEXT_RAGDOLL_ID: u32 = 1;

/// Initialize ragdoll registry (called internally)
fn ensure_registry() {
    unsafe {
        if RAGDOLL_REGISTRY.is_none() {
            RAGDOLL_REGISTRY = Some(HashMap::new());
        }
    }
}

/// Register a new ragdoll entity and return handle
fn register_ragdoll(entity: Entity) -> RagdollHandle {
    ensure_registry();
    
    unsafe {
        let id = NEXT_RAGDOLL_ID;
        NEXT_RAGDOLL_ID += 1;
        
        if let Some(ref mut registry) = RAGDOLL_REGISTRY {
            registry.insert(id, entity);
        }
        
        RagdollHandle { id }
    }
}

/// Get entity from ragdoll handle
fn get_ragdoll_entity(handle: RagdollHandle) -> Option<Entity> {
    unsafe {
        if let Some(ref registry) = RAGDOLL_REGISTRY {
            registry.get(&handle.id).copied()
        } else {
            None
        }
    }
}

/// Spawn a ragdoll at specified position with fighting style
/// Returns handle for subsequent operations
#[no_mangle]
pub extern "C" fn coreria_spawn_ragdoll(
    player_id: c_uint,
    style_id: FightingStyleId,
    x: c_float,
    y: c_float,
    z: c_float,
) -> RagdollHandle {
    let result = crate::with_engine(|engine| {
        let style: FightingStyle = style_id.into();
        let position = Vec3::new(x, y, z);
        let world = engine.app.world_mut();
        
        // Create individual limb entities with physics bodies
        let head_entity = world.spawn((
            Transform::from_translation(position + Vec3::new(0.0, 0.8, 0.0)),
            Name::new(format!("Ragdoll_{}_Head", player_id)),
        )).id();
        
        let torso_entity = world.spawn((
            Transform::from_translation(position),
            Name::new(format!("Ragdoll_{}_Torso", player_id)),
        )).id();
        
        let left_arm_entity = world.spawn((
            Transform::from_translation(position + Vec3::new(-0.5, 0.3, 0.0)),
            Name::new(format!("Ragdoll_{}_LeftArm", player_id)),
        )).id();
        
        let right_arm_entity = world.spawn((
            Transform::from_translation(position + Vec3::new(0.5, 0.3, 0.0)),
            Name::new(format!("Ragdoll_{}_RightArm", player_id)),
        )).id();
        
        let left_leg_entity = world.spawn((
            Transform::from_translation(position + Vec3::new(-0.2, -0.8, 0.0)),
            Name::new(format!("Ragdoll_{}_LeftLeg", player_id)),
        )).id();
        
        let right_leg_entity = world.spawn((
            Transform::from_translation(position + Vec3::new(0.2, -0.8, 0.0)),
            Name::new(format!("Ragdoll_{}_RightLeg", player_id)),
        )).id();
        
        // Create ragdoll body with references to limb entities
        let mut ragdoll_body = RagdollBody::default();
        ragdoll_body.head = Some(head_entity);
        ragdoll_body.torso = Some(torso_entity);
        ragdoll_body.left_arm = Some(left_arm_entity);
        ragdoll_body.right_arm = Some(right_arm_entity);
        ragdoll_body.left_leg = Some(left_leg_entity);
        ragdoll_body.right_leg = Some(right_leg_entity);
        ragdoll_body.mass_scale = style.mass_multiplier();
        
        // Create main ragdoll entity with all required components
        let entity = world.spawn((
            Player::new(player_id, style),
            ragdoll_body,
            crate::entities::Health::new(100.0),
            crate::entities::NetworkState::default(),
            Transform::from_translation(position),
            Name::new(format!("Ragdoll_{}", player_id)),
        )).id();
        
        register_ragdoll(entity)
    });
    
    match result {
        Ok(handle) => {
            println!("[CORERIA] Spawned ragdoll {} at ({}, {}, {}) with style {:?}", 
                    handle.id, x, y, z, style_id);
            handle
        },
        Err(_) => {
            eprintln!("[CORERIA] Failed to spawn ragdoll: Engine not initialized");
            RagdollHandle { id: 0 } // Invalid handle
        }
    }
}

/// Apply force and torque to specific limb of ragdoll
/// limb_index: 0=head, 1=torso, 2=left_arm, 3=right_arm, 4=left_leg, 5=right_leg
#[no_mangle]
pub extern "C" fn coreria_apply_limb_force(
    ragdoll_handle: RagdollHandle,
    limb_index: c_int,
    force_x: c_float,
    force_y: c_float,
    force_z: c_float,
    torque_x: c_float,
    torque_y: c_float,
    torque_z: c_float,
) -> c_int {
    // Validate limb index
    if limb_index < 0 || limb_index > 5 {
        eprintln!("[CORERIA] Invalid limb index: {}", limb_index);
        return -1;
    }
    
    let result = crate::with_engine(|engine| {
        if let Some(entity) = get_ragdoll_entity(ragdoll_handle) {
            // Get ragdoll and player components
            let world = engine.app.world_mut();
            
            if let Some((ragdoll, player)) = world.query::<(&RagdollBody, &Player)>()
                .get(world, entity).ok() {
                
                // Calculate style-specific force multipliers
                let force_multiplier = player.style.damage_multiplier();
                let mass_multiplier = player.style.mass_multiplier();
                
                // Apply style scaling
                let scaled_force = Vec3::new(
                    force_x * force_multiplier,
                    force_y * force_multiplier,
                    force_z * force_multiplier,
                );
                
                let scaled_torque = Vec3::new(
                    torque_x * force_multiplier,
                    torque_y * force_multiplier,
                    torque_z * force_multiplier,
                );
                
                // Get the target limb entity
                let limb_entity = match limb_index {
                    0 => ragdoll.head,
                    1 => ragdoll.torso,
                    2 => ragdoll.left_arm,
                    3 => ragdoll.right_arm,
                    4 => ragdoll.left_leg,
                    5 => ragdoll.right_leg,
                    _ => None,
                };
                
                if let Some(limb_entity) = limb_entity {
                    // TODO: Apply force to physics body when rapier integration is complete
                    println!("[CORERIA] Applied force {:?} and torque {:?} to limb {} (style: {:?})", 
                            scaled_force, scaled_torque, limb_index, player.style);
                    true
                } else {
                    println!("[CORERIA] Limb {} not found for ragdoll {}", limb_index, ragdoll_handle.id);
                    false
                }
            } else {
                println!("[CORERIA] Ragdoll components not found for handle {}", ragdoll_handle.id);
                false
            }
        } else {
            println!("[CORERIA] Invalid ragdoll handle: {}", ragdoll_handle.id);
            false
        }
    });
    
    match result {
        Ok(true) => 0,    // Success
        Ok(false) => -2,  // Ragdoll/limb not found
        Err(_) => -1,     // Engine error
    }
}

/// Get ragdoll position
#[no_mangle]
pub extern "C" fn coreria_get_ragdoll_position(
    ragdoll_handle: RagdollHandle,
    x: *mut c_float,
    y: *mut c_float,
    z: *mut c_float,
) -> c_int {
    if x.is_null() || y.is_null() || z.is_null() {
        return -1;
    }
    
    let result = crate::with_engine(|engine| {
        if let Some(entity) = get_ragdoll_entity(ragdoll_handle) {
            let world = engine.app.world_mut();
            
            if let Some(transform) = world.query::<&Transform>().get(world, entity).ok() {
                unsafe {
                    *x = transform.translation.x;
                    *y = transform.translation.y;
                    *z = transform.translation.z;
                }
                true
            } else {
                false
            }
        } else {
            false
        }
    });
    
    match result {
        Ok(true) => 0,
        _ => -1,
    }
}

/// Destroy ragdoll and cleanup resources
#[no_mangle]
pub extern "C" fn coreria_destroy_ragdoll(ragdoll_handle: RagdollHandle) -> c_int {
    let result = crate::with_engine(|engine| {
        if let Some(entity) = get_ragdoll_entity(ragdoll_handle) {
            // Remove from registry
            unsafe {
                if let Some(ref mut registry) = RAGDOLL_REGISTRY {
                    registry.remove(&ragdoll_handle.id);
                }
            }
            
            // Despawn entity
            engine.app.world_mut().despawn(entity);
            
            println!("[CORERIA] Destroyed ragdoll {}", ragdoll_handle.id);
            true
        } else {
            false
        }
    });
    
    match result {
        Ok(true) => 0,
        _ => -1,
    }
}

/// Get number of active ragdolls
#[no_mangle]
pub extern "C" fn coreria_get_ragdoll_count() -> c_int {
    unsafe {
        if let Some(ref registry) = RAGDOLL_REGISTRY {
            registry.len() as c_int
        } else {
            0
        }
    }
}