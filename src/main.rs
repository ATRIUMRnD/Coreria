use std::time::Duration;

use coreria::{Engine, Entity};

struct Player {
    counter: u32,
}

impl Player {
    fn new() -> Self {
        Self { counter: 0 }
    }
}

impl Entity for Player {
    fn update(&mut self, dt: Duration) {
        self.counter += 1;
        println!("tick {}: dt={:.3} s", self.counter, dt.as_secs_f32());
    }
}

fn main() {
    let mut engine = Engine::new();
    engine.add_entity(Player::new());
    engine.run_with_rate(Duration::from_millis(200), 5.0);
}
