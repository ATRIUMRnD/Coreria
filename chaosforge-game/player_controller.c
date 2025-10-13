#include "player_controller.h"
#include <stdio.h>
#include <stdlib.h>

const char* styles[] = {"Brawler", "Striker", "Phantom", "Titan"};

void spawn_players(GameState* state) {
    for (int i = 0; i < 4; ++i) {
        state->players[i].id = i;
        state->players[i].style = rand() % 4;
        state->players[i].lives = 2;
        state->players[i].health = 100;
        state->players[i].meter = 0;
        printf("[Player %d] Spawned at (%d, %d, %d) with style: %s (Lives: %d, Health: %d, Meter: %d)\n",
            i, (i==0?10:(i==1?-10:0)), (i==2?10:(i==3?-10:0)), (i==0?10:(i==1?-10:0)),
            styles[state->players[i].style], 2, 100, 0);
    }
}

const char* get_style_name(int style_id) {
    return styles[style_id % 4];
}
