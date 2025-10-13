#include "game_state.h"
#include "combat_system.h"
#include "physics_manager.h"
#include <stdio.h>

void init_game_state(GameState* state) {
    state->game_over = 0;
    state->winner = -1;
    state->winner_style = -1;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        state->players[i].id = i;
        state->players[i].style = i;
        state->players[i].lives = 2;
        state->players[i].health = 100;
        state->players[i].meter = 0;
        state->attack_anim[i] = 0;
        state->respawn_anim[i] = 0;
    }
}

void update_game(GameState* state, int tick) {
    update_combat(state, tick);
    update_physics(tick);
    // Check win condition
    int alive = 0, last_alive = -1;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        if (state->players[i].lives > 0) {
            alive++;
            last_alive = i;
        }
    }
    if (alive == 1) {
        state->game_over = 1;
        state->winner = last_alive;
        state->winner_style = state->players[last_alive].style;
    }
}


