#pragma once
#define MAX_PLAYERS 4

typedef struct {
    int id;
    int style;
    int lives;
    int health;
    int meter;
} Player;

typedef struct {
    Player players[MAX_PLAYERS];
    int game_over;
    int winner;
    int winner_style;
    int attack_anim[MAX_PLAYERS]; // ticks left for attack flash
    int respawn_anim[MAX_PLAYERS]; // ticks left for respawn flash
} GameState;

void init_game_state(GameState* state);
void update_game(GameState* state, int tick);
void spawn_players(GameState* state);
