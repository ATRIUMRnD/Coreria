
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "player_controller.h"
#include "combat_system.h"
#include "physics_manager.h"
#include "game_state.h"

#define MAX_DYNAMIC_OBJECTS 16
typedef struct {
    float x, z;
    int style;
} DynamicObject;
DynamicObject dynamic_objects[MAX_DYNAMIC_OBJECTS];
int dynamic_object_count = 0;

float player_x = 0.0f, player_z = -5.0f;
int player_style_global = 0;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_W) player_z += 0.5f;
        if (key == GLFW_KEY_S) player_z -= 0.5f;
        if (key == GLFW_KEY_A) player_x -= 0.5f;
        if (key == GLFW_KEY_D) player_x += 0.5f;
        if (key == GLFW_KEY_SPACE && dynamic_object_count < MAX_DYNAMIC_OBJECTS) {
            dynamic_objects[dynamic_object_count].x = player_x;
            dynamic_objects[dynamic_object_count].z = player_z;
            dynamic_objects[dynamic_object_count].style = player_style_global;
            dynamic_object_count++;
            add_log("[Game] New object spawned at player position.");
        }
    }
}

#define MAX_LOG_LINES 32
#define LOG_LINE_LENGTH 128

char game_logs[MAX_LOG_LINES][LOG_LINE_LENGTH];
int log_count = 0;

void add_log(const char* msg) {
    if (log_count < MAX_LOG_LINES) {
        strncpy(game_logs[log_count], msg, LOG_LINE_LENGTH-1);
        game_logs[log_count][LOG_LINE_LENGTH-1] = '\0';
        log_count++;
    } else {
        // Scroll logs up
        for (int i = 1; i < MAX_LOG_LINES; ++i)
            strcpy(game_logs[i-1], game_logs[i]);
        strncpy(game_logs[MAX_LOG_LINES-1], msg, LOG_LINE_LENGTH-1);
        game_logs[MAX_LOG_LINES-1][LOG_LINE_LENGTH-1] = '\0';
    }
}

void render_logs() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1,1,1);
    for (int i = 0; i < log_count; ++i) {
        glRasterPos2i(10, 580 - i*18);
        for (int j = 0; game_logs[i][j] != '\0'; ++j)
            glutBitmapCharacter(GLUT_BITMAP_8_BY_13, game_logs[i][j]);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
// ChaosForge main game loop with 3D base plate, training, bots, and synchronized output
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
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

void draw_dynamic_objects() {
    for (int i = 0; i < dynamic_object_count; ++i) {
        draw_player(dynamic_objects[i].x, dynamic_objects[i].z, dynamic_objects[i].style, 0);
    }
}

void render_scene(GameState* state, int training_phase, int master_style, int player_style) {
    draw_base_plate();
    if (training_phase) {
        draw_player(player_x, player_z, player_style, 0); // player
        draw_player(0.0f, 5.0f, master_style, 1);  // master
        draw_dynamic_objects();
    } else {
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            float angle = i * (2 * M_PI / MAX_PLAYERS);
            float x = 7.0f * cos(angle);
            float z = 7.0f * sin(angle);
            draw_player(x, z, state->players[i].style, 0);
        }
        draw_dynamic_objects();
    }
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "ChaosForge Arena", NULL, NULL);
    glfwSetKeyCallback(window, key_callback);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    int fake_argc = 1;
    char *fake_argv[] = { "chaosforge", NULL };
    glutInit(&fake_argc, fake_argv);
    add_log("[ChaosForge] Window opened: 800x600");
    add_log("[ChaosForge] Initializing Coreria engine...");
    add_log("[ChaosForge] Loading arena: chaosforge_coliseum.obj");
    add_log("[ChaosForge] Initializing ODE physics...");

    // Training phase
    int player_style = rand() % 4;
    int master_style = player_style; // Master matches player style
    player_style_global = player_style;
    char logbuf[LOG_LINE_LENGTH];
    snprintf(logbuf, LOG_LINE_LENGTH, "[Training] Welcome! You must defeat the master (%s) to earn your entrance exam ticket.", get_style_name(master_style));
    add_log(logbuf);
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
            add_log("[Training] You land a decisive blow! Master defeated. Ticket granted.");
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

    add_log("[Time: 0.016s] Game loop started (60 FPS)");
    int tick = 1;
    double target_frame_time = 1.0 / 120.0;
    while (!glfwWindowShouldClose(window)) {
        double frame_start = glfwGetTime();
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Real-time update: objects can be generated/modified here as user codes them in
        render_scene(&state, 0, 0, 0);
        render_logs();
        update_game(&state, tick);
        glfwSwapBuffers(window);
        glfwPollEvents();
        tick++;
        double frame_end = glfwGetTime();
        double elapsed = frame_end - frame_start;
        if (elapsed < target_frame_time) {
            // Sleep to maintain 120 FPS
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (long)((target_frame_time - elapsed) * 1e9);
            nanosleep(&ts, NULL);
        }
    }
    // Clean exit when window is closed
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
