# Data Model: ChaosForge Multiplayer Enhancement

**Date**: October 13, 2025 | **Feature**: 011-enhance-chaosforge-multiplayer

## Core Data Structures

### Player Entity

```rust
#[derive(Component)]
pub struct Player {
    pub id: PlayerId,
    pub name: String,
    pub style: FightingStyle,
    pub health: Health,
    pub ragdoll: RagdollBody,
    pub input_buffer: InputBuffer,
    pub network_state: NetworkState,
}

#[derive(Component)]
pub struct RagdollBody {
    pub head: Entity,
    pub torso: Entity,
    pub left_arm: Entity,
    pub right_arm: Entity, 
    pub left_leg: Entity,
    pub right_leg: Entity,
    pub joints: Vec<JointHandle>,
}

#[derive(Component)]
pub struct Health {
    pub current: f32,
    pub maximum: f32,
    pub regeneration_rate: f32,
    pub last_damage_time: Instant,
}
```

### Fighting Styles

```rust
#[derive(Clone, Copy, Debug)]
pub enum FightingStyle {
    Brawler,
    Striker, 
    Phantom,
    Titan,
}

#[derive(Component)]
pub struct StyleProperties {
    pub mass_multiplier: f32,
    pub speed_multiplier: f32,
    pub strength_multiplier: f32,
    pub special_abilities: Vec<SpecialAbility>,
    pub combo_definitions: Vec<ComboSequence>,
}
```

### Combat System

```rust
#[derive(Component)]
pub struct CombatState {
    pub current_combo: Option<ActiveCombo>,
    pub combo_meter: f32,
    pub last_hit_time: Instant,
    pub hit_streak: u32,
    pub damage_multiplier: f32,
}

#[derive(Clone)]
pub struct ComboSequence {
    pub inputs: Vec<InputAction>,
    pub timing_windows: Vec<Duration>,
    pub damage: f32,
    pub knockback: Vec3,
    pub style_requirement: FightingStyle,
}
```

### Network Protocol

```rust
// Client -> Server Messages
#[derive(Serialize, Deserialize)]
pub enum ClientMessage {
    JoinMatch { player_name: String, style: FightingStyle },
    Input { 
        frame: u64, 
        actions: Vec<InputAction>, 
        timestamp: Instant 
    },
    LeaveMatch,
}

// Server -> Client Messages  
#[derive(Serialize, Deserialize)]
pub enum ServerMessage {
    MatchJoined { player_id: PlayerId, match_state: MatchState },
    PhysicsUpdate { 
        frame: u64, 
        player_states: Vec<PlayerPhysicsState>,
        timestamp: Instant 
    },
    PlayerDamage { 
        attacker: PlayerId, 
        victim: PlayerId, 
        damage: f32,
        combo: Option<ComboSequence> 
    },
    MatchEnd { winner: Option<PlayerId>, final_scores: Vec<PlayerScore> },
}
```

### Physics Integration

```rust
#[derive(Component)]
pub struct PhysicsBody {
    pub ode_body: BodyId,        // ODE body handle
    pub mass: f32,
    pub collision_groups: u32,
    pub material: PhysicsMaterial,
}

#[derive(Component)]  
pub struct PhysicsJoint {
    pub ode_joint: JointId,      // ODE joint handle
    pub joint_type: JointType,
    pub connected_bodies: (Entity, Entity),
    pub constraints: JointConstraints,
}

pub struct JointConstraints {
    pub linear_limits: Option<(Vec3, Vec3)>,
    pub angular_limits: Option<(Vec3, Vec3)>,
    pub max_force: f32,
    pub cfm: f32,               // Constraint Force Mixing
    pub erp: f32,               // Error Reduction Parameter
}
```

### Match State Management

```rust
#[derive(Resource)]
pub struct MatchState {
    pub match_id: MatchId,
    pub status: MatchStatus,
    pub players: HashMap<PlayerId, Entity>,
    pub start_time: Instant,
    pub time_limit: Duration,
    pub win_condition: WinCondition,
    pub arena_bounds: ArenaBounds,
}

#[derive(Clone, Copy)]
pub enum MatchStatus {
    Waiting,
    Starting { countdown: Duration },
    InProgress,
    Finished { winner: Option<PlayerId> },
}

#[derive(Clone)]
pub struct ArenaBounds {
    pub min: Vec3,
    pub max: Vec3,
    pub death_plane_y: f32,
    pub respawn_points: Vec<Vec3>,
}
```

### Input System

```rust
#[derive(Component)]
pub struct InputBuffer {
    pub frames: VecDeque<InputFrame>,
    pub max_frames: usize,
    pub current_frame: u64,
}

#[derive(Clone, Serialize, Deserialize)]
pub struct InputFrame {
    pub frame_number: u64,
    pub timestamp: Instant,
    pub actions: Vec<InputAction>,
}

#[derive(Clone, Copy, Serialize, Deserialize)]
pub enum InputAction {
    Move { direction: Vec2 },
    Jump,
    Punch,
    Kick,
    Block,
    Grab,
    Special1,
    Special2,
}
```

### Performance Monitoring

```rust
#[derive(Resource)]
pub struct PerformanceMetrics {
    pub frame_time: Duration,
    pub physics_time: Duration,
    pub network_latency: Duration,
    pub update_rate: f32,
    pub player_count: usize,
    pub memory_usage: usize,
}

#[derive(Component)]
pub struct NetworkMetrics {
    pub rtt: Duration,              // Round-trip time
    pub packet_loss: f32,           // Percentage
    pub jitter: Duration,           // Network timing variance  
    pub bandwidth_used: u64,        // Bytes per second
    pub prediction_errors: u32,     // Rollback corrections
}
```

## FFI Interface Types

```c
// C-compatible types for FFI boundary
typedef struct {
    int32_t player_id;
    float x, y, z;              // Position
    float qx, qy, qz, qw;       // Rotation quaternion
    float vx, vy, vz;           // Velocity
    int32_t health;
    int32_t style_id;
} CPlayerState;

typedef struct {
    int32_t match_id;
    int32_t status;             // MatchStatus enum as int
    int32_t player_count;
    float time_remaining;
    CPlayerState players[8];    // Fixed array for simplicity
} CMatchState;

typedef struct {
    int32_t frame_number;
    int32_t action_type;        // InputAction enum as int  
    float param1, param2;       // Action parameters
} CInputAction;
```

## Database Schema (Optional - for persistence)

```sql
-- Player statistics and progression
CREATE TABLE players (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    preferred_style INTEGER NOT NULL,
    matches_played INTEGER DEFAULT 0,
    matches_won INTEGER DEFAULT 0,
    total_damage_dealt FLOAT DEFAULT 0,
    created_at TIMESTAMP DEFAULT NOW()
);

-- Match history
CREATE TABLE matches (
    id SERIAL PRIMARY KEY,
    start_time TIMESTAMP NOT NULL,
    end_time TIMESTAMP,
    winner_id INTEGER REFERENCES players(id),
    duration_seconds INTEGER,
    player_count INTEGER NOT NULL
);

-- Per-player match performance
CREATE TABLE match_participants (
    match_id INTEGER REFERENCES matches(id),
    player_id INTEGER REFERENCES players(id),
    style_used INTEGER NOT NULL,
    damage_dealt FLOAT DEFAULT 0,
    damage_taken FLOAT DEFAULT 0,
    kills INTEGER DEFAULT 0,
    deaths INTEGER DEFAULT 0,
    PRIMARY KEY (match_id, player_id)
);
```

## Memory Layout Considerations

### Rust Side (Performance Critical)
- **ECS Components**: Cache-friendly layout via Bevy's archetype storage
- **Physics State**: Contiguous arrays for ODE integration  
- **Network Buffers**: Ring buffers for input history and state snapshots

### C Side (Legacy Integration)
- **Game State**: Flat structs matching existing C code patterns
- **Rendering Data**: Compatible with OpenGL immediate mode
- **Input Handling**: Direct Windows API integration

## Data Flow Architecture

```
Input Devices → C Layer → FFI → Rust ECS → Physics Engine
                   ↓              ↓           ↓
            Legacy Rendering ← Network ← Game Logic
```

**Key Properties**:
- **Ownership**: Rust owns all dynamic game state
- **Performance**: Physics simulation at 60 Hz in Rust
- **Compatibility**: C layer handles existing windowing/rendering
- **Network**: Authoritative server with client prediction
- **Memory Safety**: Clear boundaries prevent C/Rust memory issues

This data model provides the foundation for implementing the multiplayer ChaosForge enhancement while maintaining compatibility with existing C code and achieving target performance requirements.