use std::time::Duration;

use coreria::{Engine, Entity};

/// A simple player entity that counts ticks and prints on update.
struct Player {
    ticks: u32,
}

impl Player {
    fn new() -> Self {
        Self { ticks: 0 }
    }
}

impl Entity for Player {
    fn update(&mut self, _dt: Duration) {
        self.ticks += 1;
        println!("Player tick: {}", self.ticks);
    }

    fn render(&mut self) {}
}

fn main() {
    // Run a short, deterministic demo: 60 FPS for 1 second (60 ticks).
    let mut engine = Engine::new();
    engine.set_tick_rate(60.0);
    engine.add_entity(Player::new());

    engine.run_with_rate(Duration::from_secs(1), 60.0);
}
