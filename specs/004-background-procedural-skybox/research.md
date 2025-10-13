# Research: ChaosForge Fighting Styles & Integration

## Unknowns & Clarifications
- What happens if the player does not select a style? [Default: Brawler]
- How does the system handle rendering errors (e.g., missing shader support)? [Fallback: simple color background]
- What if ODE physics initialization fails? [Fallback: disable physics, show error]

## Technology Choices
- **Language/Version**: C (C99 or later)
- **Graphics**: OpenGL 3.3+
- **Physics**: ODE (Open Dynamics Engine)
- **UI**: Custom GL overlay (menu)
- **Data Storage**: In-memory structs

## Best Practices
- Use structs with function pointers for modular fighting styles
- Store player and style state in memory for fast access
- Integrate all rendering in a single loop for performance
- Handle errors gracefully and provide user feedback

## Alternatives Considered
- ImGui for menu (rejected: keep dependencies minimal)
- External scripting for fighting styles (rejected: C modularity preferred)

## Decisions
- All styles (Brawler, Striker, Phantom, Titan) implemented as C structs with function pointers
- Menu overlays and style selection handled in OpenGL
- Physics via ODE, fallback to non-physics if unavailable
- State stored in memory, no external persistence
