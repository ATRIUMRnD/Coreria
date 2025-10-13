# Feature Specification: ChaosForge Gang Beasts-Style Ragdoll Physics

**Feature Branch**: `005-chaosforge-ragdoll-physics`
**Created**: October 12, 2025
**Status**: Draft
**Input**: Extend ChaosForge game loop for Gang Beasts-like ragdoll characters: Multi-limb players (head, torso, arms, legs as ODE bodies/joints). Random fighting styles apply forces to limbs (e.g., Striker Tornado Kick spins arms). 4-8 ragdoll players spawn in coliseum; physics sim per frame. Menu selects style pre-match. Windowed OpenGL (800x600). Implement in C with GLFW/ODE; copyable code. Focus: Emergent chaos from limb interactions, no hitboxes—collision-based combat.

## Overview

Transform ChaosForge from simple cube-based players to fully articulated ragdoll characters with Gang Beasts-style physics. Each player becomes a multi-body system with realistic joint constraints, creating emergent and chaotic combat through physics interactions rather than traditional hitbox-based combat.

## Core Requirements

### 1. Ragdoll Character System
- **Multi-Body Players**: Each player consists of 6-8 ODE rigid bodies:
  - Head (sphere, 0.3m radius)
  - Torso (box, 0.8m x 0.6m x 0.4m)
  - Upper Arms x2 (capsules, 0.4m length, 0.1m radius)
  - Lower Arms x2 (capsules, 0.4m length, 0.08m radius)
  - Upper Legs x2 (capsules, 0.5m length, 0.12m radius)
  - Lower Legs x2 (capsules, 0.5m length, 0.1m radius)

- **Joint System**: Connect bodies with ODE ball-and-socket joints:
  - Neck: Head ↔ Torso (limited rotation)
  - Shoulders: Upper Arms ↔ Torso (full rotation)
  - Elbows: Upper Arms ↔ Lower Arms (hinge, 0-150°)
  - Hips: Upper Legs ↔ Torso (limited rotation)
  - Knees: Upper Legs ↔ Lower Legs (hinge, 0-150°)

### 2. Physics-Based Combat
- **No Hitboxes**: Combat occurs through direct physics collisions between body parts
- **Force Application**: Fighting styles apply forces/torques to specific limbs
- **Momentum Transfer**: Collisions transfer momentum realistically between ragdolls
- **Damage System**: Health decreases based on collision impact forces

### 3. Fighting Style Integration
- **Brawler**: Heavy torso forces, arm swings
  - Combo Extender 1: "Haymaker" - massive force to right arm
  - Guard Break: "Body Slam" - forward torso impulse
  - Ultimate: "Ground Pound" - downward torso force + area effect

- **Striker**: Leg-focused attacks, spinning motions
  - Combo Extender 1: "Tornado Kick" - rotational force to legs
  - Guard Break: "Sweep" - horizontal leg force
  - Ultimate: "Hurricane" - full-body spin with leg extensions

- **Phantom**: Quick, precise limb movements
  - Combo Extender 1: "Phantom Jab" - rapid arm extension
  - Guard Break: "Phase Strike" - teleport + arm force
  - Ultimate: "Shadow Clone" - multiple simultaneous limb forces

- **Titan**: Slow, powerful full-body attacks
  - Combo Extender 1: "Titan Slam" - both arms downward
  - Guard Break: "Earthquake" - ground impact + shockwave
  - Ultimate: "Colossus" - all limbs extend with maximum force

### 4. Enhanced Game Loop
- **Physics Step**: ODE world step at 60Hz (0.016s timestep)
- **Force Application**: Apply fighting style forces each frame based on input
- **Collision Detection**: Monitor body-to-body collisions for damage
- **Stability**: Joint constraints prevent unrealistic poses
- **Recovery**: Automatic joint motor forces to maintain upright posture

## Technical Implementation

### Data Structures
```c
typedef struct {
    dBodyID body;
    dGeomID geom;
    float mass;
    Vector3 size;
} RagdollPart;

typedef struct {
    RagdollPart head;
    RagdollPart torso;
    RagdollPart upper_arms[2];
    RagdollPart lower_arms[2];
    RagdollPart upper_legs[2];
    RagdollPart lower_legs[2];
    dJointID joints[8];  // neck, shoulders, elbows, hips, knees
    int player_id;
    int fighting_style;
    float health;
    int lives;
} RagdollPlayer;

typedef struct {
    dWorldID world;
    dSpaceID space;
    dJointGroupID contact_group;
    RagdollPlayer players[8];
    int player_count;
} PhysicsWorld;
```

### Core Functions
```c
// Ragdoll management
RagdollPlayer create_ragdoll_player(dWorldID world, dSpaceID space, Vector3 spawn_pos, int style);
void destroy_ragdoll_player(RagdollPlayer* player);
void apply_fighting_style_forces(RagdollPlayer* player, int move_type, float intensity);

// Physics simulation
void physics_step(PhysicsWorld* world, float dt);
void handle_collisions(PhysicsWorld* world);
void apply_joint_constraints(RagdollPlayer* player);

// Rendering
void render_ragdoll_player(RagdollPlayer* player);
void render_physics_debug(PhysicsWorld* world);
```

## Success Criteria

1. **Functional Ragdolls**: 4-8 players spawn as articulated ragdoll characters
2. **Realistic Physics**: Bodies move and collide naturally with proper momentum
3. **Style-Based Combat**: Each fighting style produces distinct movement patterns
4. **Emergent Chaos**: Unpredictable but entertaining interactions between ragdolls
5. **Performance**: Maintains 60 FPS with full physics simulation
6. **Visual Feedback**: Clear representation of ragdoll state and collisions

## Non-Functional Requirements

- **Performance**: 60 FPS with 8 ragdoll players (48 rigid bodies, 64 joints)
- **Stability**: No joint explosions or unrealistic poses
- **Responsiveness**: Input to force application latency < 16ms
- **Memory**: Total physics memory usage < 50MB
- **Compatibility**: Works with existing ChaosForge codebase

## Out of Scope

- Advanced ragdoll animations or IK
- Cloth or soft body physics
- Facial expressions or detailed character models
- Network synchronization of physics state
- Advanced AI for ragdoll movement

## Dependencies

- **ODE Physics Engine**: Already integrated in ChaosForge
- **OpenGL Rendering**: Existing rendering system
- **Input System**: Current keyboard/mouse handling
- **Game State Management**: Existing player and match systems

## Acceptance Tests

1. **Ragdoll Creation**: Spawn 4 ragdoll players in arena, verify all joints functional
2. **Physics Simulation**: Drop ragdolls from height, verify realistic falling/bouncing
3. **Combat Forces**: Apply Brawler Haymaker, verify arm swings and impacts opponent
4. **Style Differentiation**: Execute all 4 fighting styles, verify distinct behaviors
5. **Collision Damage**: Verify health decreases on high-impact collisions
6. **Performance**: Maintain 60 FPS with 8 active ragdoll players
7. **Stability**: Run 10-minute match without joint failures or crashes

This specification transforms ChaosForge into a physics-driven combat experience where the emergent behavior of ragdoll interactions creates the core gameplay, similar to Gang Beasts but with the structured fighting styles of the original ChaosForge design.
