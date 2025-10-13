#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

/// Fighting style constants for C code
constexpr static const int STYLE_BRAWLER = 0;

constexpr static const int STYLE_STRIKER = 1;

constexpr static const int STYLE_PHANTOM = 2;

constexpr static const int STYLE_TITAN = 3;

/// C-compatible performance metrics
struct CPerformanceMetrics {
  float frame_time_ms;
  float physics_time_ms;
  float network_latency_ms;
  float update_rate;
  int player_count;
  int memory_usage_mb;
};

/// C-compatible match state structure
struct CMatchState {
  unsigned int match_id;
  int status;
  int player_count;
  int max_players;
  float time_remaining;
  int winner_id;
};

/// C-compatible player state structure
struct CPlayerState {
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
};

/// C-compatible input action
struct CInputAction {
  int action_type;
  float param1;
  float param2;
};

extern "C" {

/// Initialize the multiplayer engine
/// Returns 0 on success, negative on error
int coreria_init_engine();

/// Shutdown the multiplayer engine
int coreria_shutdown_engine();

/// Step the physics simulation by delta_time seconds
/// Returns 1 if update occurred, 0 if skipped (timing)
int coreria_step_simulation(float delta_time);

/// Get current player count across all matches
int coreria_get_player_count();

/// Set engine to server mode on specified port
int coreria_init_server(uint16_t port);

/// Connect to server as client
int coreria_init_client(const char *server_addr);

/// Get performance metrics
int coreria_get_performance_metrics(CPerformanceMetrics *metrics);

const char *coreria_get_last_error();

/// Create a new match with specified max players
/// Returns match_id on success, 0 on failure
unsigned int coreria_create_match(int max_players);

/// Get current match state
/// Returns 0 on success, negative on error
int coreria_get_match_state(unsigned int match_id, CMatchState *state);

/// End a match
int coreria_end_match(unsigned int match_id);

/// Get all active match IDs
/// Returns number of matches, fills match_ids array
int coreria_get_active_matches(unsigned int *match_ids, int max_matches);

/// Add a player to a match with specified fighting style
/// Returns player_id on success, 0 on failure
unsigned int coreria_add_player(unsigned int match_id, int style_id);

/// Remove a player from their current match
int coreria_remove_player(unsigned int player_id);

/// Get player state information
/// Returns 0 on success, negative on error
int coreria_get_player_state(unsigned int player_id, CPlayerState *state);

/// Apply force to a player (for combat/physics)
/// force_x, force_y, force_z: Force vector components
int coreria_apply_force(unsigned int player_id, float force_x, float force_y, float force_z);

/// Set player position (for respawning/teleporting)
int coreria_set_player_position(unsigned int player_id, float x, float y, float z);

/// Deal damage to a player
int coreria_damage_player(unsigned int attacker_id, unsigned int victim_id, float damage);

/// Get all player IDs in a match
/// Returns number of players, fills player_ids array
int coreria_get_match_players(unsigned int match_id, unsigned int *player_ids, int max_players);

/// Check if player is alive
int coreria_is_player_alive(unsigned int player_id);

/// Respawn a dead player
int coreria_respawn_player(unsigned int player_id);

/// Send input action for a player
/// Returns 0 on success, negative on error
int coreria_send_input(unsigned int player_id, CInputAction action);

/// Send multiple input actions in a batch
int coreria_send_input_batch(unsigned int player_id, const CInputAction *actions, int action_count);

/// Clear input buffer for a player
int coreria_clear_input_buffer(unsigned int player_id);

/// Check if a specific action is currently active for a player
int coreria_is_action_active(unsigned int player_id, CInputAction action);

/// Process a Windows keycode and convert to input action
/// Returns action_type on success, -1 if key not mapped
int coreria_keycode_to_action(unsigned int keycode);

/// Get input buffer size for a player
int coreria_get_input_buffer_size(unsigned int player_id);

/// Enable/disable input processing for a player
int coreria_set_input_enabled(unsigned int player_id, int enabled);

}  // extern "C"
