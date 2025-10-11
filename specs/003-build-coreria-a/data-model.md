# Data Model: ChaosForge

## Entities

### Player
- id: int
- fighting_style: enum (Brawler, Striker, Phantom, Titan)
- lives: int (2)
- stamina: float
- position: vec3
- state: enum (active, eliminated)

### FightingStyle
- name: string
- combo_extenders: [Move]
- guard_break: Move
- ultimate: Move

### Arena
- boundaries: shape/mesh
- destructible_elements: [Element]
- shrink_state: float

### MatchState
- players: [Player]
- arena: Arena
- time: float
- winner: Player/null

### PhysicsSystem
- momentum: float
- collision_events: [Event]
- ragdoll_state: [Player]
