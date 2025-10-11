# Implementation Tasks: ChaosForge (Coreria Engine)

## Phase 2 Task Breakdown

### 1. Project Setup
- Scaffold engine and game directories in `src/`
- Set up build system (Makefile or CMake)
- Integrate OpenGL and ODE dependencies

### 2. Core Engine Features
- Implement window creation and OpenGL context
- Integrate ODE physics engine
- Set up main game loop and event handling

### 3. Arena & Environment
- Model 3D coliseum geometry
- Implement destructible elements (mesh breakage, physics triggers)
- Arena shrinking logic (boundary reduction over time)

### 4. Player System
- Player entity: position, stamina, lives, fighting style
- Random fighting style assignment (Brawler, Striker, Phantom, Titan)
- Implement movement, sprint (speed boost, cooldown, stamina)
- Implement block (damage reduction, stamina cost, guard break interaction)

### 5. Combat Mechanics
- Define moves for each fighting style (combo extenders, guard break, ultimate)
- Implement input handling for attacks and combos
- Combo chaining and guaranteed follow-ups
- Guard break logic (block shatter)
- Ultimate activation (after 3 combos)

### 6. Physics & Ragdoll
- Physics-based movement and collisions
- Ragdoll death system (ODE integration)
- Handle edge cases (glitches, error recovery)

### 7. Match Flow & Multiplayer
- Match state: player tracking, lives, elimination
- No login; instant join for prototype
- Handle player disconnects and simultaneous eliminations
- Declare last survivor as winner

### 8. UI & Controls
- Basic HUD: lives, stamina, fighting style
- Controls: WASD, Shift (sprint), Ctrl (block), mouse (attacks)

### 9. Testing & Debugging
- Manual playtesting and bug tracking
- Logging for errors and game events
- Integration tests for engine subsystems

### 10. Documentation
- Update `quickstart.md` and code comments
- Document engine/game architecture

---

## Progress Tracking
- [ ] Project scaffold complete
- [ ] Engine features implemented
- [ ] Arena and environment ready
- [ ] Player system functional
- [ ] Combat mechanics working
- [ ] Physics and ragdoll deaths integrated
- [ ] Match flow and multiplayer logic
- [ ] UI and controls implemented
- [ ] Testing and debugging done
- [ ] Documentation updated
