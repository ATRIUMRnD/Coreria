import time

from elria import Engine, Entity


class Dummy(Entity):
    def __init__(self) -> None:
        self.updated = 0

    def update(self, dt: float) -> None:
        self.updated += 1

    def render(self) -> None:
        pass


def test_tick_updates_entities():
    engine = Engine()
    dummy = Dummy()
    engine.add_entity(dummy)

    engine.tick(0.1)

    assert dummy.updated == 1


def test_run_advances_time_quickly():
    engine = Engine()
    dummy = Dummy()
    engine.add_entity(dummy)

    start = time.time()
    engine.run(duration=0.05, tick_rate=20)
    elapsed = time.time() - start

    assert dummy.updated > 0
    assert elapsed >= 0.05
