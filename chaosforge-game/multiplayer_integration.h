#ifndef MULTIPLAYER_INTEGRATION_H
#define MULTIPLAYER_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif

// Include the generated Rust FFI header
#include "../chaosforge-multiplayer/target/chaosforge_multiplayer.h"

// Integration functions
int init_multiplayer_engine(void);
void cleanup_multiplayer_engine(void);
void update_multiplayer_engine(float delta_time);
int create_multiplayer_match(int max_players);
int add_multiplayer_player(int match_id, int fighting_style);
void get_multiplayer_performance(PerformanceMetrics* metrics);

// Game state synchronization
typedef struct {
    float pos_x, pos_y, pos_z;
    float yaw, pitch;
    int health;
    int fighting_style;
    int is_alive;
} GamePlayerState;

void sync_player_state(int player_id, GamePlayerState* state);
void get_player_state(int player_id, GamePlayerState* state);

// Real-time updates
void on_f12_pressed(void);  // For real-time reloading
void refresh_multiplayer_systems(void);

#ifdef __cplusplus
}
#endif

#endif // MULTIPLAYER_INTEGRATION_H
