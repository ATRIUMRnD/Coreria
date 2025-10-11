# Quickstart: ChaosForge Prototype

## Prerequisites
- C compiler (gcc/clang)
- OpenGL development libraries
- ODE (Open Dynamics Engine) library

## Build Steps
1. Clone the repository
2. Install dependencies (OpenGL, ODE)
3. Build the engine and game logic
   - `cd src/engine && make`
   - `cd ../game && make`
4. Run the game
   - `./chaosforge`

## Controls
- Movement: WASD
- Sprint: Shift (50% speed boost, 2s cooldown, stamina-based)
- Block: Ctrl (75% damage reduction, stamina cost)
- Combos/Attacks: Mouse buttons

## Notes
- No login required
- 4-8 players per match
- Arena shrinks over time
- Physics-based gameplay
