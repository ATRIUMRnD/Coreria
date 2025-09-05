"""A tiny but functional game engine skeleton.

The goal is to provide a cohesive entry point that ties together the
update and render loops.  It is intentionally minimal but demonstrates
how entities can be managed by the engine.
"""

from __future__ import annotations

import time
from typing import List


class Entity:
    """Basic object that can be processed by the :class:`Engine`.

    Sub‑classes are expected to override :meth:`update` and optionally
    :meth:`render` to provide game specific behaviour.
    """

    def update(self, dt: float) -> None:  # pragma: no cover - default no-op
        """Advance the entity's state.

        Parameters
        ----------
        dt:
            The elapsed time since the previous tick, in seconds.
        """

    def render(self) -> None:  # pragma: no cover - default no-op
        """Render the entity.  For this starter engine we simply provide a
        placeholder method that can be extended by consumers."""


class Engine:
    """Drive updates and rendering for a collection of entities."""

    def __init__(self) -> None:
        self._entities: List[Entity] = []
        self._running: bool = False

    # ------------------------------------------------------------------
    def add_entity(self, entity: Entity) -> None:
        """Register *entity* with the engine."""

        self._entities.append(entity)

    # ------------------------------------------------------------------
    def tick(self, dt: float) -> None:
        """Advance the simulation by *dt* seconds."""

        for entity in list(self._entities):
            entity.update(dt)
            entity.render()

    # ------------------------------------------------------------------
    def run(self, duration: float = 1.0, tick_rate: float = 60.0) -> None:
        """Run the engine for ``duration`` seconds.

        The engine repeatedly calls :meth:`tick` at ``tick_rate`` frames per
        second, sleeping in-between calls.  The method returns once the
        requested duration has elapsed.
        """

        self._running = True
        dt = 1.0 / tick_rate
        elapsed = 0.0

        while self._running and elapsed < duration:
            start = time.time()
            self.tick(dt)
            sleep_time = dt - (time.time() - start)
            if sleep_time > 0:
                time.sleep(sleep_time)
            elapsed += dt

        self._running = False

    # ------------------------------------------------------------------
    def stop(self) -> None:
        """Request that the running engine exits its loop."""

        self._running = False
