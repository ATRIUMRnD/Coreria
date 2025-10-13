# Tasks: ChaosForge Multiplayer Enhancement

**Input**: Design documents from `/specs/011-enhance-chaosforge-multiplayer/`
**Prerequisites**: plan.md ✓, research.md ✓, data-model.md ✓

## Execution Flow (main)
```
1. Plan loaded: Hybrid Rust-C architecture with ODE physics
2. Design documents loaded:
   → data-model.md: Rust ECS + C FFI structures
   → research.md: Network architecture + physics parameters  
   → No contracts/ directory (direct implementation)
3. Generated tasks by architecture layer:
   → Setup: Rust project, C integration, build system
   → Tests: Physics validation, network simulation, FFI safety
   → Core: ECS systems, physics engine, networking
   → Integration: C FFI layer, existing game connection
   → Polish: Performance tuning, documentation
4. Task dependencies resolved:
   → Rust core before C FFI
   → Physics before networking
   → Tests before implementation (TDD)
5. Parallel marking: Different subsystems marked [P]
6. Validation: All major systems covered
```

## Format: `[ID] [P?] Description`
- **[P]**: Can run in parallel (different subsystems, no dependencies)
- Include exact file paths based on hybrid architecture

## Phase 0: Research Resolution (COMPLETED)
- [x] R001 Network architecture decision → research.md
- [x] R002 Physics parameter research → research.md  
- [x] R003 FFI interface design → data-model.md
- [x] R004 Bevy ECS patterns → research.md

## Phase 1: Project Setup
- [ ] T001 Create Rust workspace at `D:\Coreria\chaosforge-multiplayer/`
- [ ] T002 Initialize Cargo.toml with Bevy, ODE-rs, bevy_renet dependencies
- [ ] T003 [P] Setup C FFI build system in `build.rs`
- [ ] T004 [P] Create integration with existing chaosforge-game/ via CMake
- [ ] T005 [P] Configure cross-compilation toolchain for Windows

## Phase 2: Tests First (TDD) ⚠️ MUST COMPLETE BEFORE 2.3
**CRITICAL: These tests MUST be written and MUST FAIL before ANY implementation**

### Physics Tests
- [ ] T006 [P] Ragdoll stability test in `tests/physics/test_ragdoll_stability.rs`
- [ ] T007 [P] Joint constraint test in `tests/physics/test_joint_limits.rs`
- [ ] T008 [P] Combat force application test in `tests/physics/test_combat_forces.rs`
- [ ] T009 [P] Style-specific physics test in `tests/physics/test_fighting_styles.rs`

### Networking Tests  
- [ ] T010 [P] Client-server sync test in `tests/network/test_client_prediction.rs`
- [ ] T011 [P] Input buffer rollback test in `tests/network/test_rollback_sync.rs`
- [ ] T012 [P] Packet loss resilience test in `tests/network/test_network_reliability.rs`
- [ ] T013 [P] Lag compensation test in `tests/network/test_lag_compensation.rs`

### FFI Safety Tests
- [ ] T014 [P] Memory safety test in `tests/ffi/test_memory_management.rs`
- [ ] T015 [P] C struct compatibility test in `tests/ffi/test_c_interop.rs`
- [ ] T016 [P] Null pointer handling test in `tests/ffi/test_error_handling.rs`

### Integration Tests
- [ ] T017 [P] Full match simulation test in `tests/integration/test_match_lifecycle.rs`
- [ ] T018 [P] Performance target test in `tests/integration/test_60fps_requirement.rs`

## Phase 3: Core Implementation (ONLY after tests are failing)

### ECS Foundation
- [ ] T019 [P] Player entity system in `src/entities/player.rs`
- [ ] T020 [P] Fighting style components in `src/entities/fighting_styles.rs`  
- [ ] T021 [P] Health and combat components in `src/entities/combat.rs`
- [ ] T022 [P] Input buffer component in `src/entities/input.rs`

### Physics Engine Integration
- [ ] T023 [P] ODE physics wrapper in `src/physics/ode_wrapper.rs`
- [ ] T024 [P] Ragdoll body system in `src/physics/ragdoll_system.rs`
- [ ] T025 Joint constraint system in `src/physics/joint_system.rs` (depends on T024)
- [ ] T026 Combat force system in `src/physics/combat_physics.rs` (depends on T024,T025)

### Networking Layer
- [ ] T027 [P] Network protocol definitions in `src/network/protocol.rs`
- [ ] T028 [P] Client prediction system in `src/network/client_prediction.rs`
- [ ] T029 Server authoritative system in `src/network/server_authority.rs` (depends on T028)
- [ ] T030 Input synchronization in `src/network/input_sync.rs` (depends on T028,T029)

### Game Systems
- [ ] T031 [P] Match management system in `src/game/match_manager.rs`
- [ ] T032 [P] Combat resolution system in `src/game/combat_system.rs`
- [ ] T033 Combo detection system in `src/game/combo_system.rs` (depends on T032)
- [ ] T034 Health management system in `src/game/health_system.rs` (depends on T032)

## Phase 4: C FFI Integration

### FFI Interface Layer
- [ ] T035 C-compatible types in `src/ffi/types.rs`
- [ ] T036 Player state FFI in `src/ffi/player_ffi.rs` (depends on T035)
- [ ] T037 Match state FFI in `src/ffi/match_ffi.rs` (depends on T035,T036)
- [ ] T038 Input handling FFI in `src/ffi/input_ffi.rs` (depends on T035)

### C Integration Headers
- [ ] T039 [P] FFI header generation in `include/coreria_multiplayer.h`
- [ ] T040 [P] C wrapper functions in `c_wrapper/multiplayer_wrapper.c`
- [ ] T041 Integration with main_windows.c menu system (depends on T039,T040)

## Phase 5: Performance Optimization

### Core Performance  
- [ ] T042 [P] Physics optimization for 60Hz target in `src/physics/optimizations.rs`
- [ ] T043 [P] Network bandwidth optimization in `src/network/compression.rs`
- [ ] T044 Memory allocation optimization (depends on T042,T043)

### Monitoring
- [ ] T045 [P] Performance metrics system in `src/metrics/performance.rs`
- [ ] T046 [P] Network diagnostics in `src/metrics/network_metrics.rs`
- [ ] T047 Real-time profiling integration (depends on T045,T046)

## Phase 6: Polish & Documentation

### Testing Completion
- [ ] T048 [P] Unit tests for all physics systems in `tests/unit/physics/`
- [ ] T049 [P] Unit tests for networking in `tests/unit/network/`
- [ ] T050 [P] Unit tests for FFI layer in `tests/unit/ffi/`
- [ ] T051 [P] Stress test for 8-player matches in `tests/stress/`

### Documentation  
- [ ] T052 [P] API documentation in `docs/api.md`
- [ ] T053 [P] Integration guide in `docs/integration.md`
- [ ] T054 [P] Performance tuning guide in `docs/performance.md`

### Final Integration
- [ ] T055 Build system integration with existing chaosforge-game
- [ ] T056 Cross-platform build verification 
- [ ] T057 Manual testing checklist execution

## Dependencies

### Critical Path
```
Setup (T001-T005) → Tests (T006-T018) → Core ECS (T019-T026) → 
Network (T027-T030) → Game Systems (T031-T034) → FFI (T035-T041) → 
Integration (T041) → Performance (T042-T047) → Polish (T048-T057)
```

### Parallel Groups
**Group A - Physics Tests**: T006, T007, T008, T009  
**Group B - Network Tests**: T010, T011, T012, T013  
**Group C - FFI Tests**: T014, T015, T016  
**Group D - ECS Components**: T019, T020, T021, T022  
**Group E - Physics Core**: T023, T024  
**Group F - Network Core**: T027, T028  

### Sequential Dependencies
- T025 requires T024 (joints need ragdoll bodies)
- T026 requires T024,T025 (combat needs full physics)
- T029 requires T028 (server needs client prediction model)  
- T030 requires T028,T029 (sync needs both sides)
- T033 requires T032 (combos need basic combat)
- T036 requires T035 (player FFI needs base types)
- T041 requires T039,T040 (integration needs headers + wrapper)

## Validation Checklist
*GATE: Architecture completeness verification*

- [x] All major systems have test coverage (physics, network, FFI)
- [x] All ECS entities have corresponding components
- [x] All FFI boundaries have safety tests  
- [x] Performance targets have validation tests
- [x] Integration points with existing C code defined
- [x] Cross-platform build requirements specified
- [x] Memory safety at C/Rust boundary verified

## Risk Mitigation Tasks

### High-Risk Areas
- **T024-T026**: ODE physics integration complexity
  - Mitigation: Extensive unit tests, incremental implementation
- **T028-T030**: Network synchronization accuracy  
  - Mitigation: Simulation tests, rollback validation
- **T035-T041**: C FFI memory safety
  - Mitigation: Comprehensive safety tests, clear ownership rules

### Performance Risks
- **T042**: Physics 60Hz requirement
  - Mitigation: Profiling at each step, optimization checkpoints
- **T043**: Network bandwidth for 8 players
  - Mitigation: Compression tests, bandwidth measurement

## Notes
- Each [P] task operates on different files/subsystems
- Tests must fail before implementation (TDD enforcement)
- Commit after each completed task for rollback safety  
- FFI layer requires careful memory management validation
- Performance monitoring throughout development cycle
- Integration with existing C codebase requires careful ABI management