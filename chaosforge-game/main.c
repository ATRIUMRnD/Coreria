#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <math.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif

// Fighting styles
const char* fighting_styles[] = {"Brawler", "Striker", "Phantom", "Titan"};
int selected_style = 0;
int in_menu = 1;

void copy_to_clipboard(const char* text) {
#ifdef _WIN32
    const size_t len = strlen(text) + 1;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), text, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
#endif
}
void draw_background() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.1f, 0.3f); glVertex2i(0, 0);
    glColor3f(0.2f, 0.2f, 0.5f); glVertex2i(800, 0);
    glColor3f(0.3f, 0.3f, 0.7f); glVertex2i(800, 600);
    glColor3f(0.2f, 0.2f, 0.5f); glVertex2i(0, 600);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
void draw_menu() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1,1,1);
    glRasterPos2i(300, 500);
    const char* title = "Select Fighting Style:";
    for (int i = 0; title[i] != '\0'; ++i)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, title[i]);
    for (int i = 0; i < 4; ++i) {
        glColor3f(i == selected_style ? 1 : 0.7, 1, 1);
        glRasterPos2i(320, 450 - i*40);
        for (int j = 0; fighting_styles[i][j] != '\0'; ++j)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, fighting_styles[i][j]);
    }
    glColor3f(1,1,0.7);
    glRasterPos2i(320, 250);
    const char* start = "Press ENTER to start, C to copy style";
    for (int i = 0; start[i] != '\0'; ++i)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, start[i]);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
// Prototype for add_log to fix implicit declaration
void add_log(const char* msg);
 
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
        if (key == GLFW_KEY_F12 && action == GLFW_PRESS) {
            show_console = !show_console;
        }
        if (in_menu) {
            if (key == GLFW_KEY_UP) selected_style = (selected_style + 3) % 4;
            if (key == GLFW_KEY_DOWN) selected_style = (selected_style + 1) % 4;
            if (key == GLFW_KEY_ENTER) {
                player_style_global = selected_style;
                in_menu = 0;
            }
            if (key == GLFW_KEY_C) {
                copy_to_clipboard(fighting_styles[selected_style]);
                add_log("[Menu] Style copied to clipboard.");
            }
        } else {
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
}

#define MAX_LOG_LINES 32
#define LOG_LINE_LENGTH 128

char game_logs[MAX_LOG_LINES][LOG_LINE_LENGTH];
int log_count = 0;
int show_console = 0; // 0: hidden, 1: visible

void add_log(const char* msg) {
    if (log_count < MAX_LOG_LINES) {
        strncpy(game_logs[log_count], msg, LOG_LINE_LENGTH-1);
        game_logs[log_count][LOG_LINE_LENGTH-1] = '\0';
        log_count++;
    } else {
            // Menu loop
            while (!glfwWindowShouldClose(window) && in_menu) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                draw_background();
                draw_menu();
                if (show_console) render_logs();
                glfwSwapBuffers(window);
                glfwPollEvents();
            }
            // Spawn player in lobby on base plate
            player_x = 0.0f;
            player_z = 0.0f;
            int player_style = player_style_global;
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
    glPushMatrix();
    glTranslatef(x, 0.5f, z);
    if (is_master) {
        glColor3f(1.0f, 0.8f, 0.2f);
        glutSolidSphere(0.6, 20, 20);
    } else if (style == 99) { // respawn flash
        glColor3f(1.0f, 1.0f, 0.0f);
        glutSolidSphere(0.7, 20, 20);
    } else if (style == 98) { // attack flash
        glColor3f(1.0f, 0.0f, 0.0f);
        glutSolidSphere(0.7, 20, 20);
    } else if (is_master == 2) { // bot
        glColor3f(0.2f + 0.2f * style, 0.5f, 0.8f - 0.2f * style);
        glutSolidCube(0.8);
    } else {
        glColor3f(0.2f + 0.2f * style, 0.5f, 0.8f - 0.2f * style);
        glutSolidSphere(0.5, 16, 16);
    }
    glPopMatrix();
}

void draw_dynamic_objects() {
    for (int i = 0; i < dynamic_object_count; ++i) {
        draw_player(dynamic_objects[i].x, dynamic_objects[i].z, dynamic_objects[i].style, 0);
    }
}

void render_scene(GameState* state, int training_phase, int master_style, int player_style) {
    // Example: shrink arena visually based on tick
    static float arena_scale = 1.0f;
    if (state && state->arena_shrink_tick > 0) {
        arena_scale = 1.0f - 0.1f * state->arena_shrink_tick;
        glPushMatrix();
        glScalef(arena_scale, 1.0f, arena_scale);
        draw_base_plate();
        glPopMatrix();
    } else {
        draw_base_plate();
    }
    if (training_phase) {
        draw_player(player_x, player_z, player_style, 0); // player
        draw_player(0.0f, 5.0f, master_style, 1);  // master
        draw_dynamic_objects();
    } else {
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            float angle = i * (2 * M_PI / MAX_PLAYERS);
            float x = 7.0f * cos(angle);
            float z = 7.0f * sin(angle);
            int is_bot = 2;
            draw_player(x, z, state->players[i].style, is_bot);
        }
        draw_dynamic_objects();
    }
}

int main() {
    fprintf(stderr, "[DEBUG] Starting ChaosForge Arena...\n");
    if (!glfwInit()) {
        fprintf(stderr, "[ERROR] Failed to initialize GLFW\n");
        return -1;
    } else {
        fprintf(stderr, "[DEBUG] GLFW initialized successfully.\n");
    }
    GLFWwindow* window = glfwCreateWindow(800, 600, "ChaosForge Arena", NULL, NULL);
    if (!window) {
        fprintf(stderr, "[ERROR] Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    } else {
        fprintf(stderr, "[DEBUG] GLFW window created successfully.\n");
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    fprintf(stderr, "[DEBUG] OpenGL context made current.\n");
    int fake_argc = 1;
    char *fake_argv[] = { "chaosforge", NULL };
    glutInit(&fake_argc, fake_argv);
    fprintf(stderr, "[DEBUG] GLUT initialized.\n");
    add_log("[ChaosForge] Window opened: 800x600");
    add_log("[ChaosForge] Initializing Coreria engine...");
    add_log("[ChaosForge] Loading arena: chaosforge_coliseum.obj");
    add_log("[ChaosForge] Initializing ODE physics...");

    // Training phase
    // Menu loop
    while (!glfwWindowShouldClose(window) && in_menu) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_background();
        draw_menu();
        if (show_console) render_logs();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    int player_style = player_style_global;
    int master_style = player_style;
    char logbuf[LOG_LINE_LENGTH];
    snprintf(logbuf, LOG_LINE_LENGTH, "[Training] Welcome! You must defeat the master (%s) to earn your entrance exam ticket.", get_style_name(master_style));
    add_log(logbuf);
    int passed_exam = 0;
    int training_ticks = 0;
    while (!glfwWindowShouldClose(window) && !passed_exam) {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_background();
        render_scene(NULL, 1, master_style, player_style);
        if (show_console) render_logs();
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
        draw_background();
        render_scene(&state, 0, 0, 0);
        if (show_console) render_logs();
        update_game(&state, tick);
        glfwSwapBuffers(window);
        glfwPollEvents();
        tick++;
        double frame_end = glfwGetTime();
        double elapsed = frame_end - frame_start;
        if (elapsed < target_frame_time) {
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = (long)((target_frame_time - elapsed) * 1e9);
            nanosleep(&ts, NULL);
        }
    }
        // Graceful shutdown: clean up resources and exit
        add_log("[ChaosForge] Shutting down gracefully...");
        // Add any additional cleanup here (free memory, close files, etc.)
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(0);
}
