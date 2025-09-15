use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;
use std::time::{Duration, Instant};

use coreria::{Engine, Entity};

struct Dummy {
    updates: Arc<AtomicUsize>,
}

impl Dummy {
    fn new(updates: Arc<AtomicUsize>) -> Self {
        Self { updates }
    }
}

impl Entity for Dummy {
    fn update(&mut self, _dt: Duration) {
        self.updates.fetch_add(1, Ordering::SeqCst);
    }

    fn render(&mut self) {}
}

#[test]
fn tick_updates_entities() {
    let counter = Arc::new(AtomicUsize::new(0));
    let dummy = Dummy::new(counter.clone());
    let mut engine = Engine::new();
    engine.add_entity(dummy);

    engine.tick(Duration::from_millis(16));

    assert_eq!(counter.load(Ordering::SeqCst), 1);
}

#[test]
fn run_advances_time_quickly() {
    let counter = Arc::new(AtomicUsize::new(0));
    let dummy = Dummy::new(counter.clone());
    let mut engine = Engine::new();
    engine.add_entity(dummy);

    let duration = Duration::from_millis(50);
    let tick_rate = 20.0;

    let start = Instant::now();
    engine.run_with_rate(duration, tick_rate);
    let elapsed = start.elapsed();

    assert!(counter.load(Ordering::SeqCst) > 0);
    assert!(elapsed >= duration);
}
