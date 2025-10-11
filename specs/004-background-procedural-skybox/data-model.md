# Data Model: ChaosForge Fighting Styles & Arena

## Entities

### FightingStyle
- name: string
- mass: float
- gravity: float
- move_set: function pointers (combo, guard break, ultimate)
- stats: struct (health, speed, meter)

### Player
- style: FightingStyle
- position: (x, y, z)
- velocity: (vx, vy, vz)
- health: int
- lives: int
- meter: int

### Arena
- background: shader params
- baseplate: mesh size, collider
- menu: UI state (selected style, start button)

## Relationships
- Player has one FightingStyle
- Arena contains baseplate and background
- Menu interacts with Player for style selection
