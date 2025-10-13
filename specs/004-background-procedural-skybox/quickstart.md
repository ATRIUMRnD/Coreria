# Quickstart: ChaosForge Fighting Styles & Arena

## Prerequisites
- C compiler (gcc, clang)
- OpenGL 3.3+, ODE, FreeGLUT installed

## Build & Run
```bash
make clean && make
./chaosforge
```

## Feature Overview
- Select fighting style (Brawler, Striker, Phantom, Titan) in menu
- Arena renders background (skybox) and baseplate
- Each style uses modular C structs with function pointers
- Physics via ODE; fallback to non-physics if unavailable

## Test Scenarios
1. Start game, select each style, verify moves and stats
2. Confirm background and baseplate render
3. Check error handling for missing GL/ODE
