// Enhanced Unified Combat System Implementation
// File: chaosforge-game/enhanced_combat_system.c

#include "enhanced_combat_system.h"
#include "combat_system.h"
#include "physics_manager.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>

// Global enhanced game state
static EnhancedGameState* g_enhanced_state = NULL;
static clock_t g_timer_start = 0;

// Initialize the enhanced combat system
void enhanced_combat_init(EnhancedGameState* state) {
    if (!state) return;
    
    g_enhanced_state = state;
    memset(state, 0, sizeof(EnhancedGameState));
    
    // Initialize performance monitoring
    state->performance_monitoring_enabled = 1;
    state->performance.start_time = time(NULL);
    
    // Initialize logging
    state->combat_log_file = fopen("combat_log.txt", "w");
    state->performance_log_file = fopen("performance_log.txt", "w");
    
    if (state->combat_log_file) {
        fprintf(state->combat_log_file, "Enhanced Combat System Log - Started\n");
        fflush(state->combat_log_file);
    }
    
    // Check system availability
    state->avatar_system_enabled = 1;  // Avatar system available
    
    // Try to initialize multiplayer engine
    if (init_multiplayer_engine && init_multiplayer_engine() == 0) {
        state->multiplayer_system_enabled = 1;
        state->physics_system_enabled = 1;
        enhanced_combat_log_event(state, "Multiplayer engine initialized successfully");
    } else {
        enhanced_combat_log_event(state, "Warning: Multiplayer engine not available");
    }
    
    // Initialize players
    for (int i = 0; i < MAX_PLAYERS; i++) {
        EnhancedPlayer* player = &state->players[i];
        player->id = i;
        player->max_health = 100;
        player->health = player->max_health;
        player->max_stamina = 100;
        player->stamina = player->max_stamina;
        player->lives = 3;
        
        // Initialize combat state
        player->combat.avatar = NULL;
        player->combat.current_anim = ANIM_IDLE;
        player->combat.animation_speed_multiplier = 1.0f;
        
        // Create avatar if system is available
        if (state->avatar_system_enabled) {
            switch (i % 4) {
                case 0: player->combat.avatar = gangbeast_avatar_create_preset_brawler(); break;
                case 1: player->combat.avatar = gangbeast_avatar_create_preset_striker(); break;
                case 2: player->combat.avatar = gangbeast_avatar_create_preset_phantom(); break;
                case 3: player->combat.avatar = gangbeast_avatar_create_preset_titan(); break;
            }
            player->style = i % 4;
        }
        
        // Initialize ragdoll if physics is available
        if (state->physics_system_enabled && coreria_spawn_ragdoll) {
            RagdollHandle ragdoll = coreria_spawn_ragdoll(i, player->style, 
                                                         player->x, player->y, player->z);
            if (RAGDOLL_IS_VALID(ragdoll)) {
                player->combat.ragdoll_handle = (void*)(uintptr_t)ragdoll.id;
                player->combat.ragdoll_active = 1;
                enhanced_combat_log_event(state, "Player %d ragdoll initialized (ID: %u)", i, ragdoll.id);
            }
        }
    }
    
    state->num_players = 1; // Start with 1 local player
    
    enhanced_combat_log_event(state, "Enhanced combat system initialized");
    printf("[ENHANCED] Combat system initialized with %d systems enabled\n", 
           state->avatar_system_enabled + state->physics_system_enabled + state->multiplayer_system_enabled);
}

// Cleanup the enhanced combat system
void enhanced_combat_cleanup(EnhancedGameState* state) {
    if (!state) return;
    
    enhanced_combat_log_event(state, "Enhanced combat system shutting down");
    
    // Clean up avatars
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (state->players[i].combat.avatar) {
            gangbeast_avatar_destroy(state->players[i].combat.avatar);
            state->players[i].combat.avatar = NULL;
        }
        
        // Clean up ragdolls
        if (state->players[i].combat.ragdoll_active && coreria_destroy_ragdoll) {
            RagdollHandle ragdoll = {(unsigned int)(uintptr_t)state->players[i].combat.ragdoll_handle};
            coreria_destroy_ragdoll(ragdoll);
        }
    }
    
    // Export final metrics
    enhanced_combat_export_metrics(state);
    
    // Close log files
    if (state->combat_log_file) {
        fprintf(state->combat_log_file, "Enhanced combat system shutdown complete\n");
        fclose(state->combat_log_file);
        state->combat_log_file = NULL;
    }
    
    if (state->performance_log_file) {
        fclose(state->performance_log_file);
        state->performance_log_file = NULL;
    }
    
    // Shutdown multiplayer engine
    if (state->multiplayer_system_enabled && coreria_shutdown_engine) {
        coreria_shutdown_engine();
    }
    
    printf("[ENHANCED] Combat system cleanup complete\n");
}

// Main update function that coordinates all systems
void enhanced_combat_update(EnhancedGameState* state, float delta_time) {
    if (!state) return;
    
    start_performance_timer("total_update");
    
    state->game_tick++;
    state->frame_count++;
    
    // Update each player
    for (int i = 0; i < state->num_players; i++) {
        EnhancedPlayer* player = &state->players[i];
        
        // Update combat timers
        if (player->combat.attack_animation > 0) {
            player->combat.attack_animation--;
            if (player->combat.attack_animation == 0) {
                // Attack animation finished, return to idle
                if (player->combat.avatar) {
                    gangbeast_avatar_set_animation(player->combat.avatar, ANIM_IDLE);
                    player->combat.current_anim = ANIM_IDLE;
                }
                enhanced_combat_log_event(state, "Player %d attack animation finished", i);
            }
        }
        
        if (player->combat.block_animation > 0) {
            player->combat.block_animation--;
        }
        
        if (player->combat.stun_timer > 0) {
            player->combat.stun_timer--;
        }
        
        // Decay combo meter
        if (player->combat.combo_meter > 0) {
            player->combat.combo_meter--;
        }
        
        // Regenerate stamina
        if (player->stamina < player->max_stamina) {
            player->stamina++;
        }
        
        // Update avatar animation
        if (player->combat.avatar) {
            gangbeast_avatar_update_animation(player->combat.avatar, delta_time);
        }
    }
    
    // Update physics integration
    enhanced_combat_sync_physics(state);
    
    // Check collisions and combat interactions
    enhanced_combat_check_collisions(state);
    
    // Update animations
    enhanced_combat_update_animations(state, delta_time);
    
    // Update performance metrics
    enhanced_combat_update_performance(state, delta_time);
    
    end_performance_timer(state, "total_update");
    
    // Log periodic updates
    if (state->game_tick % 600 == 0) { // Every 10 seconds at 60fps
        enhanced_combat_log_event(state, "System status - Tick: %d, Players: %d, FPS: %.1f", 
                                 state->game_tick, state->num_players, state->performance.avg_fps);
    }
}

// Handle input from various sources
void enhanced_combat_handle_input(EnhancedGameState* state, int player_id, int input_type, float param1, float param2) {
    if (!state || player_id >= state->num_players) return;
    
    EnhancedPlayer* player = &state->players[player_id];
    player->last_input_time = (float)state->game_tick;
    
    switch (input_type) {
        case ENHANCED_ACTION_PUNCH:
            if (player->stamina >= 15 && player->combat.attack_animation == 0) {
                player->combat.attack_animation = LIGHT_ATTACK_FRAMES;
                player->stamina -= 15;
                player->combat.combo_meter += 10;
                player->combat.attacks_per_second++;
                
                // Trigger animation
                trigger_combat_animation(player, ENHANCED_ACTION_PUNCH);
                
                // Send to multiplayer system
                if (state->multiplayer_system_enabled && coreria_process_mouse_input) {
                    coreria_process_mouse_input(player_id, 1, 1, param1, param2);
                }
                
                enhanced_combat_log_event(state, "Player %d light punch - Stamina: %d, Combo: %d", 
                                         player_id, player->stamina, player->combat.combo_meter);
                state->performance.combat_events++;
            }
            break;
            
        case ENHANCED_ACTION_KICK:
            if (player->stamina >= 25 && player->combat.attack_animation == 0) {
                player->combat.attack_animation = HEAVY_ATTACK_FRAMES;
                player->stamina -= 25;
                player->combat.combo_meter += 20;
                player->combat.attacks_per_second++;
                
                // Trigger animation
                trigger_combat_animation(player, ENHANCED_ACTION_KICK);
                
                // Send to multiplayer system
                if (state->multiplayer_system_enabled && coreria_process_mouse_input) {
                    coreria_process_mouse_input(player_id, 2, 1, param1, param2);
                }
                
                enhanced_combat_log_event(state, "Player %d heavy kick - Stamina: %d, Combo: %d", 
                                         player_id, player->stamina, player->combat.combo_meter);
                state->performance.combat_events++;
            }
            break;
            
        case ENHANCED_ACTION_BLOCK:
            if (player->stamina >= 5) {
                player->combat.block_animation = BLOCK_FRAMES;
                player->stamina -= 1;
                
                enhanced_combat_log_event(state, "Player %d blocking - Stamina: %d", player_id, player->stamina);
            }
            break;
            
        case ENHANCED_ACTION_MOVE_FORWARD:
        case ENHANCED_ACTION_MOVE_BACKWARD:
        case ENHANCED_ACTION_MOVE_LEFT:
        case ENHANCED_ACTION_MOVE_RIGHT:
            // Handle movement
            if (state->multiplayer_system_enabled && coreria_set_movement_input) {
                float forward = (input_type == ENHANCED_ACTION_MOVE_FORWARD) ? 1.0f : 
                               (input_type == ENHANCED_ACTION_MOVE_BACKWARD) ? -1.0f : 0.0f;
                float right = (input_type == ENHANCED_ACTION_MOVE_RIGHT) ? 1.0f : 
                             (input_type == ENHANCED_ACTION_MOVE_LEFT) ? -1.0f : 0.0f;
                coreria_set_movement_input(player_id, forward, right);
            }
            
            // Set walking animation
            if (player->combat.avatar && player->combat.current_anim == ANIM_IDLE) {
                gangbeast_avatar_set_animation(player->combat.avatar, ANIM_WALKING);
                player->combat.current_anim = ANIM_WALKING;
            }
            break;
    }
}

// Synchronize physics systems
void enhanced_combat_sync_physics(EnhancedGameState* state) {
    if (!state || !state->physics_system_enabled) return;
    
    start_performance_timer("physics_sync");
    
    // Step the multiplayer physics simulation
    if (coreria_step_simulation) {
        float delta_time = 1.0f / 60.0f; // Assume 60 FPS
        coreria_step_simulation(delta_time);
    }
    
    // Sync ragdoll positions with player positions
    for (int i = 0; i < state->num_players; i++) {
        EnhancedPlayer* player = &state->players[i];
        
        if (player->combat.ragdoll_active && coreria_get_ragdoll_position) {
            RagdollHandle ragdoll = {(unsigned int)(uintptr_t)player->combat.ragdoll_handle};
            float ragdoll_x, ragdoll_y, ragdoll_z, ragdoll_angle;
            
            if (coreria_get_ragdoll_position(ragdoll, &ragdoll_x, &ragdoll_y, &ragdoll_z, &ragdoll_angle) == 0) {
                // Update player position based on ragdoll
                player->x = ragdoll_x;
                player->y = ragdoll_y;
                player->z = ragdoll_z;
                player->facing_angle = ragdoll_angle;
            }
        }
        
        // Apply combat forces during attacks
        update_ragdoll_forces(player);
    }
    
    end_performance_timer(state, "physics_sync");
}

// Update all animation systems
void enhanced_combat_update_animations(EnhancedGameState* state, float delta_time) {
    if (!state) return;
    
    start_performance_timer("animations");
    
    for (int i = 0; i < state->num_players; i++) {
        EnhancedPlayer* player = &state->players[i];
        
        if (player->combat.avatar) {
            // Update avatar animation timing
            player->combat.animation_time += delta_time * player->combat.animation_speed_multiplier;
            
            // Sync avatar with combat state
            sync_avatar_with_combat(player);
            
            state->performance.animation_updates++;
        }
    }
    
    end_performance_timer(state, "animations");
}

// Check for combat collisions and interactions
void enhanced_combat_check_collisions(EnhancedGameState* state) {
    if (!state) return;
    
    start_performance_timer("collisions");
    
    for (int i = 0; i < state->num_players; i++) {
        for (int j = i + 1; j < state->num_players; j++) {
            EnhancedPlayer* p1 = &state->players[i];
            EnhancedPlayer* p2 = &state->players[j];
            
            float dx = p1->x - p2->x;
            float dz = p1->z - p2->z;
            float distance = sqrtf(dx*dx + dz*dz);
            
            // Combat range check
            if (distance < 2.5f) {
                // Check if p1 is attacking p2
                if (p1->combat.attack_animation > 20 && p2->combat.block_animation == 0) {
                    int damage = (p1->combat.attack_animation > 35) ? 30 : 20; // Heavy vs light
                    p2->health -= damage;
                    p2->combat.stun_timer = STUN_FRAMES;
                    p1->combat.successful_hits++;
                    
                    enhanced_combat_log_event(state, "COMBAT: Player %d hits Player %d for %d damage! Health: %d", 
                                             i, j, damage, p2->health);
                    
                    if (p2->health <= 0) {
                        p2->lives--;
                        if (p2->lives > 0) {
                            p2->health = p2->max_health;
                            p2->combat.respawn_animation = 60;
                            enhanced_combat_log_event(state, "Player %d respawns! Lives remaining: %d", j, p2->lives);
                        } else {
                            enhanced_combat_log_event(state, "Player %d eliminated!", j);
                        }
                    }
                } else if (p1->combat.attack_animation > 20 && p2->combat.block_animation > 0) {
                    // Attack was blocked
                    p2->combat.blocked_attacks++;
                    enhanced_combat_log_event(state, "Player %d blocked Player %d's attack!", j, i);
                }
            }
        }
    }
    
    end_performance_timer(state, "collisions");
}

// Enhanced logging with timestamp and context
void enhanced_combat_log_event(EnhancedGameState* state, const char* format, ...) {
    if (!state || !state->combat_log_file) return;
    
    va_list args;
    va_start(args, format);
    
    // Get current time
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    // Write timestamp
    fprintf(state->combat_log_file, "[%02d:%02d:%02d Tick:%d] ", 
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, state->game_tick);
    
    // Write formatted message
    vfprintf(state->combat_log_file, format, args);
    fprintf(state->combat_log_file, "\n");
    fflush(state->combat_log_file);
    
    va_end(args);
    
    // Also print to console for immediate feedback
    printf("[ENHANCED] ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

// Update performance metrics
void enhanced_combat_update_performance(EnhancedGameState* state, float frame_time) {
    if (!state || !state->performance_monitoring_enabled) return;
    
    state->performance.frame_count++;
    state->performance.total_frame_time += frame_time;
    
    // Calculate moving average FPS
    if (state->performance.frame_count > 0) {
        state->performance.avg_fps = 1.0f / (state->performance.total_frame_time / state->performance.frame_count);
    }
    
    // Reset counters periodically
    if (state->performance.frame_count >= 600) { // Every 10 seconds
        enhanced_combat_export_metrics(state);
        state->performance.frame_count = 0;
        state->performance.total_frame_time = 0.0f;
        state->performance.combat_events = 0;
        state->performance.animation_updates = 0;
    }
}

// Export performance metrics to file
void enhanced_combat_export_metrics(EnhancedGameState* state) {
    if (!state || !state->performance_log_file) return;
    
    fprintf(state->performance_log_file, "Tick: %d, FPS: %.1f, Combat Events: %d, Animations: %d, Physics Sync: %.2fms\n",
            state->game_tick, state->performance.avg_fps, state->performance.combat_events,
            state->performance.animation_updates, state->performance.physics_sync_time);
    fflush(state->performance_log_file);
}

// Helper functions
void sync_avatar_with_combat(EnhancedPlayer* player) {
    if (!player || !player->combat.avatar) return;
    
    // Sync animation based on combat state
    AvatarAnimation target_anim = ANIM_IDLE;
    
    if (player->combat.attack_animation > 0) {
        target_anim = ANIM_ATTACKING;
    } else if (player->vel_x != 0.0f || player->vel_z != 0.0f) {
        target_anim = ANIM_WALKING;
    }
    
    if (player->combat.current_anim != target_anim) {
        gangbeast_avatar_set_animation(player->combat.avatar, target_anim);
        player->combat.current_anim = target_anim;
    }
}

void trigger_combat_animation(EnhancedPlayer* player, int animation_type) {
    if (!player || !player->combat.avatar) return;
    
    switch (animation_type) {
        case ENHANCED_ACTION_PUNCH:
        case ENHANCED_ACTION_KICK:
            gangbeast_avatar_set_animation(player->combat.avatar, ANIM_ATTACKING);
            player->combat.current_anim = ANIM_ATTACKING;
            player->combat.animation_time = 0.0f;
            break;
    }
}

void update_ragdoll_forces(EnhancedPlayer* player) {
    if (!player || !player->combat.ragdoll_active || !coreria_apply_limb_force) return;
    
    RagdollHandle ragdoll = {(unsigned int)(uintptr_t)player->combat.ragdoll_handle};
    
    if (player->combat.attack_animation > 20) {
        // Apply attack forces
        float force_magnitude = (player->combat.attack_animation > 35) ? 150.0f : 100.0f;
        float force_x = sinf(player->facing_angle) * force_magnitude;
        float force_z = cosf(player->facing_angle) * force_magnitude;
        
        // Apply force to appropriate limbs
        coreria_apply_limb_force(ragdoll, 2, force_x, 0.0f, force_z, 0.0f, 0.0f, 0.0f); // Left arm
        coreria_apply_limb_force(ragdoll, 3, force_x, 0.0f, force_z, 0.0f, 0.0f, 0.0f); // Right arm
    }
}

// Performance timing helpers
void start_performance_timer(const char* operation) {
    g_timer_start = clock();
}

void end_performance_timer(EnhancedGameState* state, const char* operation) {
    if (!state || !state->performance_monitoring_enabled) return;
    
    clock_t end_time = clock();
    float elapsed = ((float)(end_time - g_timer_start)) / CLOCKS_PER_SEC * 1000.0f; // Convert to milliseconds
    
    if (strcmp(operation, "physics_sync") == 0) {
        state->performance.physics_sync_time = elapsed;
    }
}