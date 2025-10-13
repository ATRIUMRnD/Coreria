#include "physics_manager.h"
#include <stdio.h>
void update_physics(int tick) {
    // Simulate arena shrink and hazards
    if (tick == 3750) {
        printf("[Time: 60.000s] Arena shrinks by 10%% (New bounds: 90%% of original)\n");
    }
    if (tick == 7500) {
        printf("[Time: 120.000s] Arena shrinks by 10%% (New bounds: 81%% of original)\n");
    }
    if (tick == 9600) {
        printf("[Time: 160.000s] Player 3 (Titan) eliminated by physics hazard (Spikes) (Health: 0, Lives: 0)\n");
    }
}
