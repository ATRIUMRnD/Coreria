// ChaosForge main game loop
#include <stdio.h>
#include "player_controller.h"
#include "combat_system.h"
#include "physics_manager.h"
#include "game_state.h"

int main() {
    printf("[ChaosForge] Initializing Coreria engine...\n");
    printf("[ChaosForge] Loading arena: chaosforge_coliseum.obj\n");
    printf("[ChaosForge] Initializing ODE physics...\n");

    GameState state;
    init_game_state(&state);
    spawn_players(&state);

    printf("[Time: 0.016s] Game loop started (60 FPS)\n");
    for (int tick = 1; tick <= 16000; ++tick) { // Simulate ~160s
        update_game(&state, tick);
        if (state.game_over) break;
    }
    printf("[ChaosForge] Game Over: Player %d (%s) wins! (Lives: %d, Health: %d)\n", state.winner, get_style_name(state.winner_style), state.players[state.winner].lives, state.players[state.winner].health);
    printf("[ChaosForge] Shutting down...\n");
    return 0;
}
