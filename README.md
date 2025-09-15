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
