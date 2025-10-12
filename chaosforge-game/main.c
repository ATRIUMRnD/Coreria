// Particle system for attack/respawn effects
#define MAX_PARTICLES 64
typedef struct {
    float x, y, z;
    float vx, vy, vz;
    int life;
    int type; // 0: attack, 1: respawn
} Particle;
Particle particles[MAX_PARTICLES];
int particle_count = 0;

void spawn_particles(float x, float z, int type) {
    for (int i = 0; i < 8 && particle_count < MAX_PARTICLES; ++i) {
        float angle = (float)i * (2 * M_PI / 8);
        particles[particle_count].x = x;
        particles[particle_count].y = 0.7f;
        particles[particle_count].z = z;
        particles[particle_count].vx = 0.1f * cos(angle);
        particles[particle_count].vy = 0.08f;
        particles[particle_count].vz = 0.1f * sin(angle);
        particles[particle_count].life = 20;
        particles[particle_count].type = type;
        particle_count++;
    }
}

void update_particles() {
    for (int i = 0; i < particle_count; ++i) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].z += particles[i].vz;
        particles[i].vy -= 0.005f; // gravity
        particles[i].life--;
        if (particles[i].life <= 0) {
            // Remove particle
            particles[i] = particles[particle_count-1];
            particle_count--;
            i--;
        }
    }
}

void draw_particles() {
    for (int i = 0; i < particle_count; ++i) {
        glPushMatrix();
        glTranslatef(particles[i].x, particles[i].y, particles[i].z);
        if (particles[i].type == 0)
            glColor3f(1,0,0); // attack: red
        else
            glColor3f(1,1,0); // respawn: yellow
        glutSolidSphere(0.12, 8, 8);
        glPopMatrix();
    }
}
// Camera controls
float cam_angle = 0.0f;
float cam_radius = 18.0f;
float cam_height = 8.0f;
int cam_follow_player = 0;
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
            // Camera controls
            if (key == GLFW_KEY_LEFT) cam_angle -= 0.1f;
            if (key == GLFW_KEY_RIGHT) cam_angle += 0.1f;
            if (key == GLFW_KEY_UP) cam_height += 0.5f;
            if (key == GLFW_KEY_DOWN) cam_height -= 0.5f;
            if (key == GLFW_KEY_PAGE_UP) cam_radius -= 1.0f;
            if (key == GLFW_KEY_PAGE_DOWN) cam_radius += 1.0f;
            if (key == GLFW_KEY_F) cam_follow_player = !cam_follow_player;
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
    float r = 0.2f + 0.2f * style;
    float g = 0.5f;
    float b = 0.8f - 0.2f * style;
    if (attack_anim > 0) {
        glColor3f(1.0f, 0.0f, 0.0f); // attack flash
    } else if (respawn_anim > 0) {
        glColor3f(1.0f, 1.0f, 0.0f); // respawn flash
    } else if (is_master) {
        glColor3f(1.0f, 0.8f, 0.2f);
    } else {
        // Color based on health
        float health_frac = health / 100.0f;
        glColor3f(r * health_frac + (1-health_frac)*1.0f, g * health_frac, b * health_frac);
    }
    if (is_master == 2) {
        glutSolidCube(0.8);
    } else {
        glutSolidSphere(0.5, 16, 16);
    }
    // Draw health bar
    glPushMatrix();
    glTranslatef(0, 0.7f, 0);
    glColor3f(0.2f, 0.9f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-0.3f, 0.0f, 0.0f);
    glVertex3f(-0.3f + 0.6f * (health/100.0f), 0.0f, 0.0f);
    glVertex3f(-0.3f + 0.6f * (health/100.0f), 0.05f, 0.0f);
    glVertex3f(-0.3f, 0.05f, 0.0f);
    glEnd();
    glPopMatrix();
    // Draw floating text (name, style, lives)
    glPushMatrix();
    glTranslatef(0, 1.0f, 0);
    glColor3f(1,1,1);
    char info[64];
    snprintf(info, sizeof(info), "Player %s | %s | Lives: %d", is_master ? "Master" : "Bot", fighting_styles[style], lives);
    glRasterPos3f(-0.4f, 0.0f, 0.0f);
    for (int i = 0; info[i] != '\0'; ++i)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, info[i]);
    glPopMatrix();
    glPopMatrix();
}

void draw_dynamic_objects() {
    for (int i = 0; i < dynamic_object_count; ++i) {
        draw_player(dynamic_objects[i].x, dynamic_objects[i].z, dynamic_objects[i].style, 0);
    }
}

void render_scene(GameState* state, int training_phase, int master_style, int player_style) {
    // Draw particles
    draw_particles();
    // Camera setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 800.0/600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float target_x = 0.0f, target_z = 0.0f;
    if (cam_follow_player && state) {
        // Follow player 0
        float angle = 0 * (2 * M_PI / MAX_PLAYERS);
        target_x = 7.0f * cos(angle);
        target_z = 7.0f * sin(angle);
    }
    float cam_x = target_x + cam_radius * cos(cam_angle);
    float cam_y = cam_height;
    float cam_z = target_z + cam_radius * sin(cam_angle);
    gluLookAt(cam_x, cam_y, cam_z, target_x, 0.0f, target_z, 0.0f, 1.0f, 0.0f);
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
        draw_player(player_x, player_z, player_style, 0, 100, 0, 0, 2); // player
        draw_player(0.0f, 5.0f, master_style, 1, 100, 0, 0, 2);  // master
        draw_dynamic_objects();
    } else {
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            float angle = i * (2 * M_PI / MAX_PLAYERS);
            float x = 7.0f * cos(angle);
            float z = 7.0f * sin(angle);
            int is_bot = 2;
            int health = state->players[i].health;
            int attack_anim = state->attack_anim[i];
            int respawn_anim = state->respawn_anim[i];
            int lives = state->players[i].lives;
            draw_player(x, z, state->players[i].style, is_bot, health, attack_anim, respawn_anim, lives);
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
        // Decrement animation timers and spawn particles
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            if (state.attack_anim[i] > 0) {
                state.attack_anim[i]--;
                if (state.attack_anim[i] == 9) {
                    float angle = i * (2 * M_PI / MAX_PLAYERS);
                    float x = 7.0f * cos(angle);
                    float z = 7.0f * sin(angle);
                    spawn_particles(x, z, 0);
                }
            }
            if (state.respawn_anim[i] > 0) {
                state.respawn_anim[i]--;
                if (state.respawn_anim[i] == 19) {
                    float angle = i * (2 * M_PI / MAX_PLAYERS);
                    float x = 7.0f * cos(angle);
                    float z = 7.0f * sin(angle);
                    spawn_particles(x, z, 1);
                }
            }
        }
        update_particles();
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
