# Coreria

Tiny Rust game engine demonstrating a minimal `Engine` and `Entity` trait.

**Constitution / Principles**
- **Rust memory safety** and zero-cost abstractions.
- **Minimal dependencies**: keep external crates to only essential runtime (winit/glutin if needed for windowing).
- **Entity-Component-System** style via an `Entity` trait for behavior.
- **60 FPS** target update/render loop.
- **Testable** tick behavior and timing controls.

## Getting Started

Prerequisites: Rust toolchain (stable) installed.

Run the demo (short, headless):

```powershell
cargo run --bin coreria-demo
```

Run tests:

```powershell
cargo test
```

## Project Structure
- `src/lib.rs` - library export of `Engine` and `Entity`.
- `src/engine.rs` - engine implementation (tick management, run_for/run_with_rate).
- `src/main.rs` - demo binary (`coreria-demo`) with a `Player` entity printing ticks.
- `tests/` - integration tests validating tick behavior and timing.

## Extending
Implement `Entity` and register with `Engine::add_entity` to add new behaviors.

Contributions welcome; keep to minimal dependencies and testable logic.
# Coreria

Coreria is a tiny starter game engine rewritten in Rust.  The crate exposes a
minimal [`Engine`] and [`Entity`] that work together to drive a basic
update/render loop.  The goal is to provide a clear, approachable entry point
for experimenting with simple simulations or games.

## Getting started

1. Ensure you have the [Rust toolchain](https://www.rust-lang.org/tools/install)
   installed.
2. Run the bundled example to see the engine in action:

   ```bash
   cargo run --bin coreria-demo
   ```

   The demo creates a trivial `Player` entity and prints the tick counter during
   execution.

3. Add your own entities by implementing the [`Entity`] trait and registering
   them with an [`Engine`].

## Running tests

Execute the test suite with:

```bash
cargo test
```

This validates both the tick behaviour and the timing controls of the engine.
