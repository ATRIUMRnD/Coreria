# Research: ChaosForge Multiplayer Enhancement

**Date**: October 13, 2025 | **Feature**: 011-enhance-chaosforge-multiplayer

## Research Objectives
Resolve 4 NEEDS CLARIFICATION items from specification to enable implementation planning.

## 1. Multiplayer Networking Architecture

### Decision: Client-Server with Authoritative Physics
**Rationale**: 
- Prevents cheating through authoritative server validation
- Handles player disconnections gracefully
- Scales better than P2P for 4-8 players
- Enables spectator mode and replay systems

**Alternatives Considered**:
- P2P: Lower latency but vulnerable to cheating, complex NAT traversal
- Hybrid: Complex implementation, minimal benefits for target scale

**Implementation Details**:
- UDP-based protocol for real-time updates
- TCP for reliable match setup/teardown
- Server tick rate: 60Hz (matches physics simulation)
- Client prediction with rollback for input lag mitigation

## 2. Combo Timing and Input System

### Decision: Frame-Perfect with 3-Frame Buffer Window
**Rationale**:
- Balances skill requirement with accessibility
- Industry standard for fighting games (3-6 frame windows)
- 60Hz timing provides consistent 16.67ms per frame
- Buffer allows for human input timing variation

**Alternatives Considered**:
- Strict frame-perfect: Too difficult for casual players
- Generous 10+ frame window: Reduces skill ceiling
- Variable timing: Inconsistent across network conditions

**Implementation Details**:
- Input buffer: Store last 3 frames of input
- Combo validation: Check sequences against style definitions
- Network synchronization: Include input timestamps
- Rollback: Re-validate combos on state correction

## 3. Network Synchronization Strategy

### Decision: Client Prediction + Server Reconciliation
**Rationale**:
- Maintains responsive feel despite network latency
- Authoritative server prevents desync issues
- Proven pattern for real-time multiplayer games
- ODE physics determinism enables rollback

**Alternatives Considered**:
- Lockstep: Unplayable with network jitter
- Full client authority: Vulnerable to cheating
- State streaming only: Poor responsiveness

**Implementation Details**:
- Client: Predict physics locally, send inputs to server
- Server: Simulate authoritative world state, broadcast updates
- Rollback: Client corrects prediction errors using server state
- Compression: Delta compression for physics state updates

## 4. ODE Ragdoll Physics Parameters

### Decision: Conservative Joint Constraints for Stability
**Rationale**:
- Stability more important than perfect realism
- Players prioritize consistent gameplay over accurate physics
- Conservative limits prevent explosive joint failures
- Tunable parameters allow per-style customization

**Joint Configuration**:
```c
// Ball joints (shoulders, hips)
dJointSetBallAnchor(joint, x, y, z);
dJointSetBallParam(joint, dParamCFM, 0.001f);     // Constraint Force Mixing
dJointSetBallParam(joint, dParamERP, 0.8f);       // Error Reduction Parameter

// Hinge joints (elbows, knees)  
dJointSetHingeAxis(joint, 0, 0, 1);
dJointSetHingeParam(joint, dParamLoStop, -M_PI/2); // Joint limits
dJointSetHingeParam(joint, dParamHiStop, M_PI/2);
dJointSetHingeParam(joint, dParamFMax, 500.0f);    // Maximum force
```

**Body Mass Distribution**:
- Head: 5kg (lighter for stability)
- Torso: 40kg (center of mass)
- Arms: 8kg each (moderate for combat)
- Legs: 12kg each (heavier for grounding)

**Style-Specific Modifications**:
- **Brawler**: +20% mass across all bodies, +50% joint strength
- **Striker**: -10% mass, +25% joint flexibility  
- **Phantom**: -20% mass, reduced collision response
- **Titan**: +50% mass, +100% joint strength, slower movement

## 5. Rust-C FFI Best Practices

### Decision: Minimal FFI Surface with C-Compatible Types
**Rationale**:
- Reduces complexity at language boundary
- Safer memory management with clear ownership
- Performance-critical code stays in Rust
- C layer handles only UI and high-level game logic

**Interface Design**:
```c
// FFI types - C compatible
typedef struct {
    int32_t player_id;
    float x, y, z;
    int32_t style_id;
} CPlayer;

// FFI functions - simple signatures
int32_t coreria_create_match(int32_t max_players);
int32_t coreria_add_player(int32_t match_id, int32_t style_id);
void coreria_apply_force(int32_t player_id, float fx, float fy, float fz);
int32_t coreria_step_simulation(float delta_time);
```

**Memory Management**:
- Rust owns all game state
- C receives read-only pointers or copies
- No C malloc/free of Rust objects
- Explicit cleanup functions for resources

## 6. Bevy ECS Networking Patterns

### Decision: Bevy_renet for Real-time Networking
**Rationale**:
- Native Bevy integration with ECS systems
- Built for real-time games (UDP-based)
- Handles serialization automatically
- Active community and documentation

**Architecture**:
```rust
// Server systems
fn handle_player_input(mut commands: Commands, mut events: EventReader<PlayerInput>) {
    // Process input events, update physics
}

fn broadcast_state(query: Query<(Entity, &Transform, &Velocity), With<Player>>) {
    // Send physics state to all clients
}

// Client systems  
fn send_input(input: Res<Input<KeyCode>>, mut client: ResMut<RenetClient>) {
    // Send player input to server
}

fn apply_server_state(mut query: Query<&mut Transform>, mut events: EventReader<ServerUpdate>) {
    // Apply authoritative state from server
}
```

## Implementation Readiness
- [x] Networking architecture defined
- [x] Input system parameters established  
- [x] Synchronization strategy chosen
- [x] Physics parameters researched
- [x] FFI patterns identified
- [x] Bevy networking approach selected

**Status**: Ready for Phase 1 design and contracts