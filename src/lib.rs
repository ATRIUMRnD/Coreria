//! Coreria - a tiny starter game engine implemented in Rust.
//!
//! The library exposes a minimal [`Engine`] and [`Entity`] that can be
//! composed to drive basic simulations or games.

use winit::{event_loop::EventLoop, window::Window};
use glutin::ContextWrapper;
use instant::Instant;
use gl::Gl;

/// Trait for game entities: update logic and render.
pub trait Entity {
	fn update(&mut self, delta: f32);
	fn render(&self, gl: &Gl);
}

/// The main engine struct: manages entities and the game loop.
pub struct Engine {
	pub entities: Vec<Box<dyn Entity>>,
	pub event_loop: Option<EventLoop<()>>,
	pub window: Option<Window>,
	pub gl_context: Option<ContextWrapper<glutin::PossiblyCurrent, Window>>,
	pub gl: Option<Gl>,
	pub accumulator: f64,
	pub tick: u64,
	pub last_time: Instant,
}

impl Engine {
	pub fn new() -> Self {
		Engine {
			entities: Vec::new(),
			event_loop: None,
			window: None,
			gl_context: None,
			gl: None,
			accumulator: 0.0,
			tick: 0,
			last_time: Instant::now(),
		}
	}

	pub fn add_entity(&mut self, entity: Box<dyn Entity>) {
		self.entities.push(entity);
	}
	// Loop and rendering logic will be implemented in the next steps.
}
