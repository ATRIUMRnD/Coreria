# Research: ChaosForge Implementation

## Engine Selection
- Use Coreria game engine (C-based)
- Rendering: OpenGL
- Physics: ODE (Open Dynamics Engine)

## Multiplayer Architecture
- Peer-to-peer or client-server for 4-8 players
- No login, instant join for prototype

## Arena Design
- 3D coliseum, destructible elements
- Shrinking boundaries (mechanics TBD)

## Fighting Styles
- Brawler, Striker, Phantom, Titan
- Each style: 2 combo extenders, 1 guard break, 1 ultimate

## Physics & Gameplay
- True physics for momentum, collisions, ragdoll deaths
- Stamina system for sprint/block

## Open Questions
- Exact destructible mechanics
- Arena shrink triggers/rate
- Physics engine configuration
- Handling disconnects, edge cases
