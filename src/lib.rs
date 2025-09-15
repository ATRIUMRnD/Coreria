//! Coreria - a tiny starter game engine implemented in Rust.
//!
//! The library exposes a minimal [`Engine`] and [`Entity`] that can be
//! composed to drive basic simulations or games.

mod engine;

pub use crate::engine::{Engine, Entity};
