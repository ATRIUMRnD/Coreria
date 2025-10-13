# Implementation Plan: ChaosForge Multiplayer Arena Deathmatch with Ragdoll Physics

**Branch**: `011-enhance-chaosforge-multiplayer` | **Date**: October 13, 2025 | **Spec**: [../spec.md](./spec.md)
**Input**: Feature specification from `/specs/011-enhance-chaosforge-multiplayer/spec.md`

## Execution Flow (/plan command scope)
```
1. Load feature spec from Input path
   → Feature found: ChaosForge multiplayer enhancement with ragdoll physics
2. Fill Technical Context (scan for NEEDS CLARIFICATION)
   → Project Type: Hybrid Rust/C game engine (Bevy ECS + C game layer)
   → Architecture: Rust core exposes C FFI for game logic
3. Constitution Check: Hybrid architecture complexity justified by performance needs
4. Evaluate Constitution Check section below
   → No violations: Hybrid approach necessary for existing C codebase integration
   → Update Progress Tracking: Initial Constitution Check ✓
5. Execute Phase 0 → research.md
   → Resolve 4 NEEDS CLARIFICATION items from spec
6. Execute Phase 1 → contracts, data-model.md, quickstart.md
7. Re-evaluate Constitution Check section
   → Architecture remains justified
   → Update Progress Tracking: Post-Design Constitution Check ✓
8. Plan Phase 2 → Task generation for hybrid Rust/C implementation
9. STOP - Ready for /tasks command
```

## Summary
Transform ChaosForge from single-player arena to 4-8 player multiplayer deathmatch featuring Gang Beasts-style ragdoll physics. Players spawn with random fighting styles (Brawler/Striker/Phantom/Titan), each having unique combo systems and ultimate abilities. Characters are multi-limb ragdolls with collision-based combat in a dynamically shrinking arena. Technical approach: Rust engine core (Bevy ECS + ODE physics) with C game layer calling Rust APIs via FFI.

## Technical Context
**Language/Version**: Rust 1.75 + C11 (GCC/Clang)  
**Primary Dependencies**: Bevy ECS 0.12, ODE physics, GLFW 3.3, OpenGL 3.3+  
**Storage**: In-memory game state, configuration files  
**Testing**: cargo test (Rust), CTest (C integration)  
**Target Platform**: Windows/Linux desktop (800x600 windowed)  
**Project Type**: Hybrid game engine (Rust core + C game layer)  
**Performance Goals**: 60 FPS with 8 ragdoll players, <16ms physics step  
**Constraints**: Real-time multiplayer <100ms latency, stable ragdoll physics  
**Scale/Scope**: 4-8 concurrent players, 4 fighting styles, 10+ abilities

## Constitution Check
*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**Complexity Justification**: 
- ✅ **Hybrid Architecture**: Required to integrate with existing C codebase while leveraging Rust's memory safety for physics
- ✅ **Multi-body Ragdolls**: Essential for Gang Beasts-style physics gameplay
- ✅ **Real-time Multiplayer**: Core feature requirement for arena deathmatch
- ✅ **FFI Integration**: Necessary bridge between Rust engine and C game logic

**Risk Mitigation**:
- Physics stability through conservative joint constraints
- Network prediction to handle latency
- Phased implementation starting with local multiplayer

## Project Structure

### Documentation (this feature)
```
specs/011-enhance-chaosforge-multiplayer/
├── plan.md              # This file (/plan command output)
├── research.md          # Phase 0 output (/plan command)
├── data-model.md        # Phase 1 output (/plan command)
├── quickstart.md        # Phase 1 output (/plan command)
├── contracts/           # Phase 1 output (/plan command)
│   ├── ffi-interface.h  # Rust-C FFI definitions
│   ├── player-api.yaml  # Player management endpoints
│   └── physics-api.yaml # Physics system endpoints
└── tasks.md             # Phase 2 output (/tasks command - NOT created by /plan)
```

### Source Code (repository root)
```
# Hybrid Rust/C game engine structure
src/                     # Rust engine core
├── lib.rs              # FFI exports to C
├── multiplayer/        # Networking systems
├── physics/            # ODE integration
├── ragdoll/            # Multi-body character system
└── combat/             # Fighting style implementations

chaosforge-game/        # C game layer
├── main_windows.c      # Entry point, GLFW window
├── multiplayer.c       # Player management, lobby
├── combat_system.c     # Fighting style input handling
├── ffi_interface.h     # Rust FFI bindings
└── Makefile           # Build system

tests/
├── integration/        # Rust-C integration tests
├── physics/           # Ragdoll stability tests
└── multiplayer/       # Network synchronization tests
```

**Structure Decision**: Hybrid architecture (existing Option 1 + C integration layer)

## Phase 0: Outline & Research
1. **Extract unknowns from Technical Context** above:
   - NEEDS CLARIFICATION: Player matchmaking algorithm → research P2P vs client-server
   - NEEDS CLARIFICATION: Combo timing mechanics → research frame-perfect input systems
   - NEEDS CLARIFICATION: Network synchronization → research client prediction patterns
   - NEEDS CLARIFICATION: Specific physics parameters → research ODE constraint stability

2. **Generate and dispatch research agents**:
   ```
   Task: "Research multiplayer networking patterns for real-time games (P2P vs client-server)"
   Task: "Research frame-perfect combo systems and input buffering techniques"
   Task: "Research client prediction and rollback networking for physics games"
   Task: "Research ODE multi-body ragdoll stability and joint constraint values"
   Task: "Find best practices for Rust-C FFI in game engines"
   Task: "Research Bevy ECS networking plugins and multiplayer patterns"
   ```

3. **Consolidate findings** in `research.md` using format:
   - Decision: [chosen approach]
   - Rationale: [why chosen]
   - Alternatives considered: [what else evaluated]

**Output**: research.md with all NEEDS CLARIFICATION resolved

## Phase 1: Design & Contracts
*Prerequisites: research.md complete*

1. **Extract entities from feature spec** → `data-model.md`:
   - **Player**: ID, fighting style, health, lives, position, input state
   - **Ragdoll**: Multi-body components (head, torso, limbs), joint constraints
   - **Match**: Session state, arena bounds, timer, player list
   - **Fighting Style**: Ability definitions, physics parameters, force patterns
   - **Arena**: Boundary geometry, shrink state, destructible elements

2. **Generate API contracts** from functional requirements:
   - **FFI Interface**: Rust functions callable from C (spawn_player, apply_force, step_physics)
   - **Network Protocol**: Player actions, state synchronization, match events
   - **Physics API**: Ragdoll creation, force application, collision detection
   - Output schemas to `/contracts/`

3. **Generate contract tests** from contracts:
   - FFI function signature validation
   - Network message serialization/deserialization
   - Physics system determinism verification
   - Tests must fail (no implementation yet)

4. **Extract test scenarios** from user stories:
   - "4-8 players spawn with random styles" → multiplayer lobby test
   - "Collision-based combat without hitboxes" → physics integration test
   - "Arena shrinks 10% per minute" → game timer test
   - "Last survivor wins" → victory condition test

5. **Update agent file incrementally**:
   - Run `.specify/scripts/powershell/update-agent-context.ps1 -AgentType copilot`
   - Add: Rust/C hybrid development, Bevy ECS, ODE physics, GLFW windowing

## Phase 2 Preview: Task Generation Approach
*Will be executed by /tasks command*

**Task Categories**:
1. **Infrastructure**: FFI interface, build system, test framework
2. **Core Systems**: Player spawning, ragdoll creation, basic physics
3. **Fighting Styles**: Style-specific abilities and physics behaviors
4. **Multiplayer**: Networking, synchronization, matchmaking
5. **Arena Systems**: Boundary shrinking, environmental destruction
6. **Polish**: Performance optimization, UI integration, bug fixes

**Implementation Strategy**:
- Start with local multiplayer (single machine)
- Build ragdoll physics foundation
- Add fighting style mechanics
- Implement networking layer
- Integrate with existing ChaosForge UI

## Complexity Tracking
**Justified Complexity**:
- Hybrid Rust/C architecture enables leveraging existing C codebase
- Multi-body ragdolls essential for Gang Beasts-style gameplay
- Real-time networking required for multiplayer arena combat

**Risk Mitigation Plan**:
- Conservative physics parameters for stability
- Incremental implementation with local testing first
- Fallback to simpler physics if performance issues arise

## Progress Tracking
- [x] Initial Constitution Check (complexity justified)
- [ ] Phase 0: Research completed
- [ ] Phase 1: Design and contracts completed
- [ ] Post-Design Constitution Check
- [ ] Phase 2: Task generation ready

**Status**: Ready for Phase 0 research execution