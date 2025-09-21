use coreria::{Engine, Entity};
use gl::Gl;

struct Player {
    tick: u64,
}

impl Player {
    fn new() -> Self {
        Player { tick: 0 }
    }
}

impl Entity for Player {
    fn update(&mut self, _delta: f32) {
        self.tick += 1;
        println!("Player tick: {}", self.tick);
    }

    fn render(&self, gl: &Gl) {
        // Draw a simple triangle (fixed-function pipeline for simplicity)
        unsafe {
            gl.Begin(gl::TRIANGLES);
            gl.Color3f(1.0, 0.0, 0.0);
            gl.Vertex2f(0.0, 0.5);
            gl.Color3f(0.0, 1.0, 0.0);
            gl.Vertex2f(-0.5, -0.5);
            gl.Color3f(0.0, 0.0, 1.0);
            gl.Vertex2f(0.5, -0.5);
            gl.End();
        }
    }
}

fn main() {
    let mut engine = Engine::new();
    engine.add_entity(Box::new(Player::new()));
    engine.run();
}