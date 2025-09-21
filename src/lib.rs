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

	/// Run the engine: create window, OpenGL context, and start the event loop.
	pub fn run(mut self) {
		let event_loop = EventLoop::new();
		let window = winit::window::WindowBuilder::new()
			.with_title("Coreria Demo")
			.build(&event_loop)
			.expect("Failed to create window");
		let gl_context = unsafe {
			glutin::ContextBuilder::new()
				.build_windowed(window.clone(), &event_loop)
				.expect("Failed to create GL context")
				.make_current()
				.expect("Failed to activate GL context")
		};
		let gl = unsafe { Gl::load_with(|s| gl_context.get_proc_address(s) as *const _) };

		unsafe {
			gl.ClearColor(0.1, 0.1, 0.1, 1.0);
		}

		self.event_loop = Some(event_loop);
		self.window = Some(window);
		self.gl_context = Some(gl_context);
		self.gl = Some(gl);
		self.last_time = Instant::now();

		let mut accumulator = 0.0;
		let mut last_time = Instant::now();
		let mut tick = 0u64;
		let gl = self.gl.as_ref().unwrap().clone();
		let gl_context = self.gl_context.as_ref().unwrap();
		let window = self.window.as_ref().unwrap();
		let mut entities = self.entities;

		let event_loop = self.event_loop.take().unwrap();
		event_loop.run(move |event, _, control_flow| {
			use winit::event::{Event, WindowEvent, KeyboardInput, VirtualKeyCode};
			use winit::event_loop::ControlFlow;
			*control_flow = ControlFlow::Wait;

			match event {
				Event::WindowEvent { event: WindowEvent::CloseRequested, .. } => *control_flow = ControlFlow::Exit,
				Event::WindowEvent { event: WindowEvent::KeyboardInput { input, .. }, .. } => {
					if let Some(VirtualKeyCode::Escape) = input.virtual_keycode {
						*control_flow = ControlFlow::Exit;
					}
				}
				Event::RedrawRequested(_) => {
					unsafe {
						gl.Clear(gl::COLOR_BUFFER_BIT);
					}
					for entity in &entities {
						entity.render(&gl);
					}
					gl_context.swap_buffers().unwrap();
				}
				Event::MainEventsCleared => {
					window.request_redraw();
					let now = Instant::now();
					let delta = now.duration_since(last_time).as_secs_f64();
					last_time = now;
					accumulator += delta;
					let dt = 1.0 / 60.0;
					while accumulator >= dt {
						for entity in &mut entities {
							entity.update(dt as f32);
						}
						accumulator -= dt;
						tick += 1;
					}
				}
				_ => (),
			}
		});
	}
}
