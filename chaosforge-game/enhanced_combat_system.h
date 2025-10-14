// Enhanced Unified Combat System - Connects all codebase components
// File: chaosforge-game/enhanced_combat_system.h

#ifndef ENHANCED_COMBAT_SYSTEM_H
#define ENHANCED_COMBAT_SYSTEM_H

#include "game_state.h"
#include "gangbeast_avatar.h"
#include "multiplayer_integration.h"
#include <stdio.h>
#include <time.h>

// Combat animation timing constants
#define LIGHT_ATTACK_FRAMES 30
#define HEAVY_ATTACK_FRAMES 45
#define BLOCK_FRAMES 20
#define COMBO_WINDOW_FRAMES 15
#define STUN_FRAMES 30

// Performance monitoring
typedef struct {
    int frame_count;
    float total_frame_time;
    float avg_fps;
    int combat_events;
    int animation_updates;
    float physics_sync_time;
    time_t start_time;
} PerformanceMetrics;

// Enhanced combat state
typedef struct {
    // Core combat data
    int attack_animation;
    int block_animation;
    int stun_timer;
    int combo_count;
    int combo_meter;
    
    // Animation integration
    GangbeastAvatar* avatar;
    AvatarAnimation current_anim;
    float animation_time;
    float animation_speed_multiplier;
    
    // Physics integration
    int ragdoll_active;
    void* ragdoll_handle;
    float last_physics_sync;
    
    // Performance tracking
    float last_attack_time;
    int attacks_per_second;
    int successful_hits;
    int blocked_attacks;
    
} EnhancedCombatState;

// Enhanced player state that unifies all systems
typedef struct {
    // Basic properties
    int id;
    int style;
    int lives;
    int health;
    int max_health;
    int stamina;
    int max_stamina;
    
    // Position and movement
    float x, y, z;
    float vel_x, vel_y, vel_z;
    float facing_angle;
    
    // Enhanced combat system
    EnhancedCombatState combat;
    
    // Input tracking
    int keys_pressed[256];
    int mouse_buttons[3];
    float mouse_x, mouse_y;
    float last_input_time;
    
} EnhancedPlayer;

// Global enhanced game state
typedef struct {
    EnhancedPlayer players[MAX_PLAYERS];
    int num_players;
    int game_tick;
    int frame_count;
    
    // System integration flags
    int avatar_system_enabled;
    int physics_system_enabled;
    int multiplayer_system_enabled;
    int performance_monitoring_enabled;
    
    // Performance metrics
    PerformanceMetrics performance;
    
    // Logging system
    char log_buffer[1024];
    FILE* combat_log_file;
    FILE* performance_log_file;
    
} EnhancedGameState;

// Function declarations
void enhanced_combat_init(EnhancedGameState* state);
void enhanced_combat_cleanup(EnhancedGameState* state);
void enhanced_combat_update(EnhancedGameState* state, float delta_time);
void enhanced_combat_handle_input(EnhancedGameState* state, int player_id, int input_type, float param1, float param2);
void enhanced_combat_sync_physics(EnhancedGameState* state);
void enhanced_combat_update_animations(EnhancedGameState* state, float delta_time);
void enhanced_combat_check_collisions(EnhancedGameState* state);
void enhanced_combat_log_event(EnhancedGameState* state, const char* format, ...);
void enhanced_combat_update_performance(EnhancedGameState* state, float frame_time);
void enhanced_combat_export_metrics(EnhancedGameState* state);

// Combat action types (matching Rust FFI)
#define ENHANCED_ACTION_NONE 0
#define ENHANCED_ACTION_MOVE_FORWARD 1
#define ENHANCED_ACTION_MOVE_BACKWARD 2
#define ENHANCED_ACTION_MOVE_LEFT 3
#define ENHANCED_ACTION_MOVE_RIGHT 4
#define ENHANCED_ACTION_JUMP 5
#define ENHANCED_ACTION_CROUCH 6
#define ENHANCED_ACTION_PUNCH 7
#define ENHANCED_ACTION_KICK 8
#define ENHANCED_ACTION_BLOCK 9
#define ENHANCED_ACTION_GRAB 10

// Animation integration helpers
void sync_avatar_with_combat(EnhancedPlayer* player);
void trigger_combat_animation(EnhancedPlayer* player, int animation_type);
void update_ragdoll_forces(EnhancedPlayer* player);

// Performance monitoring helpers
void start_performance_timer(const char* operation);
void end_performance_timer(EnhancedGameState* state, const char* operation);

#endif // ENHANCED_COMBAT_SYSTEM_H