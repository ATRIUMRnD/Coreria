#!/usr/bin/env bash
# Idempotent Cloud Agent bootstrap for the Coreria game engine.
# Installs system libraries, ensures a Rust toolchain new enough for the
# pinned dependencies (edition 2024), and builds every buildable component.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

# --- System dependencies -------------------------------------------------
# bevy (Rust engine): pkg-config, ALSA (audio), udev (gilrs/gamepads).
# C game + bevy_render link stage: OpenGL/GLU/GLUT/GLFW/X11 dev libraries.
# Software rendering + virtual display so the OpenGL game can run headless.
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  pkg-config \
  libasound2-dev libudev-dev \
  libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev libglfw3-dev \
  libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev \
  libwayland-dev libxkbcommon-dev \
  libgl1-mesa-dri mesa-utils xvfb x11-utils imagemagick

# --- Rust toolchain ------------------------------------------------------
# The chaosforge-multiplayer lockfile pins crates that require edition 2024,
# which is only available on Rust >= 1.85. Use the latest stable toolchain.
rustup toolchain install stable --profile minimal
rustup default stable

# --- Build all components ------------------------------------------------
# Root Coreria engine crate (bevy 0.13).
cargo build

# ChaosForge multiplayer engine + C FFI interface (bevy 0.14, cbindgen).
cargo build --manifest-path chaosforge-multiplayer/Cargo.toml --locked --features ffi

# Flagship ChaosForge C game (OpenGL/GLFW/GLUT).
# The repo ships prebuilt Windows/MinGW .o objects; clean first so make
# recompiles every source for Linux instead of relinking stale PE objects.
make -C chaosforge-game clean
make -C chaosforge-game

echo "Coreria environment bootstrap complete."
