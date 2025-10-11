// ChaosForge main game loop with 3D base plate, training, bots, and synchronized output
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include "player_controller.h"
#include "combat_system.h"
#include "physics_manager.h"
#include "game_state.h"

void draw_base_plate() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 0.0f, -10.0f);
    glVertex3f(10.0f, 0.0f, -10.0f);
    glVertex3f(10.0f, 0.0f, 10.0f);
    glVertex3f(-10.0f, 0.0f, 10.0f);
    glEnd();
}

void draw_player(float x, float z, int style, int is_master) {
    if (is_master)
        glColor3f(1.0f, 0.8f, 0.2f);
    else
        glColor3f(0.2f + 0.2f * style, 0.5f, 0.8f - 0.2f * style);
    glPushMatrix();
    glTranslatef(x, 0.5f, z);
    glutSolidSphere(0.5, 16, 16);
    glPopMatrix();
}

void render_scene(GameState* state, int training_phase, int master_style, int player_style) {
    draw_base_plate();
    if (training_phase) {
        draw_player(0.0f, -5.0f, player_style, 0); // player
        draw_player(0.0f, 5.0f, master_style, 1);  // master
    } else {
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            float angle = i * (2 * M_PI / MAX_PLAYERS);
            float x = 7.0f * cos(angle);
            float z = 7.0f * sin(angle);
            draw_player(x, z, state->players[i].style, 0);
        }
    }
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "ChaosForge Arena", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    printf("[ChaosForge] Window opened: 800x600\n");
    printf("[ChaosForge] Initializing Coreria engine...\n");
    printf("[ChaosForge] Loading arena: chaosforge_coliseum.obj\n");
    printf("[ChaosForge] Initializing ODE physics...\n");

    // Training phase
    int player_style = rand() % 4;
    int master_style = player_style; // Master matches player style
    printf("[Training] Welcome! You must defeat the master (%s) to earn your entrance exam ticket.\n", get_style_name(master_style));
    int passed_exam = 0;
    int training_ticks = 0;
    while (!glfwWindowShouldClose(window) && !passed_exam) {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_scene(NULL, 1, master_style, player_style);
        glfwSwapBuffers(window);
        glfwPollEvents();
        training_ticks++;
        if (training_ticks == 120) {
            printf("[Training] You land a decisive blow! Master defeated. Ticket granted.\n");
            passed_exam = 1;
        }
    }

    // Main match setup
    GameState state;
    init_game_state(&state);
    // Fill lobby with bots matching player style if no other players
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        state.players[i].style = player_style;
    }
    spawn_players(&state);

    printf("[Time: 0.016s] Game loop started (60 FPS)\n");
    int tick = 1;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_scene(&state, 0, 0, 0);
        update_game(&state, tick);
        if (state.game_over) break;
        glfwSwapBuffers(window);
        glfwPollEvents();
        tick++;
    }
    printf("[ChaosForge] Game Over: Player %d (%s) wins! (Lives: %d, Health: %d)\n", state.winner, get_style_name(state.winner_style), state.players[state.winner].lives, state.players[state.winner].health);
    printf("[ChaosForge] Shutting down...\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
