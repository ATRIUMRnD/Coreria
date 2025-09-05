"""Example entry point that wires together the engine components."""

from __future__ import annotations

from elria import Engine, Entity


class Player(Entity):
    """Minimal example entity that counts how often it is updated."""

    def __init__(self) -> None:
        self.counter = 0

    def update(self, dt: float) -> None:  # pragma: no cover - example code
        self.counter += 1
        print(f"tick {self.counter}: dt={dt:.3f}")

    def render(self) -> None:  # pragma: no cover - example code
        pass


if __name__ == "__main__":  # pragma: no cover - manual execution
    engine = Engine()
    engine.add_entity(Player())
    engine.run(duration=0.2, tick_rate=5)
