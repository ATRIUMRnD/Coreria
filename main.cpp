// main.c
#include "coreria_engine.h"
#include "player_controller.h"
#include "combat_system.h"
#include "physics_manager.h"
#include "game_state.h"

int main() {
    coreria_engine_init();  // ChaosForge runs on Coreria engine
    physics_manager_init_ode();
    GameState state;
    game_state_load_arena(&state, "chaosforge_coliseum.obj");  // Updated asset name
    for (int i = 0; i < 4; i++) {
        game_state_spawn_player(&state, i);
    }
    while (game_state_is_running(&state)) {
        coreria_engine_update(0.016f);  // 60 FPS
    }
    coreria_engine_shutdown();
    return 0;
}