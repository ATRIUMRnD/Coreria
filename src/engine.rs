use std::thread;
use std::time::{Duration, Instant};

/// Trait implemented by objects that can be processed by the [`Engine`].
///
/// Entities receive regular update ticks with the elapsed time since the
/// previous frame.  Rendering is represented by the [`render`] method which is a
/// no-op by default.
pub trait Entity {
    /// Advance the entity's state by `dt`.
    fn update(&mut self, dt: Duration);

    /// Render the entity.  The default implementation does nothing.
    fn render(&mut self) {}
}

/// Drives updates and rendering for a collection of [`Entity`] values.
pub struct Engine {
    entities: Vec<Box<dyn Entity>>,
    running: bool,
    tick_rate: f64,
}

impl Engine {
    /// Create a new engine instance with a default tick rate of 60 frames per
    /// second.
    pub fn new() -> Self {
        Self {
            entities: Vec::new(),
            running: false,
            tick_rate: 60.0,
        }
    }

    /// Set the number of frames processed per second.
    pub fn set_tick_rate(&mut self, tick_rate: f64) {
        assert!(
            tick_rate.is_finite() && tick_rate > 0.0,
            "tick rate must be positive and finite"
        );
        self.tick_rate = tick_rate;
    }

    /// Return the currently configured tick rate in frames per second.
    pub fn tick_rate(&self) -> f64 {
        self.tick_rate
    }

    /// Register `entity` with the engine.
    pub fn add_entity<E>(&mut self, entity: E)
    where
        E: Entity + 'static,
    {
        self.entities.push(Box::new(entity));
    }

    /// Advance the simulation by `dt` seconds.
    pub fn tick(&mut self, dt: Duration) {
        for entity in &mut self.entities {
            entity.update(dt);
            entity.render();
        }
    }

    /// Run the engine for the provided duration using the currently configured
    /// tick rate.
    pub fn run_for(&mut self, duration: Duration) {
        let tick_rate = self.tick_rate;
        self.run_with_rate(duration, tick_rate);
    }

    /// Run the engine for `duration` seconds using the provided `tick_rate`.
    pub fn run_with_rate(&mut self, duration: Duration, tick_rate: f64) {
        assert!(
            tick_rate.is_finite() && tick_rate > 0.0,
            "tick rate must be positive and finite"
        );
        self.tick_rate = tick_rate;
        self.running = true;

        let dt = Duration::from_secs_f64(1.0 / tick_rate);
        let start = Instant::now();

        while self.running && start.elapsed() < duration {
            let frame_start = Instant::now();
            self.tick(dt);

            if let Some(remaining) = dt.checked_sub(frame_start.elapsed()) {
                thread::sleep(remaining);
            }
        }

        self.running = false;
    }

    /// Request that the running engine exits its loop at the next opportunity.
    pub fn stop(&mut self) {
        self.running = false;
    }

    /// Returns `true` when the engine is actively ticking.
    pub fn is_running(&self) -> bool {
        self.running
    }
}

impl Default for Engine {
    fn default() -> Self {
        Self::new()
    }
}
