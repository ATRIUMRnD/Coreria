#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// C-compatible declarations (extracted from header)
typedef struct {
    float frame_time_ms;
    float physics_time_ms;
    float network_latency_ms;
    float update_rate;
    int player_count;
    int memory_usage_mb;
} CPerformanceMetrics;

typedef struct {
    unsigned int match_id;
    int status;
    int player_count;
    int max_players;
    float time_remaining;
    int winner_id;
} CMatchState;

typedef struct {
    unsigned int player_id;
    float x, y, z;          // Position
    float qx, qy, qz, qw;   // Rotation quaternion  
    float vx, vy, vz;       // Velocity
    float health;
    int style_id;
} CPlayerState;

// Fighting style constants
#define STYLE_BRAWLER 0
#define STYLE_STRIKER 1  
#define STYLE_PHANTOM 2
#define STYLE_TITAN   3

// FFI function declarations (with coreria_ prefix)
extern int coreria_initialize_engine(void);
extern int coreria_create_match(int max_players, float time_limit);
extern int coreria_add_player(int match_id, int style_id, float x, float y, float z);
extern const CMatchState* coreria_get_match_state(int match_id);
extern int coreria_get_performance_metrics(CPerformanceMetrics* metrics);
extern const CPlayerState* coreria_get_player_state(int player_id);
extern int coreria_step_simulation(float delta_time);
extern int coreria_shutdown_engine(void);

int main() {
    printf("ChaosForge Multiplayer FFI Integration Test\n");
    printf("==========================================\n\n");
    
    // Test engine initialization
    printf("1. Initializing engine...\n");
    int result = coreria_initialize_engine();
    if (result != 0) {
        printf("   ERROR: Engine initialization failed with code %d\n", result);
        return 1;
    }
    printf("   ✓ Engine initialized successfully\n\n");
    
    // Test match creation
    printf("2. Creating match...\n");
    int match_id = coreria_create_match(8, 300.0f);  // Max 8 players, 5 min limit
    if (match_id < 0) {
        printf("   ERROR: Match creation failed with code %d\n", match_id);
        return 1;
    }
    printf("   ✓ Match created with ID: %d\n\n", match_id);
    
    // Test player creation
    printf("3. Adding players...\n");
    int player1, player2;
    
    player1 = coreria_add_player(match_id, STYLE_BRAWLER, 0.0f, 0.0f, 0.0f);
    if (player1 >= 0) {
        printf("   ✓ Player 1 added (Brawler) with ID: %d\n", player1);
    } else {
        printf("   ERROR: Failed to add player 1 (code %d)\n", player1);
    }
    
    player2 = coreria_add_player(match_id, STYLE_STRIKER, 5.0f, 0.0f, 0.0f);
    if (player2 >= 0) {
        printf("   ✓ Player 2 added (Striker) with ID: %d\n", player2);
    } else {
        printf("   ERROR: Failed to add player 2 (code %d)\n", player2);
    }
    printf("\n");
    
    // Test match state query
    printf("4. Querying match state...\n");
    const CMatchState* match_state = coreria_get_match_state(match_id);
    if (match_state != NULL) {
        printf("   ✓ Match ID: %u\n", match_state->match_id);
        printf("   ✓ Player count: %d/%d\n", match_state->player_count, match_state->max_players);
        printf("   ✓ Status: %d\n", match_state->status);
        printf("   ✓ Time remaining: %.1f seconds\n", match_state->time_remaining);
    } else {
        printf("   ERROR: Failed to get match state\n");
    }
    printf("\n");
    
    // Test performance metrics
    printf("5. Getting performance metrics...\n");
    CPerformanceMetrics metrics;
    result = coreria_get_performance_metrics(&metrics);
    if (result == 0) {
        printf("   ✓ Frame time: %.2f ms\n", metrics.frame_time_ms);
        printf("   ✓ Physics time: %.2f ms\n", metrics.physics_time_ms);
        printf("   ✓ Update rate: %.1f FPS\n", metrics.update_rate);
        printf("   ✓ Player count: %d\n", metrics.player_count);
    } else {
        printf("   ERROR: Failed to get performance metrics\n");
    }
    printf("\n");
    
    // Test player state query
    printf("6. Querying player states...\n");
    if (player1 >= 0) {
        const CPlayerState* player_state = coreria_get_player_state(player1);
        if (player_state != NULL) {
            printf("   ✓ Player %u: Position (%.2f, %.2f, %.2f)\n", 
                   player_state->player_id, player_state->x, player_state->y, player_state->z);
            printf("   ✓ Health: %.1f\n", player_state->health);
            printf("   ✓ Style: %d\n", player_state->style_id);
        } else {
            printf("   ERROR: Failed to get player state\n");
        }
    }
    printf("\n");
    
    // Test simulation step
    printf("7. Running simulation step...\n");
    result = coreria_step_simulation(0.016f); // 60 FPS (16.6ms)
    if (result != 0) {
        printf("   ✓ Simulation step completed\n");
    } else {
        printf("   WARNING: Simulation step returned false\n");
    }
    printf("\n");
    
    // Clean up
    printf("8. Cleaning up...\n");
    result = coreria_shutdown_engine();
    if (result == 0) {
        printf("   ✓ Engine shutdown successful\n");
    } else {
        printf("   ERROR: Engine shutdown failed\n");
    }
    
    printf("\n==========================================\n");
    printf("Integration test completed!\n");
    return 0;
}