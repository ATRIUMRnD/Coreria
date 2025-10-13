#include "multiplayer_integration.h"
#include <stdio.h>
#include <stdlib.h>

// Global state
static int multiplayer_initialized = 0;
static int current_match_id = -1;

// Initialize the multiplayer engine
int init_multiplayer_engine(void) {
    if (multiplayer_initialized) {
        return 1; // Already initialized
    }
    
    printf("[Multiplayer] Initializing Rust multiplayer engine...\n");
    
    // Initialize the Rust engine
    if (coreria_init_engine() != 0) {
        printf("[Multiplayer] Failed to initialize Rust engine\n");
        return 0;
    }
    
    multiplayer_initialized = 1;
    printf("[Multiplayer] Rust multiplayer engine initialized successfully\n");
    return 1;
}

// Cleanup the multiplayer engine
void cleanup_multiplayer_engine(void) {
    if (!multiplayer_initialized) {
        return;
    }
    
    printf("[Multiplayer] Cleaning up multiplayer engine...\n");
    coreria_cleanup_engine();
    multiplayer_initialized = 0;
}

// Update the multiplayer engine
void update_multiplayer_engine(float delta_time) {
    if (!multiplayer_initialized) {
        return;
    }
    
    // Step the simulation
    coreria_step_simulation(delta_time);
}

// Create a multiplayer match
int create_multiplayer_match(int max_players) {
    if (!multiplayer_initialized) {
        printf("[Multiplayer] Engine not initialized\n");
        return -1;
    }
    
    current_match_id = coreria_create_match(max_players);
    if (current_match_id >= 0) {
        printf("[Multiplayer] Created match %d with %d max players\n", current_match_id, max_players);
    } else {
        printf("[Multiplayer] Failed to create match\n");
    }
    
    return current_match_id;
}

// Add a player to the multiplayer match
int add_multiplayer_player(int match_id, int fighting_style) {
    if (!multiplayer_initialized) {
        printf("[Multiplayer] Engine not initialized\n");
        return -1;
    }
    
    int player_id = coreria_add_player(match_id, fighting_style);
    if (player_id >= 0) {
        printf("[Multiplayer] Added player %d with style %d to match %d\n", 
               player_id, fighting_style, match_id);
    } else {
        printf("[Multiplayer] Failed to add player to match %d\n", match_id);
    }
    
    return player_id;
}

// Get performance metrics
void get_multiplayer_performance(PerformanceMetrics* metrics) {
    if (!multiplayer_initialized || !metrics) {
        return;
    }
    
    coreria_get_performance_metrics(metrics);
}

// Sync player state to the multiplayer engine
void sync_player_state(int player_id, GamePlayerState* state) {
    if (!multiplayer_initialized || !state) {
        return;
    }
    
    // Update player position
    coreria_set_player_position(player_id, state->pos_x, state->pos_y, state->pos_z);
    
    // Update player rotation (convert to the format expected by Rust)
    // Note: You may need to adjust the coordinate system conversion
    
    // Update health if changed
    // coreria_set_player_health(player_id, state->health);
}

// Get player state from the multiplayer engine
void get_player_state(int player_id, GamePlayerState* state) {
    if (!multiplayer_initialized || !state) {
        return;
    }
    
    // Get player position
    coreria_get_player_position(player_id, &state->pos_x, &state->pos_y, &state->pos_z);
    
    // Get player health
    state->health = coreria_get_player_health(player_id);
    
    // Check if player is alive
    state->is_alive = coreria_is_player_alive(player_id);
}

// Handle F12 key press for real-time updates
void on_f12_pressed(void) {
    printf("[Multiplayer] F12 pressed - refreshing systems...\n");
    refresh_multiplayer_systems();
}

// Refresh multiplayer systems (for real-time development)
void refresh_multiplayer_systems(void) {
    if (!multiplayer_initialized) {
        return;
    }
    
    printf("[Multiplayer] Refreshing multiplayer systems...\n");
    
    // Get current performance metrics
    PerformanceMetrics metrics;
    get_multiplayer_performance(&metrics);
    
    printf("[Multiplayer] Performance: %.2f ms frame time, %d players\n", 
           metrics.frame_time_ms, metrics.player_count);
    
    // Force a simulation step to refresh state
    update_multiplayer_engine(0.016f); // 60 FPS
    
    printf("[Multiplayer] Systems refreshed\n");
}
