/*!
# Coreria Game Engine

A custom hybrid Rust/C game engine for physics-based combat games.
Built with Bevy ECS, OpenGL rendering, and ODE physics integration.
*/

use bevy::prelude::*;
use log::{info, error};
use std::env;

mod engine;
mod graphics;
mod physics;
mod input;
mod ffi;

use engine::CoreiaEngine;

fn main() {
    // Initialize logging
    env_logger::init();
    
    let args: Vec<String> = env::args().collect();
    
    info!("🎮 Starting Coreria Game Engine v{}", env!("CARGO_PKG_VERSION"));
    info!("🔧 Built with Rust {} and Bevy {}", 
          env!("RUSTC_VERSION"), 
          "0.13"); // Bevy version from Cargo.toml
    
    // Parse command line arguments
    let mode = if args.len() > 1 {
        args[1].as_str()
    } else {
        "engine"
    };
    
    match mode {
        "engine" => {
            info!("🚀 Launching Coreria Engine in standalone mode");
            run_engine();
        },
        "chaosforge" => {
            info!("⚔️  Launching ChaosForge game");
            run_chaosforge();
        },
        "headless" => {
            info!("🤖 Running in headless server mode");
            run_headless_server();
        },
        "--help" | "-h" => {
            print_help();
        },
        _ => {
            error!("❌ Unknown mode: {}", mode);
            print_help();
            std::process::exit(1);
        }
    }
}

fn run_engine() {
    let mut engine = CoreiaEngine::new();
    
    if let Err(e) = engine.initialize() {
        error!("Failed to initialize engine: {}", e);
        std::process::exit(1);
    }
    
    info!("✅ Engine initialized successfully");
    engine.run();
}

#[cfg(feature = "multiplayer")]
fn run_chaosforge() {
    use chaosforge_multiplayer::MultiplayerEngine;
    
    info!("🏟️  Initializing ChaosForge arena...");
    
    let mut mp_engine = MultiplayerEngine::new();
    
    // Create a default player for testing
    let player_id = mp_engine.create_player();
    info!("👤 Created test player with ID: {}", player_id);
    
    // Main game loop
    let mut running = true;
    let target_fps = 60.0;
    let frame_time = 1.0 / target_fps;
    
    info!("🎮 ChaosForge is running! Press Ctrl+C to exit...");
    
    while running {
        let start_time = std::time::Instant::now();
        
        // Step simulation
        mp_engine.step_simulation(frame_time);
        
        // Check for exit conditions (simplified)
        // In a real implementation, this would handle window events
        if start_time.elapsed().as_secs() > 10 {
            info!("🏁 Demo completed after 10 seconds");
            running = false;
        }
        
        // Frame rate limiting
        let elapsed = start_time.elapsed().as_secs_f32();
        if elapsed < frame_time {
            std::thread::sleep(std::time::Duration::from_secs_f32(frame_time - elapsed));
        }
    }
    
    info!("👋 ChaosForge session ended");
}

#[cfg(not(feature = "multiplayer"))]
fn run_chaosforge() {
    error!("❌ ChaosForge requires the 'multiplayer' feature to be enabled");
    error!("💡 Try: cargo run --features multiplayer chaosforge");
    std::process::exit(1);
}

fn run_headless_server() {
    info!("🌐 Starting headless server...");
    // Placeholder for dedicated server implementation
    info!("⚠️  Headless server not yet implemented");
    
    // Keep server alive
    loop {
        std::thread::sleep(std::time::Duration::from_secs(1));
        // In real implementation, handle network connections
    }
}

fn print_help() {
    println!(r#"
🎮 Coreria Game Engine v{}

USAGE:
    coreria [MODE]

MODES:
    engine      Launch the core engine (default)
    chaosforge  Launch ChaosForge multiplayer game
    headless    Run as dedicated server
    --help, -h  Show this help message

EXAMPLES:
    coreria                    # Run engine
    coreria chaosforge         # Play ChaosForge
    coreria headless           # Dedicated server

FEATURES:
    • Hybrid Rust/C architecture
    • Bevy ECS system integration  
    • OpenGL graphics rendering
    • ODE physics simulation
    • Multiplayer networking
    • Spec-driven development

For more information, visit:
https://github.com/CGXDevTeam/Coreria
"#, env!("CARGO_PKG_VERSION"));
}
