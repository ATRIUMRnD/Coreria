"""Core package for the Epoch of Elria game engine.

This package exposes the :class:`Engine` and :class:`Entity` classes
for building simple games or simulations.  The implementation is kept
intentionally small so it can serve as a starting point for a more
feature rich engine.
"""

from .engine import Engine, Entity

__all__ = ["Engine", "Entity"]
