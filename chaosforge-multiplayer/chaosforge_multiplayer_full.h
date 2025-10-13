#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Fighting style constants for C code
 */
#define STYLE_BRAWLER 0

#define STYLE_STRIKER 1

#define STYLE_PHANTOM 2

#define STYLE_TITAN 3

/**
 * Fighting style IDs for C compatibility
 */
typedef enum FightingStyleId {
  Brawler = 0,
  Striker = 1,
  Phantom = 2,
  Titan = 3,
} FightingStyleId;

/**
 * C-compatible performance metrics
 */
typedef struct CPerformanceMetrics {
  float frame_time_ms;
  float physics_time_ms;
  float network_latency_ms;
  float update_rate;
  int player_count;
  int memory_usage_mb;
} CPerformanceMetrics;

/**
 * C-compatible match state structure
 */
typedef struct CMatchState {
  unsigned int match_id;
  int status;
  int player_count;
  int max_players;
  float time_remaining;
  int winner_id;
} CMatchState;

/**
 * C-compatible player state structure
 */
typedef struct CPlayerState {
  unsigned int player_id;
  float x;
  float y;
  float z;
  float qx;
  float qy;
  float qz;
  float qw;
  float vx;
  float vy;
  float vz;
  float health;
  int style_id;
  int is_alive;
} CPlayerState;

/**
 * C-compatible input action
 */
typedef struct CInputAction {
  int action_type;
  float param1;
  float param2;
} CInputAction;

/**
 * Handle to a ragdoll entity for FFI safety
 */
typedef struct RagdollHandle {
  uint32_t id;
} RagdollHandle;

/**
 * Initialize the multiplayer engine
 * Returns 0 on success, negative on error
 */
int coreria_init_engine(void);

/**
 * Shutdown the multiplayer engine
 */
int coreria_shutdown_engine(void);

/**
 * Step the physics simulation by delta_time seconds
 * Returns 1 if update occurred, 0 if skipped (timing)
 */
int coreria_step_simulation(float delta_time);

/**
 * Get current player count across all matches
 */
int coreria_get_player_count(void);

/**
 * Set engine to server mode on specified port
 */
int coreria_init_server(uint16_t port);

/**
 * Connect to server as client
 */
int coreria_init_client(const char *server_addr);

/**
 * Get performance metrics
 */
int coreria_get_performance_metrics(struct CPerformanceMetrics *metrics);

const char *coreria_get_last_error(void);

/**
 * Create a new match with specified max players
 * Returns match_id on success, 0 on failure
 */
unsigned int coreria_create_match(int max_players);

/**
 * Get current match state
 * Returns 0 on success, negative on error
 */
int coreria_get_match_state(unsigned int match_id, struct CMatchState *state);

/**
 * End a match
 */
int coreria_end_match(unsigned int match_id);

/**
 * Get all active match IDs
 * Returns number of matches, fills match_ids array
 */
int coreria_get_active_matches(unsigned int *match_ids, int max_matches);

/**
 * Add a player to a match with specified fighting style
 * Returns player_id on success, 0 on failure
 */
unsigned int coreria_add_player(unsigned int match_id, int style_id);

/**
 * Remove a player from their current match
 */
int coreria_remove_player(unsigned int player_id);

/**
 * Get player state information
 * Returns 0 on success, negative on error
 */
int coreria_get_player_state(unsigned int player_id, struct CPlayerState *state);

/**
 * Apply force to a player (for combat/physics)
 * force_x, force_y, force_z: Force vector components
 */
int coreria_apply_force(unsigned int player_id, float force_x, float force_y, float force_z);

/**
 * Set player position (for respawning/teleporting)
 */
int coreria_set_player_position(unsigned int player_id, float x, float y, float z);

/**
 * Deal damage to a player
 */
int coreria_damage_player(unsigned int attacker_id, unsigned int victim_id, float damage);

/**
 * Get all player IDs in a match
 * Returns number of players, fills player_ids array
 */
int coreria_get_match_players(unsigned int match_id, unsigned int *player_ids, int max_players);

/**
 * Check if player is alive
 */
int coreria_is_player_alive(unsigned int player_id);

/**
 * Respawn a dead player
 */
int coreria_respawn_player(unsigned int player_id);

/**
 * Send input action for a player
 * Returns 0 on success, negative on error
 */
int coreria_send_input(unsigned int player_id, struct CInputAction action);

/**
 * Send multiple input actions in a batch
 */
int coreria_send_input_batch(unsigned int player_id,
                             const struct CInputAction *actions,
                             int action_count);

/**
 * Clear input buffer for a player
 */
int coreria_clear_input_buffer(unsigned int player_id);

/**
 * Check if a specific action is currently active for a player
 */
int coreria_is_action_active(unsigned int player_id, struct CInputAction action);

/**
 * Process a Windows keycode and convert to input action
 * Returns action_type on success, -1 if key not mapped
 */
int coreria_keycode_to_action(unsigned int keycode);

/**
 * Get input buffer size for a player
 */
int coreria_get_input_buffer_size(unsigned int player_id);

/**
 * Enable/disable input processing for a player
 */
int coreria_set_input_enabled(unsigned int player_id, int enabled);

/**
 * Spawn a ragdoll at specified position with fighting style
 * Returns handle for subsequent operations
 */
struct RagdollHandle coreria_spawn_ragdoll(unsigned int player_id,
                                           enum FightingStyleId style_id,
                                           float x,
                                           float y,
                                           float z);

/**
 * Apply force and torque to specific limb of ragdoll
 * limb_index: 0=head, 1=torso, 2=left_arm, 3=right_arm, 4=left_leg, 5=right_leg
 */
int coreria_apply_limb_force(struct RagdollHandle ragdoll_handle,
                             int limb_index,
                             float force_x,
                             float force_y,
                             float force_z,
                             float torque_x,
                             float torque_y,
                             float torque_z);

/**
 * Get ragdoll position
 */
int coreria_get_ragdoll_position(struct RagdollHandle ragdoll_handle, float *x, float *y, float *z);

/**
 * Destroy ragdoll and cleanup resources
 */
int coreria_destroy_ragdoll(struct RagdollHandle ragdoll_handle);

/**
 * Get number of active ragdolls
 */
int coreria_get_ragdoll_count(void);
